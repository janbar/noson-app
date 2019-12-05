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
#include <climits>
#include <cerrno>

static inline int string_to_int8(const char *str, int8_t *num)
{
  int v = atoi(str);
  if (v > INT8_MAX || v < INT8_MIN)
    return -(ERANGE);
  *num = (int8_t) v;
  return 0;
}

static inline int string_to_uint8(const char *str, uint8_t *num)
{
  int v = atoi(str);
  if (v > UINT8_MAX || v < 0)
    return -(ERANGE);
  *num = (uint8_t) v;
  return 0;
}

static inline int string_to_int16(const char *str, int16_t *num)
{
  int v = atoi(str);
  if (v > INT16_MAX || v < INT16_MIN)
    return -(ERANGE);
  *num = (int16_t) v;
  return 0;
}

static inline int string_to_uint16(const char *str, uint16_t *num)
{
  int v = atoi(str);
  if (v > UINT16_MAX || v < 0)
    return -(ERANGE);
  *num = (uint16_t) v;
  return 0;
}

static inline int string_to_int32(const char *str, int32_t *num)
{
  long long v = atoll(str);
  if (v > INT32_MAX || v < INT32_MIN)
    return -(ERANGE);
  *num = (int32_t) v;
  return 0;
}

static inline int string_to_uint32(const char *str, uint32_t *num)
{
  long long v = atoll(str);
  if (v > UINT32_MAX || v < 0)
    return -(ERANGE);
  *num = (uint32_t) v;
  return 0;
}

#endif	/* BUILTIN_H */
