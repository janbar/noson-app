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

#include "audioencoder.h"
#include "private/debug.h"

using namespace NSROOT;

AudioEncoder::AudioEncoder()
: IODevice()
, m_format()
, m_source(nullptr)
{
}

void AudioEncoder::setAudioFormat(const AudioFormat& format)
{
  if (IODevice::isOpen())
  {
    DBG(DBG_WARN, "Audio encoder is already opened\n");
    close();
  }
  m_format = format;
}

void AudioEncoder::setSource(IODevice* source)
{
  if (m_source)
    m_source->connectOutput(nullptr);
  if (source)
    source->connectOutput(this);
  m_source = source;
}

int AudioEncoder::writeData(const char * data, int len)
{
  return encode(data, len);
}
