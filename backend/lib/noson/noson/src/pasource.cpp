/*
 *      Copyright (C) 2018-2019 Jean-Luc Barriere
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "pasource.h"
#include "private/debug.h"
#include "private/pcmblankkiller.h"
#include "private/os/threads/thread.h"

#include <cassert>

#define FRAME_BUFFER    256   // frame size = channels * sampleSize / 8
#define MIN_FRAME_SIZE  1     // 1 channel with sampleSize 8
#define MAX_FRAME_SIZE  32    // 8 channels with sampleSize 32
#define BLANK_FRAMES    FRAME_BUFFER / 4

using namespace NSROOT;

namespace NSROOT
{

class PASourceWorker : private OS::CThread
{
public:
  explicit PASourceWorker(PASource * source);
  virtual ~PASourceWorker() override;

  bool isRunning() { return OS::CThread::IsRunning(); }
  bool isFinished() { return OS::CThread::IsStopped() && !OS::CThread::IsRunning(); }
  bool isInterruptionRequested() { return OS::CThread::IsStopped() && OS::CThread::IsRunning(); }
  void start() { OS::CThread::StartThread(true); }
  void requestInterruption() { OS::CThread::StopThread(false); }
  bool waitFinished() { return OS::CThread::WaitThread(-1); }

private:
  void * Process() override;
  PASource * m_source;
};

}

PASource::PASource(const std::string& name, const std::string& deviceName)
: AudioSource()
, m_name(name)
, m_deviceName(deviceName)
, m_format(AudioFormat::CDLPCM())
, m_pa_error(0)
, m_pa(nullptr)
, m_blankKiller(nullptr)
, m_p(new PASourceWorker(this))

{
}

PASource::~PASource()
{
  close();
  delete m_p;
  freePA();
}

bool PASource::open(OpenMode mode)
{
  bool ok = AudioSource::open(mode);
  if (ok)
    m_p->start();
  return ok;
}

void PASource::close()
{
  if (m_p->isRunning())
  {
    m_p->requestInterruption();
    m_p->waitFinished();
  }
  AudioSource::close();
}

bool PASource::initPA()
{
  DBG(DBG_INFO, "Open PA session\n");
  /* The sample type to use */
  pa_sample_spec ss;

  // Reset the blank killer
  m_blankKiller = &PCMBlankKillerNull;

  ss.format = PA_SAMPLE_INVALID;
  if (m_format.sampleSize == 8 && m_format.sampleType == AudioFormat::UnSignedInt)
  {
    ss.format = PA_SAMPLE_U8;
  }
  else if (m_format.byteOrder == AudioFormat::LittleEndian &&
          m_format.sampleType == AudioFormat::SignedInt)
  {
    switch (m_format.sampleSize)
    {
    case 16:
      ss.format = PA_SAMPLE_S16LE;
      m_blankKiller = &PCMBlankKillerS16LE;
      break;
    case 24:
      ss.format = PA_SAMPLE_S24LE;
      m_blankKiller = &PCMBlankKillerS24LE;
      break;
    case 32:
      ss.format = PA_SAMPLE_S32LE;
      m_blankKiller = &PCMBlankKillerS32LE;
      break;
    default:
      break;
    }
  }
  else if (m_format.byteOrder == AudioFormat::BigEndian &&
          m_format.sampleType == AudioFormat::SignedInt)
  {
    switch (m_format.sampleSize)
    {
    case 16:
      ss.format = PA_SAMPLE_S16BE;
      m_blankKiller = &PCMBlankKillerS16BE;
      break;
    case 24:
      ss.format = PA_SAMPLE_S24BE;
      m_blankKiller = &PCMBlankKillerS24BE;
      break;
    case 32:
      ss.format = PA_SAMPLE_S32BE;
      m_blankKiller = &PCMBlankKillerS32BE;
      break;
    default:
      break;
    }
  }

  if (ss.format == PA_SAMPLE_INVALID)
  {
    DBG(DBG_WARN, "ERROR: Audio format not supported: %d%s%s\n", m_format.sampleSize,
          m_format.sampleType == AudioFormat::SignedInt ? "S" : "U",
          m_format.sampleSize > 8 ? m_format.byteOrder == AudioFormat::LittleEndian ? "LE" : "BE" : "");
    return false;
  }
  ss.rate = m_format.sampleRate;
  ss.channels = m_format.channelCount;
  m_pa = pa_simple_new(NULL, m_name.c_str(), PA_STREAM_RECORD, m_deviceName.c_str(), "record", &ss, NULL, NULL, &m_pa_error);
  return (m_pa != nullptr);
}

void PASource::freePA()
{
  if (m_pa)
  {
    DBG(DBG_INFO, "Close PA session\n");
    pa_simple_free(m_pa);
    m_pa = nullptr;
  }
}

PASourceWorker::PASourceWorker(PASource* source)
: OS::CThread()
, m_source(source)
{
}

PASourceWorker::~PASourceWorker()
{
  if (IsRunning())
    StopThread(true);
}

void* PASourceWorker::Process()
{
  if (m_source->initPA())
  {
    int channels = m_source->m_format.channelCount;
    int bytesPerFrame = m_source->m_format.bytesPerFrame();
    assert(bytesPerFrame >= MIN_FRAME_SIZE && bytesPerFrame <= MAX_FRAME_SIZE);
    int bsize = bytesPerFrame * FRAME_BUFFER;
    char * buf = new char[bsize];
    while (!OS::CThread::IsStopped())
    {
      // Record some data
      if (pa_simple_read(m_source->m_pa, buf, bsize, &m_source->m_pa_error) < 0)
      {
        DBG(DBG_ERROR, "pa_simple_read() failed: %s\n", pa_strerror(m_source->m_pa_error));
        break;
      }
      // Apply the blank killer
      m_source->m_blankKiller(buf, channels, BLANK_FRAMES);
      // And write it to out
      if (m_source->writeData(buf, bsize) != bsize)
      {
        DBG(DBG_ERROR, "write() failed\n");
        break;
      }
    }
    delete [] buf;
    m_source->freePA();
  }
  return nullptr;
}
