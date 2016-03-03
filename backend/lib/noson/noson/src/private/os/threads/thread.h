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
#include "condition.h"

#ifdef NSROOT
namespace NSROOT {
#endif
namespace OS
{

  class CThread
  {
  public:
    CThread()
    : m_finalizeOnStop(false)
    , m_handle(new Handle()) { }

    virtual ~CThread()
    {
      delete m_handle;
    }

    CThread(const CThread& _thread)
    {
      this->m_handle = new Handle();
      this->m_finalizeOnStop = _thread.m_finalizeOnStop;
    }

    CThread& operator=(const CThread& _thread)
    {
      if (this != &_thread)
      {
        delete this->m_handle;
        this->m_handle = new Handle();
        this->m_finalizeOnStop = _thread.m_finalizeOnStop;
      }
      return *this;
    }

    thread_t* NativeHandle()
    {
      return &(m_handle->nativeHandle);
    }

    bool StartThread(bool wait = true)
    {
      CLockGuard lock(m_handle->mutex);
      if (!m_handle->running)
      {
        m_handle->started = false;
        m_handle->notifiedStop = false;
        if (thread_create(&(m_handle->nativeHandle), CThread::ThreadHandler, ((void*)static_cast<CThread*>(this))))
        {
          if (wait)
            m_handle->condition.Wait(m_handle->mutex, m_handle->started);
          return true;
        }
      }
      return false;
    }

    void StopThread(bool wait = true)
    {
      // First signal stop
      {
        CLockGuard lock(m_handle->mutex);
        m_handle->notifiedStop = true;
        m_handle->condition.Broadcast();
      }
      // Waiting stopped
      if (wait)
      {
        CLockGuard lock(m_handle->mutex);
        m_handle->condition.Wait(m_handle->mutex, m_handle->stopped);
      }
    }

    bool WaitThread(unsigned timeout)
    {
      CLockGuard lock(m_handle->mutex);
      return m_handle->stopped ? true : m_handle->condition.Wait(m_handle->mutex, m_handle->stopped, timeout);
    }

    bool IsRunning()
    {
      CLockGuard lock(m_handle->mutex);
      return m_handle->running;
    }

    bool IsStopped()
    {
      CLockGuard lock(m_handle->mutex);
      return m_handle->notifiedStop;
    }

    void Sleep(unsigned timeout)
    {
      CLockGuard lock(m_handle->mutex);
      if (!m_handle->notifiedStop)
        m_handle->condition.Wait(m_handle->mutex, m_handle->notifiedStop, timeout);
    }

  protected:
    virtual void* Process(void) = 0;
    virtual void Finalize(void) { };
    bool m_finalizeOnStop;

  private:
    struct Handle
    {
      thread_t      nativeHandle;
      volatile bool running;
      volatile bool stopped;
      volatile bool notifiedStop;
      volatile bool started;
      CCondition<volatile bool> condition;
      CMutex        mutex;

      Handle()
      : nativeHandle(0)
      , running(false)
      , stopped(true)
      , notifiedStop(false)
      , started(false)
      , condition()
      , mutex() { }
    };

    Handle* m_handle;

    static void* ThreadHandler(void* _thread)
    {
      CThread* thread = static_cast<CThread*>(_thread);
      void* ret = NULL;

      if (thread)
      {
        bool finalize = thread->m_finalizeOnStop;
        {
          CLockGuard lock(thread->m_handle->mutex);
          thread->m_handle->started = true;
          thread->m_handle->running = true;
          thread->m_handle->stopped = false;
          thread->m_handle->condition.Broadcast();
          lock.Unlock();
          ret = thread->Process();
          lock.Lock();
          thread->m_handle->running = false;
          thread->m_handle->stopped = true;
          thread->m_handle->condition.Broadcast();
        }
        // Thread without finalizer could be freed here
        if (finalize)
          thread->Finalize();
      }

      return ret;
    }

  };

}
#ifdef NSROOT
}
#endif
