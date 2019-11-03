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

#ifndef BYTEORDER_H
#define BYTEORDER_H

#include <cinttypes>
#include <cstring>

#ifndef BYTE_ORDER
#define LITTLE_ENDIAN     1234
#define BIG_ENDIAN        4321
extern int __endianess__;
#define BYTE_ORDER        __endianess__
#endif
#define is_big_endian     (BYTE_ORDER == BIG_ENDIAN)
#define is_little_endian  (BYTE_ORDER == LITTLE_ENDIAN)


template <typename T> inline T fromUnaligned(const void * data)
{
  T val;
  memcpy(&val, data, sizeof(T));
  return val;
}

template <typename T> inline void toUnaligned(void * data, T val)
{
  memcpy(data, &val, sizeof(T));
}

static inline int8_t read8(const void * data)
{
  return fromUnaligned<int8_t>(data);
}

static inline void write8(void * data, int8_t val)
{
  toUnaligned(data, val);
}

static inline int16_t swap16(int16_t val)
{
  return (int16_t)(0 |
          (((uint32_t)val & 0x00ff) << 8) |
          (((uint32_t)val & 0xff00) >> 8));
}

static inline int32_t swap32(int32_t val)
{
  return (int32_t)(0 |
          (((uint32_t)val & 0x000000ff) << 24) |
          (((uint32_t)val & 0x0000ff00) << 8 ) |
          (((uint32_t)val & 0x00ff0000) >> 8 ) |
          (((uint32_t)val & 0xff000000) >> 24)
          );
}

static inline int16_t read16le(const void * data)
{
  int16_t val = fromUnaligned<int16_t>(data);
  if (is_little_endian)
    return val;
  return swap16(val);
}

static inline void write16le(void * data, int16_t val)
{
  if (is_little_endian)
    toUnaligned(data, val);
  else
    toUnaligned(data, swap16(val));
}

static inline int16_t read16be(const void * data)
{
  int16_t val = fromUnaligned<int16_t>(data);
  if (is_big_endian)
    return val;
  return swap16(val);
}

static inline void write16be(void * data, int16_t val)
{
  if (is_big_endian)
    toUnaligned(data, val);
  else
    toUnaligned(data, swap16(val));
}

static inline int32_t read24le(const void * data)
{
  const char * p = (const char*)data;
  uint8_t val = fromUnaligned<uint8_t>(&p[0]);
  return ((int32_t)read16le(&p[1]) * 256) | (val & 0xff);
}

static inline int32_t read24be(const void * data)
{
  const char * p = (const char*)data;
  uint8_t val = fromUnaligned<uint8_t>(&p[2]);
  return ((int32_t)read16be(&p[1]) * 256) | (val & 0xff);
}

static inline int32_t read32le(const void * data)
{
  int32_t val = fromUnaligned<int32_t>(data);
  if (is_little_endian)
    return val;
  return swap32(val);
}

static inline void write32le(void * data, int32_t val)
{
  if (is_little_endian)
    toUnaligned(data, val);
  else
    toUnaligned(data, swap32(val));
}

static inline int32_t read32be(const void * data)
{
  int32_t val = fromUnaligned<int32_t>(data);
  if (is_big_endian)
    return val;
  return swap32(val);
}

static inline void write32be(void * data, int32_t val)
{
  if (is_big_endian)
    toUnaligned(data, val);
  else
    toUnaligned(data, swap32(val));
}

#endif /* BYTEORDER_H */

