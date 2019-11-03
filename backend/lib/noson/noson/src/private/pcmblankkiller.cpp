/*
 *      Copyright (C) 2019 Jean-Luc Barriere
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

#include "pcmblankkiller.h"
#include "byteorder.h"

#include <cinttypes>

#define PCM_KILLER_LEVEL    1
#define ZEROS   0
#define ZEROU8  0x80
#define ZEROU16 0x8000
#define ZEROU24 0x800000
#define ZEROU32 0x80000000

namespace NSROOT
{

void PCMBlankKillerNull(void * buf, int channels, int frames)
{
  (void)buf;
  (void)channels;
  (void)frames;
}

void PCMBlankKillerU8(void * buf, int channels, int frames)
{
  uint8_t * p = (uint8_t*)buf;
  uint8_t * e = p + frames * channels;
  int v = 0;
  while (p < e) { v |= ((*p++) - ZEROU8); }
  if (v == 0)
  {
    p = (uint8_t*)buf;
    for (int c = 0; c < channels; ++c)
    {
      write8(p + c, (int8_t)(ZEROU8 + PCM_KILLER_LEVEL));
      write8(p + channels + c, (int8_t)(ZEROU8 - PCM_KILLER_LEVEL));
    }
  }
}

/* LITTLE ENDIAN */

void PCMBlankKillerS16LE(void * buf, int channels, int frames)
{
  int16_t * p = (int16_t*)buf;
  int16_t * e = p + frames * channels;
  int v = 0;
  while (p < e) { v |= (read16le(p++) - ZEROS); }
  if (v == 0)
  {
    p = (int16_t*)buf;
    for (int c = 0; c < channels; ++c)
    {
      write16le(p + c, (ZEROS + (PCM_KILLER_LEVEL << 4)));
      write16le(p + channels + c, (ZEROS - (PCM_KILLER_LEVEL << 4)));
    }
  }
}

void PCMBlankKillerU16LE(void * buf, int channels, int frames)
{
  uint16_t * p = (uint16_t*)buf;
  uint16_t * e = p + frames * channels;
  int v = 0;
  while (p < e) { v |= ((uint16_t)read16le(p++) - ZEROU16); }
  if (v == 0)
  {
    p = (uint16_t*)buf;
    for (int c = 0; c < channels; ++c)
    {
      write16le(p + c, (int16_t)(ZEROU16 + (PCM_KILLER_LEVEL << 4)));
      write16le(p + channels + c, (int16_t)(ZEROU16 - (PCM_KILLER_LEVEL << 4)));
    }
  }
}

void PCMBlankKillerS24LE(void * buf, int channels, int frames)
{
  int8_t * p = (int8_t*)buf;
  int8_t * e = p + 3 * frames * channels;
  int v = 0;
  while (p < e) { v |= ((read32le(p) >> 8) - ZEROS); p += 3; }
  if (v == 0)
  {
    p = (int8_t*)buf;
    for (int c = 0; c < channels; ++c)
      write32le(p + 3 * c, (ZEROS + (PCM_KILLER_LEVEL << 8)));
    for (int c = 0; c < channels; ++c)
      write32le(p + 3 * (channels + c), (ZEROS - (PCM_KILLER_LEVEL << 8)));
    write8(p + 3 * 2 * channels, (int8_t)(ZEROS & 0xff));
  }
}

void PCMBlankKillerU24LE(void * buf, int channels, int frames)
{
  uint8_t * p = (uint8_t*)buf;
  uint8_t * e = p + 3 * frames * channels;
  int v = 0;
  while (p < e) { v |= (((uint32_t)read32le(p) >> 8) - ZEROU24); p += 3; }
  if (v == 0)
  {
    p = (uint8_t*)buf;
    for (int c = 0; c < channels; ++c)
      write32le(p + 3 * c, (int32_t)(ZEROU24 + (PCM_KILLER_LEVEL << 8)));
    for (int c = 0; c < channels; ++c)
      write32le(p + 3 * (channels + c), (int32_t)(ZEROU24 - (PCM_KILLER_LEVEL << 8)));
    write8(p + 3 * 2 * channels, (int8_t)(ZEROU24 & 0xff));
  }
}

void PCMBlankKillerS32LE(void * buf, int channels, int frames)
{
  int32_t * p = (int32_t*)buf;
  int32_t * e = p + frames * channels;
  int v = 0;
  while (p < e) { v |= (read32le(p++) - ZEROS); }
  if (v == 0)
  {
    p = (int32_t*)buf;
    for (int c = 0; c < channels; ++c)
    {
      write32le(p + c, (ZEROS + (PCM_KILLER_LEVEL << 16)));
      write32le(p + channels + c, (ZEROS - (PCM_KILLER_LEVEL << 16)));
    }
  }
}

void PCMBlankKillerU32LE(void * buf, int channels, int frames)
{
  uint32_t * p = (uint32_t*)buf;
  uint32_t * e = p + frames * channels;
  int v = 0;
  while (p < e) { v |= ((uint32_t)read32le(p++) - ZEROU32); }
  if (v == 0)
  {
    p = (uint32_t*)buf;
    for (int c = 0; c < channels; ++c)
    {
      write32le(p + c, (int32_t)(ZEROU32 + (PCM_KILLER_LEVEL << 16)));
      write32le(p + channels + c, (int32_t)(ZEROU32 - (PCM_KILLER_LEVEL << 16)));
    }
  }
}

/* BIG ENDIAN */

void PCMBlankKillerS16BE(void * buf, int channels, int frames)
{
  int16_t * p = (int16_t*)buf;
  int16_t * e = p + frames * channels;
  int v = 0;
  while (p < e) { v |= (read16be(p++) - ZEROS); }
  if (v == 0)
  {
    p = (int16_t*)buf;
    for (int c = 0; c < channels; ++c)
    {
      write16be(p + c, (ZEROS + (PCM_KILLER_LEVEL << 4)));
      write16be(p + channels + c, (ZEROS - (PCM_KILLER_LEVEL << 4)));
    }
  }
}

void PCMBlankKillerU16BE(void * buf, int channels, int frames)
{
  uint16_t * p = (uint16_t*)buf;
  uint16_t * e = p + frames * channels;
  int v = 0;
  while (p < e) { v |= ((uint16_t)read16be(p++) - ZEROU16); }
  if (v == 0)
  {
    p = (uint16_t*)buf;
    for (int c = 0; c < channels; ++c)
    {
      write16be(p + c, (int16_t)(ZEROU16 + (PCM_KILLER_LEVEL << 4)));
      write16be(p + channels + c, (int16_t)(ZEROU16 - (PCM_KILLER_LEVEL << 4)));
    }
  }
}

void PCMBlankKillerS24BE(void * buf, int channels, int frames)
{
  int8_t * p = (int8_t*)buf;
  int8_t * e = p + 3 * frames * channels;
  int v = 0;
  while (p < e) { v |= ((read32be(p) >> 8) - ZEROS); p += 3; }
  if (v == 0)
  {
    p = (int8_t*)buf;
    for (int c = 0; c < channels; ++c)
      write32be(p + 3 * c, (ZEROS + (PCM_KILLER_LEVEL << 8)) << 8);
    for (int c = 0; c < channels; ++c)
      write32be(p + 3 * (channels + c), (uint32_t)(ZEROS - (PCM_KILLER_LEVEL << 8)) << 8);
    write8(p + 3 * 2 * channels, (int8_t)((ZEROS & 0x00ff0000) >> 16));
  }
}

void PCMBlankKillerU24BE(void * buf, int channels, int frames)
{
  uint8_t * p = (uint8_t*)buf;
  uint8_t * e = p + 3 * frames * channels;
  int v = 0;
  while (p < e) { v |= (((uint32_t)read32be(p) >> 8) - ZEROU24); p += 3; }
  if (v == 0)
  {
    p = (uint8_t*)buf;
    for (int c = 0; c < channels; ++c)
      write32be(p + 3 * c, (int32_t)(ZEROU24 + (PCM_KILLER_LEVEL << 8)) << 8);
    for (int c = 0; c < channels; ++c)
      write32be(p + 3 * (channels + c), (int32_t)(ZEROU24 - (PCM_KILLER_LEVEL << 8)) << 8);
    write8(p + 3 * 2 * channels, (int8_t)((ZEROU24 & 0x00ff0000) >> 16));
  }
}

void PCMBlankKillerS32BE(void * buf, int channels, int frames)
{
  int32_t * p = (int32_t*)buf;
  int32_t * e = p + frames * channels;
  int v = 0;
  while (p < e) { v |= (read32be(p++) - ZEROS); }
  if (v == 0)
  {
    p = (int32_t*)buf;
    for (int c = 0; c < channels; ++c)
    {
      write32be(p + c, (ZEROS + (PCM_KILLER_LEVEL << 16)));
      write32be(p + channels + c, (ZEROS - (PCM_KILLER_LEVEL << 16)));
    }
  }
}

void PCMBlankKillerU32BE(void * buf, int channels, int frames)
{
  uint32_t * p = (uint32_t*)buf;
  uint32_t * e = p + frames * channels;
  int v = 0;
  while (p < e) { v |= ((uint32_t)read32be(p++) - ZEROU32); }
  if (v == 0)
  {
    p = (uint32_t*)buf;
    for (int c = 0; c < channels; ++c)
    {
      write32be(p + c, (int32_t)(ZEROU32 + (PCM_KILLER_LEVEL << 16)));
      write32be(p + channels + c, (int32_t)(ZEROU32 - (PCM_KILLER_LEVEL << 16)));
    }
  }
}

}
