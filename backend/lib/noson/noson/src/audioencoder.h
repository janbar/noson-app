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
#ifndef AUDIOENCODER_H
#define AUDIOENCODER_H

#include "local_config.h"
#include "iodevice.h"
#include "audioformat.h"
#include "streambuffer.h"

namespace NSROOT
{

class AudioEncoder : public IODevice
{
public:
  AudioEncoder();
  virtual ~AudioEncoder() { }

  void setAudioFormat(const AudioFormat& format);
  AudioFormat audioFormat() const { return m_format; }

  void setSource(IODevice * source);
  IODevice * source() { return m_source; }

  virtual std::string mediaType() const = 0;
  virtual bool open(OpenMode mode = ReadWrite) override { return IODevice::open(mode); }

  void close() override { onClose(); IODevice::close(); }
  bool isSequential() const override { return true; }

protected:
  AudioFormat m_format;
  IODevice * m_source;

  virtual void onClose() = 0; // will run on close
  virtual int encode(const char * data, int len) = 0;
  virtual int writeData(const char *data, int len) override;
};

}

#endif /* AUDIOENCODER_H */

