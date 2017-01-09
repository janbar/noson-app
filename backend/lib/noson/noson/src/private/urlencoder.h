/*
 *      Copyright (C) 2017 Jean-Luc Barriere
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

#ifndef URLENCODER_H
#define URLENCODER_H

#include <cstdio>
#include <cstring>
#include <string>

#define urlencode __urlencode
inline std::string __urlencode(const std::string& str) {
  std::string out;
  out.reserve(2 * str.length());
  const char* cstr = str.c_str();
  while (*cstr)
  {
    if (isalnum(*cstr) || *cstr == '-' || *cstr == '_' || *cstr == '.' || *cstr == '~')
      out.push_back(*cstr);
    else {
      char buf[4];
      sprintf(buf, "%%%.2x", static_cast<unsigned char>(*cstr));
      out.append(buf);
    }
    ++cstr;
  }
  return out;
}

#define urldecode __urldecode
inline std::string __urldecode(const std::string& str) {
  std::string out;
  out.reserve(str.length());
  const char* cstr = str.c_str();
  while (*cstr)
  {
    char c = *cstr;
    if (c == '%')
    {
      int v;
      char buf[3];
      strncpy(buf, cstr + 1, 3);
      buf[2] = '\0';
      if (sscanf(buf, "%x", &v) == 1 || sscanf(buf, "%X", &v) == 1)
      {
        c = static_cast<char>(v);
        cstr += 2;
      }
    }
    out.push_back(c);
    ++cstr;
  }
  return out;
}

#endif /* URLENCODER_H */

