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

#include "mutex.h"
#include "timeout.h"

#ifdef NSROOT
namespace NSROOT {
#endif
namespace OS
{

  template <typename P>
  class CCondition
  {
  public:
    CCondition()
    {
      cond_init(&m_condition);
    }

    ~CCondition()
    {
      cond_destroy(&m_condition);
    }

    void Broadcast()
    {
      cond_broadcast(&m_condition);
    }

    void Signal()
    {
      cond_signal(&m_condition);
    }

    bool Wait(CMutex& mutex, P& predicate)
    {
      while(!predicate)
        cond_wait(&m_condition, mutex.NativeHandle());
      return true;
    }

    bool Wait(CMutex& mutex, P& predicate, unsigned timeout)
    {
      CTimeout _timeout(timeout);
      while (!predicate)
      {
        // wait for time left
        timeout = _timeout.TimeLeft();
        if (timeout == 0)
          return false;
        cond_timedwait(&m_condition, mutex.NativeHandle(), timeout);
      }
      return true;
    }

    bool Wait(CMutex& mutex, CTimeout& timeout)
    {
      cond_timedwait(&m_condition, mutex.NativeHandle(), timeout.TimeLeft());
      return (timeout.TimeLeft() > 0 ? true : false);
    }

  private:
    condition_t m_condition;

    // Prevent copy
    CCondition(const CCondition<P>& other);
    CCondition<P>& operator=(const CCondition<P>& other);
  };

}
#ifdef NSROOT
}
#endif
