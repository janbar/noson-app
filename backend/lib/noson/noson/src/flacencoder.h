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
#ifndef FLACENCODER_H
#define FLACENCODER_H

#include "local_config.h"
#include "audioencoder.h"

#include <FLAC++/metadata.h>
#include <FLAC++/encoder.h>

namespace NSROOT
{

class StreamBuffer;

class FLACEncoder : public AudioEncoder
{
  friend class FLACEncoderPrivate;

public:
  FLACEncoder();
  FLACEncoder(int buffered);
  virtual ~FLACEncoder() override;

  virtual std::string mediaType() const override { return "audio/x-flac"; }

  virtual bool open(OpenMode mode = ReadWrite) override;
  virtual int bytesAvailable() const override;

protected:
  virtual int readData(char * data, int maxlen) override;

private:
  void onClose() override;
  int encode(const char * data, int len) override;

private:
  bool m_ok;
  int m_bytesPerFrame;
  int m_sampleSize;
  FLAC__int32 * m_pcm;

  StreamBuffer * m_buffer;

  class FLACEncoderPrivate : public FLAC::Encoder::Stream
  {
  public:
    explicit FLACEncoderPrivate(FLACEncoder * p) : m_p(p) { }
    virtual FLAC__StreamEncoderWriteStatus write_callback(const FLAC__byte buffer[], size_t bytes, unsigned samples, unsigned current_frame) override;
  private:
    FLACEncoder * m_p;
  };
  
  FLACEncoderPrivate m_encoder;
};

}

#endif // FLACENCODER_H
