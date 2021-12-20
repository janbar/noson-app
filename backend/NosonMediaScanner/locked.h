/*
 *      Copyright (C) 2019 Jean-Luc Barriere
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#ifndef LOCKED_H
#define LOCKED_H

#include <QMutex>

#if QT_VERSION < 0x050E00 //QT_VERSION_CHECK(5, 14, 0)
class QRecursiveMutex : private QMutex
{
public:
  QRecursiveMutex() : QMutex(QMutex::Recursive) { }
  void lock() { QMutex::lock(); }
  void unlock() { QMutex::unlock(); }
  bool try_lock() { return QMutex::try_lock(); }
};
#endif /* QT_VERSION */

namespace mediascanner
{

/**
 * This implements a "guard" pattern
 */
template<typename T>
class LockGuard
{
public:
  LockGuard() : m_lock(nullptr) { }
  /**
   * Initialize a guard which hold the lock. The lock will be released by the
   * destructor.
   * @param lock The pointer to lockable object
   */
  LockGuard(T * lock)
  : m_lock(lock)
  {
    if (m_lock)
      m_lock->lock();
  }

  ~LockGuard()
  {
    if (m_lock)
      m_lock->unlock();
  }

  LockGuard(const LockGuard& other) = delete;
  LockGuard& operator=(const LockGuard& other) = delete;

  LockGuard(LockGuard&& other) : m_lock(other.m_lock)
  {
    other.m_lock = nullptr;
  }

  LockGuard& operator=(LockGuard&& other)
  {
    T * tmp = m_lock;
    m_lock = other.m_lock;
    other.m_lock = tmp;
    return *this;
  }

private:
  T * m_lock;
};

template<typename T>
class Locked
{
public:
  Locked(const T& val)
  : m_val(val)
  , m_lock(new QMutex()) {}

  ~Locked()
  {
    delete m_lock;
  }

  T Load()
  {
    LockGuard<QMutex> g(m_lock);
    return m_val; // return copy
  }

  const T& Store(const T& newval)
  {
    LockGuard<QMutex> g(m_lock);
    m_val = newval;
    return newval; // return input
  }

  class pointer
  {
  public:
    pointer(T& val, QMutex*& lock) : m_val(val), m_g(lock) {}
    T& operator* () const { return m_val; }
    T *operator->() const { return &m_val; }
  private:
    T& m_val;
    LockGuard<QMutex> m_g;
  };

  pointer Get()
  {
    return pointer(m_val, m_lock);
  }

protected:
  T m_val;
  QMutex * m_lock;

  // Prevent copy
  Locked(const Locked<T>& other);
  Locked<T>& operator=(const Locked<T>& other);
};

template <typename T>
class LockedNumber : public Locked<T>
{
public:
  LockedNumber(T val)
  : Locked<T>(val) {}

  T Add(T amount)
  {
    LockGuard<QMutex> g(Locked<T>::m_lock);
    return Locked<T>::m_val += amount;
  }

  T operator+=(T amount)
  {
    return Add(amount);
  }

  T Sub(T amount)
  {
    LockGuard<QMutex> g(Locked<T>::m_lock);
    return Locked<T>::m_val -= amount;
  }

  T operator-=(T amount)
  {
    return Sub(amount);
  }
};

}

#endif /* LOCKED_H */

