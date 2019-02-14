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
#include "iodevice.h"

#include "private/os/threads/condition.h"
#include "private/os/threads/timeout.h"
#include "private/os/threads/mutex.h"
#include "private/debug.h"

using namespace NSROOT;

namespace NSROOT
{
  struct IODevice::Lockable
  {
    OS::CMutex mutex;
  };
}

IODevice::IODevice()
: m_out(nullptr)
, m_lock(new Lockable())
, m_readyRead(new OS::CCondition<bool>())
, m_mode(NotOpen)
{
}

IODevice::~IODevice()
{
  delete static_cast<OS::CCondition<bool>*>(m_readyRead);
  delete m_lock;
}

bool IODevice::open(OpenMode mode)
{
  m_mode = mode;
  return (m_mode != NotOpen);
}

void IODevice::close()
{
  m_mode = NotOpen;
}

int IODevice::read(char* data, int maxlen, unsigned timeout)
{
  if (!readable())
    return -1;

  m_lock->mutex.Lock();
  if (bytesAvailable() == 0)
  {
    OS::CTimeout _timeout(timeout);
    OS::CCondition<bool>* _condition = static_cast<OS::CCondition<bool>*>(m_readyRead);
    bool signaled = _condition->Wait(m_lock->mutex, _timeout);
    m_lock->mutex.Unlock();
    if (!signaled)
      return 0;
  }
  return readData(data, maxlen);
}

int IODevice::write(const char* data, int len)
{
  if (!writable())
    return -1;

  return writeData(data, len);
}

void IODevice::connectOutput(IODevice* io)
{
  m_out = io;
}

void IODevice::readyRead()
{
  OS::CCondition<bool>* _condition = static_cast<OS::CCondition<bool>*>(m_readyRead);
  _condition->Broadcast();
}
