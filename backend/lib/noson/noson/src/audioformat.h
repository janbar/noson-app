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
#ifndef AUDIOFORMAT_H
#define AUDIOFORMAT_H

#include "local_config.h"

#include <string>

namespace NSROOT
{

struct AudioFormat
{
  enum { LittleEndian, BigEndian } byteOrder;
  enum { SignedInt, UnSignedInt } sampleType;
  uint8_t sampleSize;
  uint32_t sampleRate;
  uint8_t channelCount;
  std::string codec;

  AudioFormat()
  : byteOrder(LittleEndian)
  , sampleType(SignedInt)
  , sampleSize(0)
  , sampleRate(0)
  , channelCount(0)
  , codec()
  { }

  bool isValid()
  {
    return sampleRate && sampleSize && channelCount && !codec.empty();
  }

  int bytesPerFrame()
  {
    return (int)channelCount * ((sampleSize + 7) / 8);
  }

  static AudioFormat CDLPCM()
  {
    AudioFormat af;
    af.byteOrder = LittleEndian;
    af.sampleType = SignedInt;
    af.sampleSize = 16;
    af.sampleRate = 44100;
    af.channelCount = 2;
    af.codec = "audio/pcm";
    return af;
  }
};

}

#endif /* AUDIOFORMAT_H */

