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

#include "threadpool.h"

#include <cassert>

#define WTH_KEEPALIVE 5000

#ifdef NSROOT
using namespace NSROOT::OS;
#else
using namespace OS;
#endif

CThreadPool::CThreadPool()
: m_size(1)
, m_keepAlive(WTH_KEEPALIVE)
, m_poolSize(0)
, m_waitingCount(0)
, m_stopped(false)
, m_suspended(false)
, m_empty(false)
{
}

CThreadPool::CThreadPool(unsigned size)
: m_size(size)
, m_keepAlive(WTH_KEEPALIVE)
, m_poolSize(0)
, m_waitingCount(0)
, m_stopped(false)
, m_suspended(false)
, m_empty(false)
{
}

CThreadPool::~CThreadPool()
{
  m_mutex.Lock();
  // Reject new runs
  m_stopped = true;
  // Destroy all queued workers
  while (!m_queue.empty())
  {
    delete m_queue.front();
    m_queue.pop();
  }
  // Finalize all running
  if (!m_pool.empty())
  {
    m_empty = false;
    // Signal stop
    for (std::set<CWorkerThread*>::iterator it = m_pool.begin(); it != m_pool.end(); ++it)
      (*it)->StopThread(false);
    // Wake sleeper
    m_queueContent.Broadcast();
    // Waiting all finalized
    m_condition.Wait(m_mutex, m_empty);
  }
}

bool CThreadPool::Enqueue(CWorker* worker)
{
  assert(worker->m_queued != true);
  CLockGuard lock(m_mutex);
  if (!m_stopped)
  {
    worker->m_queued = true;
    m_queue.push(worker);
    if (!m_suspended)
    {
      if (m_waitingCount)
      {
        // Wake a thread
        m_queueContent.Signal();
        return true;
      }
      else
      {
        __resize();
        return true;
      }
    }
    // Delayed work
    return true;
  }
  return false;
}

void CThreadPool::SetMaxSize(unsigned size)
{
  CLockGuard lock(m_mutex);
  m_size = size;
  if (!m_suspended)
    __resize();
}

void CThreadPool::SetKeepAlive(unsigned millisec)
{
  CLockGuard lock(m_mutex);
  m_keepAlive = millisec;
}

unsigned CThreadPool::Size() const
{
  CLockGuard lock(m_mutex);
  return m_poolSize;
}

unsigned CThreadPool::QueueSize() const
{
  CLockGuard lock(m_mutex);
  return static_cast<unsigned>(m_queue.size());
}

void CThreadPool::Suspend()
{
  CLockGuard lock(m_mutex);
  m_suspended = true;
}

void CThreadPool::Resume()
{
  CLockGuard lock(m_mutex);
  m_suspended = false;
  __resize();
}

bool CThreadPool::IsSuspended() const
{
  CLockGuard lock(m_mutex);
  return m_suspended;
}

void CThreadPool::Reset()
{
  CLockGuard lock(m_mutex);
  m_stopped = true;
  // Destroy all queued workers
  while (!m_queue.empty())
  {
    delete m_queue.front();
    m_queue.pop();
  }
}

void CThreadPool::Stop()
{
  CLockGuard lock(m_mutex);
  m_stopped = true;
}

void CThreadPool::Start()
{
  CLockGuard lock(m_mutex);
  m_stopped = false;
}

bool CThreadPool::IsStopped() const
{
  CLockGuard lock(m_mutex);
  return m_stopped;
}

CWorker* CThreadPool::PopQueue(CWorkerThread* _thread)
{
  (void)_thread;
  CLockGuard lock(m_mutex);
  if (!m_suspended && !m_queue.empty())
  {
    CWorker* worker = m_queue.front();
    m_queue.pop();
    return worker;
  }
  return NULL;
}

void CThreadPool::WaitQueue(CWorkerThread* _thread)
{
  (void)_thread;
  CLockGuard lock(m_mutex);
  ++m_waitingCount;
  unsigned millisec = m_keepAlive;
  lock.Unlock();
  m_queueContent.Wait(millisec);
  lock.Lock();
  --m_waitingCount;
}

void CThreadPool::StartThread(CWorkerThread* _thread)
{
  ++m_poolSize;
  m_pool.insert(_thread);
  if (!_thread->StartThread(false))
    FinalizeThread(_thread);
}

void CThreadPool::FinalizeThread(CWorkerThread* _thread)
{
  CLockGuard lock(m_mutex);
  if (m_pool.erase(_thread))
  {
    --m_poolSize;
    delete _thread;
  }
  if (m_pool.empty())
  {
    m_empty = true;
    m_condition.Broadcast();
  }
}

void CThreadPool::__resize()
{
  if (m_poolSize < m_size && !m_queue.empty())
  {
    for (unsigned i = m_queue.size(); i > 0; --i)
    {
      if (m_poolSize >= m_size)
        break;
      CWorkerThread* _thread = new CWorkerThread(*this);
      // The new thread will check the queue
      StartThread(_thread);
    }
  }
  else if (m_poolSize > m_size)
  {
    std::set<CWorkerThread*>::iterator it = m_pool.begin();
    for (unsigned i = m_poolSize - m_size; i > 0; --i)
    {
      if (it == m_pool.end())
        break;
      (*it)->StopThread(false);
      ++it;
    }
    // Wake up the waiting threads to stop
    if (m_waitingCount)
        m_queueContent.Broadcast();
  }
}
