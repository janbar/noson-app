/*
 *      Copyright (C) 2014-2015 Jean-Luc Barriere
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

#if defined __cplusplus
#define __STDC_LIMIT_MACROS
extern "C" {
#endif

#include "local_config.h"

#include <string.h>
#include <stdint.h>
#include <time.h>
#include <stdio.h>

#define string_to_int64 __str2int64
extern int string_to_int64(const char *str, int64_t *num);

#define string_to_int32 __str2int32
extern int string_to_int32(const char *str, int32_t *num);

#define string_to_int16 __str2int16
extern int string_to_int16(const char *str, int16_t *num);

#define string_to_int8 __str2int8
extern int string_to_int8(const char *str, int8_t *num);

#define string_to_uint32 __str2uint32
extern int string_to_uint32(const char *str, uint32_t *num);

#define string_to_uint16 __str2uint16
extern int string_to_uint16(const char *str, uint16_t *num);

#define string_to_uint8 __str2uint8
extern int string_to_uint8(const char *str, uint8_t *num);

#define int64_to_string __int64str
static CC_INLINE void int64_to_string(int64_t num, char *str)
{
  sprintf(str, "%lld", (long long)num);
}

#define int32_to_string __int32str
static CC_INLINE void int32_to_string(int32_t num, char *str)
{
  sprintf(str, "%ld", (long)num);
}

#define int16_to_string __int16str
static CC_INLINE void int16_to_string(int16_t num, char *str)
{
  sprintf(str, "%d", num);
}

#define int8_to_string __int8str
static CC_INLINE void int8_to_string(int8_t num, char *str)
{
  sprintf(str, "%d", num);
}

#define uint32_to_string __uint32str
static CC_INLINE void uint32_to_string(uint32_t num, char *str)
{
  sprintf(str, "%lu", (unsigned long)num);
}

#define uint16_to_string __uint16str
static CC_INLINE void uint16_to_string(uint16_t num, char *str)
{
  sprintf(str, "%u", num);
}

#define uint8_to_string __uint8str
static CC_INLINE void uint8_to_string(uint8_t num, char *str)
{
  sprintf(str, "%u", num);
}

#define TIMESTAMP_UTC_LEN (sizeof("YYYY-MM-DDTHH:MM:SSZ") - 1)
#define TIMESTAMP_LEN     (sizeof("YYYY-MM-DDTHH:MM:SS") - 1)
#define DATESTAMP_LEN     (sizeof("YYYY-MM-DD") - 1)
#define INVALID_TIME      (time_t)(-1)

#if !HAVE_TIMEGM && !defined(timegm)
#define timegm __timegm
extern time_t timegm(struct tm *utctime_tm);
#endif

#if !HAVE_LOCALTIME_R && !defined(localtime_r)
#define localtime_r __localtime_r
static CC_INLINE struct tm *localtime_r(const time_t *clock, struct tm *result)
{
  struct tm *data;
  if (!clock || !result)
    return NULL;
  data = localtime(clock);
  if (!data)
    return NULL;
  memcpy(result, data, sizeof(*result));
  return result;
}
#endif

#if !HAVE_GMTIME_R && !defined(gmtime_r)
#define gmtime_r __gmtime_r
static CC_INLINE struct tm *gmtime_r(const time_t *clock, struct tm *result)
{
  struct tm *data;
  if (!clock || !result)
    return NULL;
  data = gmtime(clock);
  if (!data)
    return NULL;
  memcpy(result, data, sizeof(*result));
  return result;
}
#endif

#define string_to_time __str2time
extern int string_to_time(const char *str, time_t *time);

#define time_to_iso8601utc __time2iso8601utc
extern void time_to_iso8601utc(time_t time, char *str);

#define time_to_iso8601 __time2iso8601
extern void time_to_iso8601(time_t time, char *str);

#define time_to_isodate __time2isodate
extern void time_to_isodate(time_t time, char *str);

typedef struct { int tz_dir; int tz_hour; int tz_min; char tz_str[7]; } tz_t;
#define time_tz __timetz
extern tz_t *time_tz(time_t time, tz_t* tz);

#ifdef __cplusplus
}
#endif

#endif	/* BUILTIN_H */
