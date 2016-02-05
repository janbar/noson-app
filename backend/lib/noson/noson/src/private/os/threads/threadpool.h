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

#include "thread.h"
#include "event.h"

#include <queue>
#include <set>

#ifdef NSROOT
namespace NSROOT {
#endif
namespace OS
{

  class CWorker;

  class CWorkerThread;

  class CThreadPool
  {
    friend class CWorkerThread;
  public:
    CThreadPool();
    CThreadPool(unsigned size);
    ~CThreadPool();

    bool Enqueue(CWorker* worker);

    unsigned GetMaxSize() const { return m_size; }

    void SetMaxSize(unsigned size);

    void SetKeepAlive(unsigned millisec);

    unsigned Size() const;

    unsigned QueueSize() const;

    void Suspend();
    void Resume();
    bool IsSuspended() const;

    void Reset();
    void Stop();
    void Start();
    bool IsStopped() const;

  private:
    unsigned      m_size;
    unsigned      m_keepAlive;
    unsigned      m_poolSize;
    unsigned      m_waitingCount;
    volatile bool m_stopped;
    volatile bool m_suspended;
    volatile bool m_empty;

    std::queue<CWorker*>      m_queue;
    std::set<CWorkerThread*>  m_pool;
    mutable CMutex            m_mutex;
    CCondition<volatile bool> m_condition;
    CEvent                    m_queueContent;

    CWorker* PopQueue(CWorkerThread* _thread);
    void WaitQueue(CWorkerThread* _thread);
    void StartThread(CWorkerThread* _thread);
    void FinalizeThread(CWorkerThread* _thread);
    void __resize();
  };

  class CWorker
  {
    friend class CThreadPool;
  public:
    CWorker() : m_queued(false) { }
    virtual ~CWorker() { }
    virtual void Process() = 0;

  private:
    bool m_queued;
  };

  class CWorkerThread : public CThread
  {
  public:
    CWorkerThread(CThreadPool& pool)
    : CThread()
    , m_threadPool(pool) { m_finalizeOnStop = true; }

    void* Process(void)
    {
      bool waiting = false;

      while (!IsStopped())
      {
        CWorker* worker = m_threadPool.PopQueue(this);
        if (worker != NULL)
        {
          worker->Process();
          delete worker;
          waiting = false;
        }
        else if (!waiting)
        {
          m_threadPool.WaitQueue(this);
          waiting = true;
        }
        else
          break;
      }

      return NULL;
    }

    void Finalize(void)
    {
      m_threadPool.FinalizeThread(this);
    }

  private:
    CThreadPool& m_threadPool;
  };

}
#ifdef NSROOT
}
#endif
