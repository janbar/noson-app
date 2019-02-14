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
#include "flacencoder.h"
#include "streambuffer.h"
#include "private/byteorder.h"
#include "private/debug.h"

#define SAMPLES 1024
#define BUFFER_SIZE 0x10000

using namespace NSROOT;

FLACEncoder::FLACEncoder()
: FLACEncoder(BUFFER_SIZE)
{
}

FLACEncoder::FLACEncoder(int buffered)
: AudioEncoder()
, m_ok(false)
, m_bytesPerFrame(0)
, m_sampleSize(0)
, m_pcm(nullptr)
, m_buffer(nullptr)
, m_encoder(this)
{
  m_buffer = new StreamBuffer(buffered);
}

FLACEncoder::~FLACEncoder()
{
  if (AudioEncoder::isOpen())
    AudioEncoder::close();
  if (m_pcm != nullptr)
    delete[] m_pcm;
  delete m_buffer;
}

bool FLACEncoder::open(OpenMode mode)
{
  if (AudioEncoder::isOpen())
  {
    DBG(DBG_WARN, "FLAC Encoder already opened\n");
    return false;
  }

  DBG(DBG_INFO, "Open FLAC encoder\n");

  // configure the encoder
  if (!(m_ok = m_format.isValid()))
    DBG(DBG_WARN, "ERROR: Invalid format\n");
  else if (!(m_ok = m_encoder.set_verify(true)))
    DBG(DBG_WARN, "ERROR: Set verify failed\n");
  else if (!(m_ok = m_encoder.set_compression_level(5)))
    DBG(DBG_WARN, "ERROR: Set compression level failed\n");
  else if (!(m_ok = m_encoder.set_channels(m_format.channelCount)))
    DBG(DBG_WARN, "ERROR: Set channels (%d) failed\n", m_format.channelCount);
  else if (!(m_ok = m_encoder.set_bits_per_sample(m_format.sampleSize)))
    DBG(DBG_WARN, "ERROR: Set sample size (%d) failed\n", m_format.sampleSize);
  else if (!(m_ok = m_encoder.set_sample_rate(m_format.sampleRate)))
    DBG(DBG_WARN, "ERROR: Set sample rate (%d) failed\n", m_format.sampleRate);
  else if (!(m_ok = (m_format.sampleSize == 8 && m_format.sampleType == AudioFormat::UnSignedInt) ||
          (m_format.sampleSize == 16 && m_format.sampleType == AudioFormat::SignedInt && m_format.byteOrder == AudioFormat::LittleEndian) ||
          (m_format.sampleSize == 24 && m_format.sampleType == AudioFormat::SignedInt && m_format.byteOrder == AudioFormat::LittleEndian)
          //(m_format.sampleSize == 32 && m_format.sampleType == AudioFormat::SignedInt && m_format.byteOrder == AudioFormat::LittleEndian)
          ))
    DBG(DBG_WARN, "ERROR: Audio format not supported: %d%s%s\n", m_format.sampleSize,
            m_format.sampleType == AudioFormat::SignedInt ? "S" : "U",
            m_format.sampleSize > 8 ? m_format.byteOrder == AudioFormat::LittleEndian ? "LE" : "BE" : "");

  if (!m_ok)
    return false;

  m_bytesPerFrame = m_format.bytesPerFrame();
  m_sampleSize = m_format.sampleSize;
  m_pcm = new FLAC__int32 [SAMPLES * m_format.channelCount];
  m_buffer->clear();

  AudioEncoder::open(mode);
  FLAC__StreamEncoderInitStatus init_status = m_encoder.init();
  if(init_status == FLAC__STREAM_ENCODER_INIT_STATUS_OK)
    return true;
  AudioEncoder::close();
  DBG(DBG_WARN, "ERROR: initializing FLAC encoder: %s\n", FLAC__StreamEncoderInitStatusString[init_status]);
  return false;
}

int FLACEncoder::bytesAvailable() const
{
  return m_buffer->size();
}

int FLACEncoder::readData(char * data, int maxlen)
{
  return m_buffer->read(data, maxlen);
}

void FLACEncoder::onClose()
{
  if (AudioEncoder::isOpen())
  {
    DBG(DBG_INFO, "Close FLAC encoder\n");
    m_encoder.finish();
  }
}

int FLACEncoder::encode(const char * data, int len)
{
  bool ok = true;
  int samples = len / m_bytesPerFrame;
  while (ok && samples > 0)
  {
    int need = (samples > SAMPLES ? SAMPLES : static_cast<int>(samples));
    // convert the packed little-endian PCM samples into an interleaved FLAC__int32 buffer for libFLAC
    for(int i = 0; i < (need * m_format.channelCount); i++)
    {
      switch (m_sampleSize)
      {
      case 8:
        m_pcm[i] = (unsigned char)(*data) - 128;
        data += 1;
        break;
      case 16:
        m_pcm[i] = read16le(data);
        data += 2;
        break;
      case 24:
        m_pcm[i] = read24le(data);
        data += 3;
        break;
      case 32:
        m_pcm[i] = read32le(data);
        data += 4;
        break;
      default:
        m_pcm[i] = 0;
      }
    }
    // feed samples to encoder
    ok = m_encoder.process_interleaved(m_pcm, need);
    samples -= need;
  }
  return len;
}

FLAC__StreamEncoderWriteStatus FLACEncoder::FLACEncoderPrivate::write_callback(const FLAC__byte buffer[], size_t bytes, unsigned samples, unsigned current_frame)
{
  DBG(DBG_DEBUG, "FLAC encoder wrote %" PRIu64 " bytes, %u samples, %u frame\n", bytes, samples, current_frame);
  int r;
  if (m_p->m_out)
  {
    r = m_p->m_out->write((char*)buffer, (int)bytes);
  }
  else
  {
    r = m_p->m_buffer->write((char*)buffer, (int)bytes);
    if (r > 0)
      m_p->readyRead();
  }
  return (r == (int)bytes ? FLAC__STREAM_ENCODER_WRITE_STATUS_OK : FLAC__STREAM_ENCODER_WRITE_STATUS_FATAL_ERROR);
}
