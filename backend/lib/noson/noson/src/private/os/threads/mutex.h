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

#include "os-threads.h"

#ifdef NSROOT
namespace NSROOT {
#endif
namespace OS
{

  class CMutex
  {
  public:
    CMutex()
    : m_lockCount(0)
    {
      mutex_init(&m_handle);
    }

    ~CMutex()
    {
      Clear();
      mutex_destroy(&m_handle);
    }

    mutex_t* NativeHandle()
    {
      return &m_handle;
    }

    bool TryLock()
    {
      if (mutex_trylock(&m_handle))
      {
        ++m_lockCount;
        return true;
      }
      return false;
    }

    void Lock()
    {
      mutex_lock(&m_handle);
      ++m_lockCount;
    }

    void Unlock()
    {
      if (mutex_trylock(&m_handle))
      {
        if (m_lockCount > 0)
        {
          mutex_unlock(&m_handle);
          --m_lockCount;
        }
        mutex_unlock(&m_handle);
      }
    }

    void Clear()
    {
      if (mutex_trylock(&m_handle))
      {
        for (unsigned i = m_lockCount; i > 0; --i)
          mutex_unlock(&m_handle);
        m_lockCount = 0;
        mutex_unlock(&m_handle);
      }
    }

  private:
    mutex_t           m_handle;
    volatile unsigned m_lockCount;

    // Prevent copy
    CMutex(const CMutex& other);
    CMutex& operator=(const CMutex& other);
  };

  class CLockGuard
  {
  public:
    CLockGuard(CMutex& mutex)
    : m_mutex(mutex)
    , m_lockCount(0)
    {
      Lock();
    }

    ~CLockGuard()
    {
      Clear();
    }

    bool TryLock()
    {
      if (m_mutex.TryLock())
      {
        ++m_lockCount;
        return true;
      }
      return false;
    }

    void Lock()
    {
      m_mutex.Lock();
      ++m_lockCount;
    }

    void Unlock()
    {
      if (m_mutex.TryLock())
      {
        if (m_lockCount > 0)
        {
          m_mutex.Unlock();
          --m_lockCount;
        }
        m_mutex.Unlock();
      }
    }

    void Clear()
    {
      if (m_mutex.TryLock())
      {
        for (unsigned i = m_lockCount; i > 0; --i)
          m_mutex.Unlock();
        m_lockCount = 0;
        m_mutex.Unlock();
      }
    }

  private:
    CMutex&           m_mutex;
    volatile unsigned m_lockCount;

    // Prevent copy
    CLockGuard(const CLockGuard& other);
    CLockGuard& operator=(const CLockGuard& other);
  };

}
#ifdef NSROOT
}
#endif
