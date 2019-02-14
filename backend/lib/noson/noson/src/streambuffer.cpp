/*
 *      Copyright (C) 2018-2019 Jean-Luc Barriere
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
#include "streambuffer.h"

#include "private/os/threads/mutex.h"

using namespace NSROOT;

namespace NSROOT
{
  struct StreamBuffer::Lockable
  {
    OS::CMutex mutex;
  };
}

StreamBuffer::StreamBuffer(int capacity)
: m_lock(new Lockable())
, m_buffer(new char [capacity])
, m_capacity(capacity)
, m_wx(0)
, m_rx(0)
, m_reset(false)
{
  assert(capacity > 0);
}

StreamBuffer::~StreamBuffer ()
{
  delete [] m_buffer;
  delete m_lock;
}

int StreamBuffer::capacity () const
{
  return m_capacity;
}

int StreamBuffer::size () const
{
  m_lock->mutex.Lock ();
  int left = m_wx - m_rx;
  m_lock->mutex.Unlock ();
  return left > m_capacity ? m_capacity : left;
}

void StreamBuffer::clear ()
{
  m_lock->mutex.Lock ();
  m_wx = m_rx = 0;
  m_reset = true;
  m_lock->mutex.Unlock ();
}

int StreamBuffer::write (const char * data, int len)
{
  m_lock->mutex.Lock ();
  int wx = m_wx;
  m_reset = false;
  m_lock->mutex.Unlock ();
  int left = len;
  while (left > 0)
  {
    int p = wx % m_capacity;
    int l = m_capacity - p;
    if (l > left)
      l = left;
    memcpy (m_buffer + p, data, l);
    m_lock->mutex.Lock ();
    wx = m_reset ? m_wx : (m_wx += l);
    m_reset = false;
    m_lock->mutex.Unlock ();
    data += l;
    left -= l;
  }
  m_lock->mutex.Unlock ();
  return len;
}

int StreamBuffer::read (char * data, int maxlen)
{
  m_lock->mutex.Lock ();
  int wx = m_wx;
  m_lock->mutex.Unlock ();
  // adjust available bytes
  int left = wx - m_rx;
  if (left > m_capacity)
  {
    m_rx = wx - m_capacity;
    left = m_capacity;
  }
  if (left > maxlen)
    left = maxlen;
  else
    maxlen = left;
  while (left > 0)
  {
    int p = m_rx % m_capacity;
    int l = m_capacity - p;
    if (l > left)
      l = left;
    memcpy (data, m_buffer + p, l);
    m_rx += l;
    data += l;
    left -= l;
  }
  return (maxlen - left);
}
