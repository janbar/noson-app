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

#ifndef XMLNAME_H
#define XMLNAME_H

#include <local_config.h>

#define XMLNAME_MAXLEN  63

namespace NSROOT
{

  class XMLName
  {
  public:
    XMLName(const char* qname);
    virtual ~XMLName() {}

    // compare prefix of a qualified element name
    static bool XMLPrefixEqual(const char* qname, const char* prefix);

    // compare name of qualified element name
    static bool XMLNameEqual(const char* qname, const char* name);

    const char* Prefix() const { return m_prefix; }
    const char* Name() const { return m_name; }

  private:
    char m_prefix[XMLNAME_MAXLEN + 1];
    char m_name[XMLNAME_MAXLEN + 1];
    int m_prefixLen;
    int m_nameLen;
  };

}

#endif /* XMLNAME_H */

