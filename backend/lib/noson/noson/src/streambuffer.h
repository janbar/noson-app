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

#ifndef STREAMBUFFER_H
#define STREAMBUFFER_H

#include "local_config.h"

#include <cstring>
#include <cassert>

namespace NSROOT
{

class StreamBuffer
{
public:
  StreamBuffer(int capacity);
  virtual ~StreamBuffer();

  int capacity() const;

  int size() const;

  void clear();

  int write(const char * data, int len);

  int read(char * data, int maxlen);

private:
  // Prevent copy
  StreamBuffer(const StreamBuffer& other);
  StreamBuffer& operator=(const StreamBuffer& other);

private:
  struct Lockable;
  mutable Lockable * m_lock;
  char * m_buffer;
  const int m_capacity;
  volatile int m_wx;
  volatile int m_rx;
  volatile bool m_reset;
};

}

#endif /* STREAMBUFFER_H */

