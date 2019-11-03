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

#ifndef BASE64_H
#define BASE64_H

#include "local_config.h"

#include <cstddef>
#include <string>

namespace NSROOT
{

  class Base64
  {
  public:
    static size_t b64encode(const void * data, size_t len, char ** b64);
    static size_t b64decode(const void * b64, size_t len, char ** data);

  private:
    Base64() { }
    ~Base64() { }
    static const char* B64chars;
    static const int B64index[256];
  };
}

#endif /* BASE64_H */

