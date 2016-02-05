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

#include "uriparser.h"
#include "cppdef.h"

#include <ctype.h> // for toupper
#include <cstring> // for strchr
#include <cstdlib> // for atoi

using namespace NSROOT;

URIParser::URIParser(const std::string& location)
{
  size_t len = location.length();
  m_buffer = new char[len + 1];
  strncpy(m_buffer, location.c_str(), len);
  m_buffer[len] = '\0';
  URIScan(m_buffer, &m_parts);
}

URIParser::~URIParser()
{
  SAFE_DELETE_ARRAY(m_buffer);
}

void URIParser::URIScan(char *uri, URI_t *parts)
{
  char *p;
  char *after_scheme = uri;
  memset(parts, '\0', sizeof(URI_t));

  /* look for fragment identifier */
  if ((p = strchr(uri, '#')) != NULL)
  {
    *p = '\0';
    parts->fragment = ++p;
  }
  if ((p = strchr(uri, ' ')) != NULL)
    *p = '\0';

  for (p = after_scheme; *p; p++)
  {
    if (*p == '/' || *p == '#' || *p == '?')
      break;
    if (*p == ':')
    {
      /* terminate scheme */
      *p = '\0';
      /* scheme has been specified */
      if (toupper(after_scheme[0]) == 'U'
              && toupper(after_scheme[1]) == 'R'
              && toupper(after_scheme[2]) == 'L')
      {
        /* ignore IETF's URL: pre-prefix */
        parts->scheme = NULL;
      }
      else
      {
        parts->scheme = after_scheme;
      }
      after_scheme = p + 1;
      break;
    }
  }

  p = after_scheme;
  if (*p == '/')
  {
    if (p[1] == '/')
    {
      /* host has been specified */
      parts->host = p + 2;
      /* terminate scheme */
      *p = '\0';
      /* look for end of host:port if any */
      if ((p = strchr(parts->host, '/')) != NULL)
      {
        /* terminate host:port */
        *p = '\0';
        /* root has been found */
        parts->absolute = p + 1;
      }
      /* look for user:pass from host */
      if ((p = strchr(parts->host, '@')) != NULL)
      {
        /* terminate user:pass */
        *p = '\0';
        parts->user = parts->host;
        parts->host = p + 1;
        /* look for pass from user */
        if ((p = strchr(parts->user, ':')) != NULL)
        {
          /* terminate user */
          *p = '\0';
          parts->pass = p + 1;
        }
      }
      /* look for port from [host] */
      if ((p = strchr(parts->host, ']')) != NULL)
      {
        /* terminate host */
        *p = '\0';
        ++(parts->host);
        if (p[1] == ':')
          parts->port = (unsigned) atoi(p + 2);
      }
      else
      {
        /* look for port from host */
        if ((p = strchr(parts->host, ':')) != NULL)
        if (p)
        {
          /* terminate host */
          *p = '\0';
          parts->port = (unsigned) atoi(p + 1);
        }
      }
    }
    else
    {
      /* root found but no host */
      parts->absolute = p + 1;
    }
  }
  else
  {
    /* NULL for "" */
    parts->relative = (*after_scheme) ? after_scheme : NULL;
  }
}
