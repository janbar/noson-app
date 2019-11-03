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

#ifndef AUDIOSTREAM_H
#define AUDIOSTREAM_H

#include "local_config.h"
#include "audiosource.h"
#include "audioencoder.h"

#include <string>

namespace NSROOT
{

class AudioStream
{
public:
  AudioStream(AudioSource& source, AudioEncoder& encoder) : m_source(source), m_encoder(encoder) { }
  virtual ~AudioStream() { }

  inline bool start()
  {
    if (m_source.isRecording())
      return false;
    m_encoder.setSource(&m_source);
    m_encoder.setAudioFormat(m_source.getFormat());
    bool started = (m_encoder.open(IODevice::ReadWrite) &&
            m_source.open(IODevice::ReadOnly) &&
            m_source.startRecording());
    if (!started)
      stop();
    return started;
  }

  inline void stop()
  {
    m_source.close();
    m_encoder.close();
  }

  inline int read(char * data, int maxlen, unsigned timeout)
  {
    return m_encoder.read(data, maxlen, timeout);
  }

private:
  AudioSource& m_source;
  AudioEncoder& m_encoder;
};

}

#endif /* AUDIOSTREAM_H */

