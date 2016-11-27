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
#include "tinyxml2.h"

#include <string>
#include <list>

namespace NSROOT
{

  struct XMLNS
  {
    XMLNS(const char* k, const char* n) : key(k), name(n) {}
    XMLNS(const char* n) : key(), name(n) {}
    virtual ~XMLNS() {}

    std::string key;
    std::string name;

    // compare prefix of a qualified element name
    static bool PrefixEqual(const char* qname, const char* prefix);

    // compare name of qualified element name
    static bool NameEqual(const char* qname, const char* name);

    // return the local part of qualified name
    static const char* LocalName(const char* qname);
  };

  class XMLNames;

  class XMLDict
  {
  public:
    XMLDict();
    virtual ~XMLDict() {}

    void DefineNS(const char* key, const char* name);
    void DefineNS(const char* name) { DefineNS("", name); }
    const char* ToString() { return m_xmlstring.c_str(); }
    const char* KeyForName(const char* name);
    std::string TranslateQName(const XMLNames& names, const char* qname);

  private:
    typedef std::list<XMLNS> XMLNSList;
    XMLNSList m_nsout;

    std::string m_xmlstring;

    XMLNS* FindKey(const char* key);
  };

  class XMLNames
  {
  public:
    XMLNames() {}
    virtual ~XMLNames() {}

    void AddXMLNS(const tinyxml2::XMLElement* e);
    void AddXMLNS(const char* key, const char* name);
    void AddXMLNS(const char* name) { AddXMLNS("", name); }

    XMLNS* FindKey(const char* key);
    const XMLNS* FindKey(const char* key) const;

  private:
    typedef std::list<XMLNS> XMLNSList;
    XMLNSList m_names;
  };
}

#endif /* XMLNAME_H */

