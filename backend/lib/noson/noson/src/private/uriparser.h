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

#ifndef URIPARSER_H
#define	URIPARSER_H

#include "local_config.h"
#include <string>

namespace NSROOT
{
  
  class URIParser
  {
  public:
    URIParser(const std::string& location);
    ~URIParser();

    const char *Scheme() const { return m_parts.scheme; }
    const char *Host() const { return m_parts.host; }
    unsigned Port() const { return m_parts.port; }
    const char *User() const { return m_parts.user; }
    const char *Pass() const { return m_parts.pass; }
    bool IsRelative() const { return m_parts.relative ? true : false; }
    const char *Path() const { return IsRelative() ? m_parts.relative : m_parts.absolute; }
    const char *Fragment() const { return m_parts.fragment; }

  private:
    // prevent copy
    URIParser(const URIParser&);
    URIParser& operator=(const URIParser&);

    typedef struct
    {
      char      *scheme;
      char      *host;
      unsigned  port;
      char      *user;
      char      *pass;
      char      *absolute;
      char      *relative;
      char      *fragment;
    } URI_t;

    URI_t m_parts;
    char *m_buffer;

    static void URIScan(char *uri, URI_t *parts);
  };

}

#endif	/* URIPARSER_H */
