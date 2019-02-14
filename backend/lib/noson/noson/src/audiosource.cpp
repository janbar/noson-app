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
#include "audiosource.h"
#include "streambuffer.h"

using namespace NSROOT;

#define BUFFER_SIZE 0x10000

AudioSource::AudioSource()
: AudioSource(BUFFER_SIZE)
{
}

AudioSource::AudioSource(int buffered)
: m_record(false)
, m_buffer(new StreamBuffer(buffered))
{
}

AudioSource::~AudioSource()
{
  delete m_buffer;
}

bool AudioSource::startRecording()
{
  m_buffer->clear();
  m_record = true;
  return true;
}

void AudioSource::stopRecording()
{
  m_record = false;
}

int AudioSource::bytesAvailable() const
{
  return m_buffer->size();
}

int AudioSource::readData(char * data, int maxlen)
{
  return m_buffer->read(data, maxlen);
}

int AudioSource::writeData(const char * data, int len)
{
  if (m_record)
  {
    if (m_out)
    {
      len = m_out->write(data, len);
    }
    else
    {
      if ((len = m_buffer->write(data, len)) > 0)
        readyRead();
    }
  }
  return len;
}
