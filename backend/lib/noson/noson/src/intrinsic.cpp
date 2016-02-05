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

#include "intrinsic.h"

#include <local_config.h>
#if CXX_STANDARD >= 201103L
#include <atomic>
typedef std::atomic<int> counter_t;
#define GETVALUE(p)   (p)->load()
#define INCREMENT(p)  ((p)->fetch_add(1, std::memory_order_relaxed) + 1)
#define DECREMENT(p)  ((p)->fetch_sub(1, std::memory_order_relaxed) - 1)

#elif defined _MSC_VER
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
typedef volatile LONG counter_t;
#define GETVALUE(p)   (*(p))
#define INCREMENT(p)  InterlockedIncrement(p)
#define DECREMENT(p)  InterlockedDecrement(p)

#elif defined __APPLE__
#include <libkern/OSAtomic.h>
typedef volatile int32_t counter_t;
#define GETVALUE(p)   (*(p))
#define INCREMENT(p)  OSAtomicIncrement32(p)
#define DECREMENT(p)  OSAtomicDecrement32(p)

#elif defined HAS_BUILTIN_SYNC_ADD_AND_FETCH
typedef volatile int counter_t;
#define GETVALUE(p)   (*(p))
#define INCREMENT(p)  __sync_add_and_fetch(p, 1)
#if defined HAS_BUILTIN_SYNC_SUB_AND_FETCH
#define DECREMENT(p)  __sync_sub_and_fetch(p, 1)
#else
#define DECREMENT(p)  __sync_add_and_fetch(p, -1)
#endif

#else
#include "private/atomic.h"
#ifndef ATOMIC_NOATOMIC
typedef NSROOT::atomic<int> counter_t;
#define GETVALUE(p)   (p)->load()
#define INCREMENT(p)  (p)->add_fetch(1)
#define DECREMENT(p)  (p)->sub_fetch(1)
//
// Don't know how to do atomic operation for the architecture
//
#elif defined USE_MYTH_LOCKED
#include "locked.h"
typedef NSROOT::LockedNumber<int> counter_t;
#define GETVALUE(p)   (p)->Load()
#define INCREMENT(p)  (p)->Add(1)
#define DECREMENT(p)  (p)->Sub(1)

#else
#error Atomic add/sub are not. Overcome using definition USE_MYTH_LOCKED.
#endif
#endif

using namespace NSROOT;

namespace NSROOT
{
  struct IntrinsicCounter::Counter
  {
    counter_t counter;
    Counter(int val) : counter(val) {}
  };
}

IntrinsicCounter::IntrinsicCounter(int val)
: m_ptr(new Counter(val))
{
}

IntrinsicCounter::~IntrinsicCounter()
{
  delete m_ptr;
}

int IntrinsicCounter::GetValue()
{
  return GETVALUE(&m_ptr->counter);
}

int IntrinsicCounter::Increment()
{
  return INCREMENT(&m_ptr->counter);
}

int IntrinsicCounter::Decrement()
{
  return DECREMENT(&m_ptr->counter);
}
