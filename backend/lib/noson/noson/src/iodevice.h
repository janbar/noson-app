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

#ifndef IODEVICE_H
#define IODEVICE_H

#include "local_config.h"

namespace NSROOT
{

class IODevice
{
public:
  IODevice();
  virtual ~IODevice();

  enum OpenMode { NotOpen = 0, ReadOnly = 1, WriteOnly = 2, ReadWrite = 3 };

  virtual bool open(OpenMode mode);
  virtual void close();
  virtual bool isSequential() const = 0;
  virtual int bytesAvailable() const = 0;

  bool readable() { return (m_mode & ReadOnly); }
  bool writable() { return (m_mode & WriteOnly); }
  bool isOpen() { return (m_mode != NotOpen); }
  
  int read(char * data, int maxlen, unsigned timeout);
  int write(const char * data, int len);

  void connectOutput(IODevice * io);

protected:
  void readyRead();
  virtual int readData(char * data, int maxlen) = 0;
  virtual int writeData(const char *data, int len) = 0;

  IODevice * m_out;

private:
  struct Lockable;
  mutable Lockable * m_lock;
  void * m_readyRead;
  OpenMode m_mode;
};

}

#endif /* IODEVICE_H */

