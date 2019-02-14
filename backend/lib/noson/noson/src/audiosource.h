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
#ifndef AUDIOSOURCE_H
#define AUDIOSOURCE_H

#include "local_config.h"
#include "iodevice.h"
#include "audioformat.h"

#include <string>

namespace NSROOT
{

class StreamBuffer;

class AudioSource : public IODevice
{
public:
  AudioSource();
  AudioSource(int buffered);
  virtual ~AudioSource();

  /**
   * Open the audio source. This allow read or write operations.
   * If output is connected then the write grant is controlled by the sink, and
   * read permission only can be assigned.
   * @param mode default by ReadWrite
   * @return true if succeeded, else false
   */
  virtual bool open(OpenMode mode = ReadWrite) override { return IODevice::open(mode); }

  /**
   * Close the audio source. This disallows read and write operations.
   */
  virtual void close() override { stopRecording(); IODevice::close(); }

  /**
   * Return the name of the source
   * @return the name
   */
  virtual std::string getName() const = 0;

  /**
   * Return the description of the source
   * @return the description
   */
  virtual std::string getDescription() const = 0;

  /**
   * Return the audio format that the source provides
   * @return the audio format
   */
  virtual AudioFormat getFormat() const = 0;


  inline bool isSequential() const override { return true; }
  inline bool isRecording() const { return m_record; }
  bool startRecording();
  void stopRecording();
  int bytesAvailable() const override;

protected:
  int readData(char * data, int maxlen) override;
  int writeData(const char *data, int len) override;

private:
  bool m_record;
  StreamBuffer * m_buffer;
};

}

#endif /* AUDIOSOURCE_H */

