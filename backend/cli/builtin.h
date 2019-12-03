/*
 *      Copyright (C) 2019 Jean-Luc Barriere
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published
 *  by the Free Software Foundation; either version 3, or (at your option)
 *  any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this library; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 51 Franklin Street, Fifth Floor, Boston,
 *  MA 02110-1301 USA
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#ifndef BUILTIN_H
#define	BUILTIN_H

#include <cstdlib>
#include <cstdint>
#include <limits>

static inline int string_to_int8(const char *str, int8_t *num)
{
  int v = std::atoi(str);
  if (v > std::numeric_limits<int8_t>::max() || v < std::numeric_limits<int8_t>::min())
    return 1;
  *num = (int8_t) v;
  return 0;
}

static inline int string_to_uint8(const char *str, uint8_t *num)
{
  int v = std::atoi(str);
  if (v > std::numeric_limits<uint8_t>::max() || v < 0)
    return 1;
  *num = (uint8_t) v;
  return 0;
}

static inline int string_to_int16(const char *str, int16_t *num)
{
  int v = std::atoi(str);
  if (v > std::numeric_limits<int16_t>::max() || v < std::numeric_limits<int16_t>::min())
    return 1;
  *num = (int16_t) v;
  return 0;
}

static inline int string_to_uint16(const char *str, uint16_t *num)
{
  int v = std::atoi(str);
  if (v > std::numeric_limits<uint16_t>::max() || v < 0)
    return 1;
  *num = (uint16_t) v;
  return 0;
}

static inline int string_to_int32(const char *str, int32_t *num)
{
  long long v = std::atoll(str);
  if (v > std::numeric_limits<int32_t>::max() || v < std::numeric_limits<int32_t>::min())
    return 1;
  *num = (int32_t) v;
  return 0;
}

static inline int string_to_uint32(const char *str, uint32_t *num)
{
  long long v = std::atoll(str);
  if (v > std::numeric_limits<uint32_t>::max() || v < 0)
    return 1;
  *num = (uint32_t) v;
  return 0;
}

#endif	/* BUILTIN_H */
