#pragma once
/*
 *      Copyright (C) 2015 Jean-Luc Barriere
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

#include "../os.h"

#if defined(__APPLE__)
#include <mach/mach_time.h>
#endif

#ifdef NSROOT
namespace NSROOT {
#endif
namespace OS
{
#define gettime_ms __gettime_ms
  inline int64_t __gettime_ms()
  {
#if defined(__APPLE__)
    // Recommended by Apple's QA1398.
    int64_t ticks = 0;
    static mach_timebase_info_data_t timebase;
    // Get the timebase if this is the first time we run.
    if (timebase.denom == 0)
      (void)mach_timebase_info(&timebase);
    // Use timebase to convert absolute time tick units into nanoseconds.
    ticks = mach_absolute_time() * timebase.numer / timebase.denom;
    return ticks / 1000000;
#elif defined(__WINDOWS__)
    LARGE_INTEGER tickPerSecond;
    LARGE_INTEGER tick;
    if (QueryPerformanceFrequency(&tickPerSecond))
    {
      QueryPerformanceCounter(&tick);
      return (int64_t) (tick.QuadPart / (tickPerSecond.QuadPart / 1000.0));
    }
    return -1;
#else
    timespec time;
    clock_gettime(CLOCK_MONOTONIC, &time);
    return (int64_t)time.tv_sec * 1000 + time.tv_nsec / 1000000;
#endif
  }

  class CTimeout
  {
  public:
    CTimeout() : m_time(0) { }
    CTimeout(unsigned millisec) : m_time(0) { Set(millisec); }

    void Set(unsigned millisec)
    {
      m_time = gettime_ms() + millisec;
    }

    bool IsSet() const
    {
      return (m_time > 0 ? true : false);
    }

    void Clear()
    {
      m_time = 0;
    }

    unsigned TimeLeft() const
    {
      int64_t time = gettime_ms();
      return (time > m_time ? 0 : static_cast<unsigned>(m_time - time));
    }

    bool operator==(const CTimeout& other) const
    {
      return m_time == other.m_time;
    }

    bool operator!=(const CTimeout& other) const
    {
      return m_time != other.m_time;
    }

    bool operator<(const CTimeout& other) const
    {
      return m_time < other.m_time;
    }

    bool operator>(const CTimeout& other) const
    {
      return m_time > other.m_time;
    }

    bool operator>=(const CTimeout& other) const
    {
      return !(m_time < other.m_time);
    }

    bool operator<=(const CTimeout& other) const
    {
      return !(m_time > other.m_time);
    }

  private:
    int64_t m_time;
  };
}
#ifdef NSROOT
}
#endif
