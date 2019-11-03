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

#ifndef STREAMREADER_H
#define STREAMREADER_H

#include "local_config.h"

#include <string>

namespace NSROOT
{

class StreamReader
{
public:

  struct STREAM
  {
    void * opaque;              ///< Internal data handled by the reader
    const char * contentType;   ///< Mime type of the stream data
    unsigned contentLength;     ///< Total length of the stream data
    const char * data;          ///< Pointer to buffer of last read data
    unsigned size;              ///< Size of buffer
  };

  virtual ~StreamReader() = default;
  virtual STREAM * OpenStream(const std::string& streamUrl) = 0;
  virtual int ReadStream(STREAM * stream) = 0;
  virtual void CloseStream(STREAM * stream) = 0;

};

}

#endif // STREAMREADER_H
