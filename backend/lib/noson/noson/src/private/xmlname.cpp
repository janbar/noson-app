/*
 *      Copyright (C) 2016 Jean-Luc Barriere
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

#include "xmlname.h"
#include <string.h>

using namespace NSROOT;

#define MIN(a,b)  ((a) < (b) ? (a) : (b))
#define MAX(a,b)  ((a) > (b) ? (a) : (b))
#define ZERO      '\0'

XMLName::XMLName(const char* qname)
: m_prefixLen(0)
, m_nameLen(0)
{
  memset(m_prefix, ZERO, XMLNAME_MAXLEN);
  memset(m_name, ZERO, XMLNAME_MAXLEN);
  // fill working buffers
  if (qname)
  {
    for (const char* p = qname; ; ++p)
    {
      if (*p == ':')
      {
        int len = p - qname;
        m_prefixLen = MIN(len , XMLNAME_MAXLEN);
        memcpy(m_prefix, qname, m_prefixLen);
        len = strlen(++p);
        m_nameLen =  MIN(len, XMLNAME_MAXLEN);
        memcpy(m_name, p, m_nameLen);
        break;
      }
      if (*p == ZERO)
      {
        m_prefixLen = 0;
        int len = strlen(qname);
        m_nameLen =  MIN(len, XMLNAME_MAXLEN);
        memcpy(m_name, qname, m_nameLen);
        break;
      }
    }
  }
}

bool XMLName::XMLPrefixEqual(const char* qname, const char* prefix)
{
  unsigned n = 0;
  const char* p = qname;
  while (*p != ZERO)
    if (*(++p) == ':')
    {
      n = p - qname;
      break;
    }
  return (strlen(prefix) == n && strncmp(qname, prefix, n) == 0);
}

bool XMLName::XMLNameEqual(const char* qname, const char* name)
{
  const char* n = qname;
  const char* p = qname;
  while (*p != ZERO)
    if (*(++p) == ':')
    {
      n = p + 1;
      break;
    }
  return (strcmp(n, name) == 0);
}
