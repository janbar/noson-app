/*
 *      Copyright (C) 2014-2016 Jean-Luc Barriere
 *
 *  This file is part of Noson
 *
 *  Noson is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Noson is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "locked.h"
#include "private/os/threads/mutex.h"

using namespace NSROOT;

namespace NSROOT
{
  struct LockGuard::Lockable
  {
    OS::CMutex mutex;
  };
}

LockGuard::LockGuard(Lockable* lock)
: m_lock(lock)
{
  if (m_lock)
    m_lock->mutex.Lock();
}

LockGuard::~LockGuard()
{
  if (m_lock)
    m_lock->mutex.Unlock();
}

LockGuard::LockGuard(const LockGuard& other)
: m_lock(other.m_lock)
{
  if (m_lock)
    m_lock->mutex.Lock();
}

LockGuard& LockGuard::operator=(const LockGuard& other)
{
  if (m_lock)
    m_lock->mutex.Unlock();
  if (other.m_lock)
    other.m_lock->mutex.Lock();
  m_lock = other.m_lock;
  return *this;
}

LockGuard::Lockable* LockGuard::CreateLock()
{
  return new Lockable();
}

void LockGuard::DestroyLock(Lockable* lock)
{
  delete lock;
}

void LockGuard::Lock(Lockable* lock)
{
  lock->mutex.Lock();
}

void LockGuard::Unlock(Lockable* lock)
{
  lock->mutex.Unlock();
}

void LockGuard::ClearLock(Lockable* lock)
{
  lock->mutex.Clear();
}
