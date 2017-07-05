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

#include "builtin.h"

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

int string_to_int64(const char *str, int64_t *num)
{
  uint64_t val = 0;
  int sign = 1;
  uint64_t limit = INT64_MAX;

  if (str == NULL)
    return -(EINVAL);
  while (isspace(*str))
  {
    ++str;
  }

  if (*str && (*str == '-'))
  {
    ++str;
    sign = -1;
  }
  while (*str && !isspace(*str))
  {
    if (!isdigit(*str))
      return -(EINVAL);
    val *= 10;
    val += ((*str) - '0');
    /*
     * Check and make sure we are still under the limit (this is
     * an absolute value limit, sign will be applied later).
     */
    if (val > limit)
      return -(ERANGE);
    str++;
  }

  *num = (int64_t) (sign * val);
  return 0;
}

int string_to_int32(const char *str, int32_t *num)
{
  uint64_t val = 0;
  int sign = 1;
  uint64_t limit = INT32_MAX;

  if (str == NULL)
    return -(EINVAL);
  while (isspace(*str))
  {
    ++str;
  }

  if (*str && (*str == '-'))
  {
    ++str;
    sign = -1;
  }
  while (*str && !isspace(*str))
  {
    if (!isdigit(*str))
      return -(EINVAL);
    val *= 10;
    val += ((*str) - '0');
    /*
     * Check and make sure we are still under the limit (this is
     * an absolute value limit, sign will be applied later).
     */
    if (val > limit)
      return -(ERANGE);
    str++;
  }

  *num = (int32_t) (sign * val);
  return 0;
}

int string_to_int16(const char *str, int16_t *num)
{
  int32_t val;
  int err;

  if ((err = string_to_int32(str, &val)))
    return err;
  if (val > 32767 || val < -32768)
    return -(ERANGE);

  *num = (int16_t) val;
  return 0;
}

int string_to_int8(const char *str, int8_t *num)
{
  int32_t val;
  int err;

  if ((err = string_to_int32(str, &val)))
    return err;
  if (val > 127 || val < -128)
    return -(ERANGE);

  *num = (int8_t) val;
  return 0;
}

int string_to_uint32(const char *str, uint32_t *num)
{
  uint64_t val = 0;
  uint64_t limit = UINT32_MAX;

  if (str == NULL)
    return -(EINVAL);
  while (isspace(*str))
  {
    ++str;
  }

  while (*str && !isspace(*str))
  {
    if (!isdigit(*str))
      return -(EINVAL);
    val *= 10;
    val += ((*str) - '0');
    /*
     * Check and make sure we are still under the limit (this is
     * an absolute value limit, sign will be applied later).
     */
    if (val > limit)
      return -(ERANGE);
    str++;
  }

  *num = (uint32_t) val;
  return 0;
}

int string_to_uint16(const char *str, uint16_t *num)
{
  uint32_t val;
  int err;

  if ((err = string_to_uint32(str, &val)))
    return err;
  if (val > 65535)
    return -(ERANGE);

  *num = (uint16_t) val;
  return 0;
}

int string_to_uint8(const char *str, uint8_t *num)
{
  uint32_t val;
  int err;

  if ((err = string_to_uint32(str, &val)))
    return err;
  if (val > 255)
    return -(ERANGE);

  *num = (uint8_t) val;
  return 0;
}

time_t __timegm(struct tm *utctime_tm)
{
  time_t time;
  struct tm adj_tm, chk_tm;

  adj_tm.tm_year = utctime_tm->tm_year;
  adj_tm.tm_mon = utctime_tm->tm_mon;
  adj_tm.tm_mday = utctime_tm->tm_mday;
  adj_tm.tm_hour = utctime_tm->tm_hour;
  adj_tm.tm_min = utctime_tm->tm_min;
  adj_tm.tm_sec = utctime_tm->tm_sec;
  adj_tm.tm_isdst = -1;

  for (;;)
  {
    time = mktime(&adj_tm);
    if (time == INVALID_TIME)
      return time;
    if (NULL == gmtime_r(&time, &chk_tm))
      return INVALID_TIME;
    if (chk_tm.tm_min == utctime_tm->tm_min &&
            chk_tm.tm_hour == utctime_tm->tm_hour &&
            chk_tm.tm_mday == utctime_tm->tm_mday &&
            chk_tm.tm_mon == utctime_tm->tm_mon &&
            chk_tm.tm_year == utctime_tm->tm_year)
      break;
    adj_tm.tm_min += utctime_tm->tm_min - chk_tm.tm_min;
    adj_tm.tm_hour += utctime_tm->tm_hour - chk_tm.tm_hour;
    adj_tm.tm_mday += utctime_tm->tm_mday - chk_tm.tm_mday;
    adj_tm.tm_mon += utctime_tm->tm_mon - chk_tm.tm_mon;
    adj_tm.tm_year += utctime_tm->tm_year - chk_tm.tm_year;
  }
  return time;
}

int string_to_time(const char *str, time_t *time)
{
  struct tm time_tm;
  int i, len, format, isutc;
  char *yyyy, *MM, *dd, *hh, *mm, *ss;
  char buf[TIMESTAMP_UTC_LEN + 1];

  if (*str == '\0')
  {
    /* empty string */
    *time = INVALID_TIME;
    return 0;
  }
  memset(buf, 0, sizeof(buf));
  strncpy(buf, str, sizeof(buf) - 1);
  len = strlen(buf);

  switch (len)
  {
  case TIMESTAMP_UTC_LEN:
    if (((buf[4] != '-') || (buf[7] != '-') || (buf[10] != 'T') ||
            (buf[13] != ':') || (buf[16] != ':') || (buf[19] != 'Z')))
    {
      fprintf(stderr, "%s: string is badly formed '%s'\n", __FUNCTION__, buf);
      goto err;
    }
    format = 1;
    isutc = 1;
    break;
  case TIMESTAMP_LEN:
    if (((buf[4] != '-') || (buf[7] != '-') || (buf[10] != 'T') ||
            (buf[13] != ':') || (buf[16] != ':')))
    {
      fprintf(stderr, "%s: string is badly formed '%s'\n", __FUNCTION__, buf);
      goto err;
    }
    format = 2;
    isutc = 0;
    break;
  case DATESTAMP_LEN:
    if (((buf[4] != '-') || (buf[7] != '-')))
    {
      fprintf(stderr, "%s: string is badly formed '%s'\n", __FUNCTION__, buf);
      goto err;
    }
    format = 3;
    isutc = 0;
    break;
  default:
    fprintf(stderr, "%s: string is not a timestamp '%s'\n", __FUNCTION__, buf);
    goto err;
    break;
  }

  yyyy = buf;
  MM = buf + 5;
  dd = buf + 8;
  hh = buf + 11;
  mm = buf + 14;
  ss = buf + 17;

  yyyy[4] = MM[2] = dd[2] = hh[2] = mm[2] = ss[2] = '\0';

  for (i = 0; i < len; ++i)
  {
    if (buf[i] && !isdigit(buf[i]))
    {
      fprintf(stderr, "%s: expected numeral at '%s'[%d]\n", __FUNCTION__, str, i);
      goto err;
    }
  }

  time_tm.tm_isdst = -1;
  time_tm.tm_year = atoi(yyyy) - 1900;
  time_tm.tm_mon = atoi(MM) - 1;
  if (time_tm.tm_mon > 11)
  {
    fprintf(stderr, "%s: month value too big '%s'\n", __FUNCTION__, str);
    goto err;
  }
  time_tm.tm_mday = atoi(dd);
  if (time_tm.tm_mday > 31)
  {
    fprintf(stderr, "%s: day value too big '%s'\n", __FUNCTION__, str);
    goto err;

  }

  if (format == 3)
  {
    time_tm.tm_hour = time_tm.tm_min = time_tm.tm_sec = 0;
    *time = mktime(&time_tm);
    return 0;
  }

  time_tm.tm_hour = atoi(hh);
  if (time_tm.tm_hour > 23)
  {
    fprintf(stderr, "%s: hour value too big '%s'\n", __FUNCTION__, str);
    goto err;
  }
  time_tm.tm_min = atoi(mm);
  if (time_tm.tm_min > 59)
  {
    fprintf(stderr, "%s: minute value too big '%s'\n", __FUNCTION__, str);
    goto err;
  }
  time_tm.tm_sec = atoi(ss);
  if (time_tm.tm_sec > 59)
  {
    fprintf(stderr, "%s: second value too big '%s'\n", __FUNCTION__, str);
    goto err;
  }

  if (isutc)
    *time = timegm(&time_tm);
  else
    *time = mktime(&time_tm);
  return 0;

err:
  *time = INVALID_TIME;
  return -(EINVAL);
}

void time_to_iso8601utc(time_t time, char *str)
{
  struct tm time_tm;

  if (time == INVALID_TIME || NULL == gmtime_r(&time, &time_tm))
  {
    str[0] = '\0';
    return;
  }
  sprintf(str, "%4.4d-%2.2d-%2.2dT%2.2d:%2.2d:%2.2dZ",
          time_tm.tm_year + 1900,
          time_tm.tm_mon + 1,
          time_tm.tm_mday,
          time_tm.tm_hour,
          time_tm.tm_min,
          time_tm.tm_sec);
}

void time_to_iso8601(time_t time, char *str)
{
  struct tm time_tm;

  if (time == INVALID_TIME || NULL == localtime_r(&time, &time_tm))
  {
    str[0] = '\0';
    return;
  }
  sprintf(str, "%4.4d-%2.2d-%2.2dT%2.2d:%2.2d:%2.2d",
          time_tm.tm_year + 1900,
          time_tm.tm_mon + 1,
          time_tm.tm_mday,
          time_tm.tm_hour,
          time_tm.tm_min,
          time_tm.tm_sec);
}

void time_to_isodate(time_t time, char *str)
{
  struct tm time_tm;

  if (time == INVALID_TIME || NULL == localtime_r(&time, &time_tm))
  {
    str[0] = '\0';
    return;
  }
  sprintf(str, "%4.4d-%2.2d-%2.2d",
          time_tm.tm_year + 1900,
          time_tm.tm_mon + 1,
          time_tm.tm_mday);
}

tz_t *time_tz(time_t time, tz_t* tz) {
  struct tm loc;
  localtime_r(&time, &loc);
  struct tm gmt;
  gmtime_r(&time, &gmt);
  int minutes = ((loc.tm_hour * 60 + loc.tm_min) - (gmt.tm_hour * 60 + gmt.tm_min)) % 720;
  tz->tz_dir = minutes < 0 ? (-1) : 1;
  tz->tz_hour = tz->tz_dir * minutes / 60;
  tz->tz_min  = tz->tz_dir * ( minutes - tz->tz_hour * 60 );
  sprintf(tz->tz_str, "%+2.2d:%2.2d", tz->tz_dir * tz->tz_hour, tz->tz_min);
  return tz;
}
