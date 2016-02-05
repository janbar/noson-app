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

#if defined(_MSC_VER)
#include "../windows/winpthreads.h"
#else
#include <pthread.h>
#endif

#ifdef NSROOT
namespace NSROOT {
#endif
namespace OS
{

#if defined (__WINDOWS__)
#define gettimeofday __gettimeofday
  inline int __gettimeofday(struct timeval *pcur_time, struct timezone *tz)
  {
    if (pcur_time == NULL)
    {
      SetLastError(EFAULT);
      return -1;
    }
    struct _timeb current;
    _ftime(&current);
    pcur_time->tv_sec = (long) current.time;
    pcur_time->tv_usec = current.millitm * 1000L;
    if (tz)
    {
      tz->tz_minuteswest = current.timezone;  /* minutes west of Greenwich  */
      tz->tz_dsttime = current.dstflag;       /* type of dst correction  */
    }
    return 0;
  }
#endif

  typedef pthread_t thread_t;

#define thread_create(a, b, c) __thread_create(a, b, c)
  inline bool __thread_create(thread_t* thread, void* (*func)(void*), void* arg)
  {
    static pthread_attr_t _attr;
    static bool _init = false;
    if (!_init)
    {
      pthread_attr_init(&_attr);
      pthread_attr_setdetachstate(&_attr, PTHREAD_CREATE_DETACHED);
      _init = true;
    }
    return pthread_create(thread, &_attr, func, arg) == 0;
  }

  typedef pthread_mutex_t mutex_t;

#define mutex_init(a) __mutex_init(a)
  inline bool __mutex_init(mutex_t* mutex)
  {
    static pthread_mutexattr_t _attr;
    static bool _init = false;
    if (!_init)
    {
      pthread_mutexattr_init(&_attr);
      pthread_mutexattr_settype(&_attr, PTHREAD_MUTEX_RECURSIVE);
      _init = true;
    }
    return pthread_mutex_init(mutex, &_attr) == 0;
  }

#define mutex_lock(a) __mutex_lock(a)
  inline bool __mutex_lock(mutex_t* mutex) { return pthread_mutex_lock(mutex) == 0; }

#define mutex_trylock(a) __mutex_trylock(a)
  inline bool __mutex_trylock(mutex_t* mutex) { return pthread_mutex_trylock(mutex) == 0; }

#define mutex_unlock(a) __mutex_unlock(a)
  inline void __mutex_unlock(mutex_t* mutex) { pthread_mutex_unlock(mutex); }

#define mutex_destroy(a) __mutex_destroy(a)
  inline void __mutex_destroy(mutex_t* mutex) { pthread_mutex_destroy(mutex); }

  typedef pthread_cond_t condition_t;

#define cond_init(a) __cond_init(a)
  inline bool __cond_init(condition_t* cond) { return pthread_cond_init(cond, NULL) == 0; }

#define cond_signal(a) __cond_signal(a)
  inline void __cond_signal(condition_t* cond) { pthread_cond_signal(cond); }

#define cond_broadcast(a) __cond_broadcast(a)
  inline void __cond_broadcast(condition_t* cond) { pthread_cond_broadcast(cond); }

#define cond_wait(a, b) __cond_wait(a, b)
  inline bool __cond_wait(condition_t* cond, mutex_t* mutex) { return pthread_cond_wait(cond, mutex) == 0; }

#define cond_timedwait(a, b, c) __cond_timedwait(a, b, c)
  inline bool __cond_timedwait(condition_t* cond, mutex_t* mutex, unsigned millisec)
  {
    if (millisec == 0)
      return cond_wait(cond, mutex);

    struct timespec time;
#if defined(__APPLE__) || defined(__WINDOWS__)
    struct timeval tv;
    gettimeofday(&tv, NULL);
    tv.tv_usec += (millisec % 1000) * 1000;
    tv.tv_sec += millisec / 1000;
    time.tv_sec = tv.tv_sec + tv.tv_usec / 1000000;
    time.tv_nsec = (tv.tv_usec % 1000000) * 1000;
#else
    clock_gettime(CLOCK_REALTIME, &time);
    time.tv_nsec += (millisec % 1000) * 1000000;
    time.tv_sec  += millisec / 1000 + time.tv_nsec / 1000000000;
    time.tv_nsec %= 1000000000;
#endif
    return (pthread_cond_timedwait(cond, mutex, &time) == 0);
  }

#define cond_destroy(a) __cond_destroy(a)
  inline void __cond_destroy(condition_t* cond) { pthread_cond_destroy(cond); }

}
#ifdef NSROOT
}
#endif
