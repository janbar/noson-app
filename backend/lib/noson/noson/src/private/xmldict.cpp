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

#include "xmldict.h"
#include <string.h>

using namespace NSROOT;

#define ZERO      '\0'

bool XMLNS::PrefixEqual(const char* qname, const char* prefix)
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

bool XMLNS::NameEqual(const char* qname, const char* name)
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

const char* XMLNS::LocalName(const char* qname)
{
  const char* n = qname;
  const char* p = qname;
  while (*p != ZERO)
    if (*(++p) == ':')
    {
      n = p + 1;
      break;
    }
  return n;
}

///////////////////////////////////////////////////////////////////////////////
////
//// XMLDict
////

XMLDict::XMLDict()
{
}

void XMLDict::DefineNS(const char* key, const char* name)
{
  // remove ending colon from key
  std::string _key;
  const char* p = key;
  while (*p != ZERO && *p != ':') ++p;
  if (p > key)
    _key.assign(key, p - key);

  XMLNS* ns = FindKey(_key.c_str());
  if (ns)
  {
    ns->name.assign(name);
    // rebuild xml string
    m_xmlstring.assign(" ");
    for (XMLNSList::iterator it = m_nsout.begin(); it != m_nsout.end(); ++it)
    {
      if (it->key.empty())
        m_xmlstring.append("xmlns");
      else
        m_xmlstring.append("xmlns:").append(it->key);
      m_xmlstring.append("=\"").append(it->name).append("\" ");
    }
  }
  else
  {
    m_nsout.push_back(XMLNS(_key.c_str(), name));
    // update xml string
    XMLNS& xmlns = m_nsout.back();
    if (m_xmlstring.empty())
      m_xmlstring.assign(" ");
    if (xmlns.key.empty())
      m_xmlstring.append("xmlns");
    else
      m_xmlstring.append("xmlns:").append(xmlns.key);
    m_xmlstring.append("=\"").append(xmlns.name).append("\" ");
  }
}

const char* XMLDict::KeyForName(const char* name)
{
  XMLNSList::iterator it;
  for (it = m_nsout.begin(); it != m_nsout.end(); ++it)
    if (it->name.compare(name) == 0)
      return it->key.c_str();
  return 0;
}

std::string XMLDict::TranslateQName(const XMLNames& names, const char* qname)
{
  unsigned l = 0;
  const char* n = qname;
  const char* p = qname;
  while (*p != ZERO)
    if (*(++p) == ':')
    {
      l = p - qname;
      n = p + 1;
      break;
    }
  std::string prefix(qname, l);
  const XMLNS* in = names.FindKey(prefix.c_str());
  if (in)
  {
    XMLNSList::const_iterator it;
    for (it = m_nsout.begin(); it != m_nsout.end(); ++it)
      if (it->name.compare(in->name) == 0)
      {
        if (it->key.empty())
          return std::string(n); // return local name without qualifier
        return std::string(it->key).append(":").append(n);
      }
  }
  // cannot translate qualified name
  return std::string(qname);
}

XMLNS* XMLDict::FindKey(const char* key)
{
  XMLNSList::iterator it;
  for (it = m_nsout.begin(); it != m_nsout.end(); ++it)
    if (it->key.compare(key) == 0)
      return &(*it);
  return 0;
}

///////////////////////////////////////////////////////////////////////////////
////
//// XMLNames
////

void XMLNames::AddXMLNS(const tinyxml2::XMLElement* e)
{
  const tinyxml2::XMLAttribute* attr = e->FirstAttribute();
  while (attr)
  {
    const char* key = 0;
    if (XMLNS::PrefixEqual(attr->Name(), "xmlns"))
      key = XMLNS::LocalName(attr->Name());
    else if (XMLNS::NameEqual(attr->Name(), "xmlns"))
      key = "";
    if (key)
      AddXMLNS(key, attr->Value());
    attr = attr->Next();
  }
}

void XMLNames::AddXMLNS(const char* key, const char* name)
{
  XMLNS* ns = FindKey(key);
  if (ns)
    ns->name.assign(name);
  else
    m_names.push_back(XMLNS(key, name));
}

XMLNS* XMLNames::FindKey(const char* key)
{
  XMLNSList::iterator it;
  for (it = m_names.begin(); it != m_names.end(); ++it)
    if (it->key.compare(key) == 0)
      return &(*it);
  return 0;
}

const XMLNS* XMLNames::FindKey(const char* key) const
{
  XMLNSList::const_iterator it;
  for (it = m_names.begin(); it != m_names.end(); ++it)
    if (it->key.compare(key) == 0)
      return &(*it);
  return 0;
}

XMLNS* XMLNames::FindName(const char* name)
{
  XMLNSList::iterator it;
  for (it = m_names.begin(); it != m_names.end(); ++it)
    if (it->name.compare(name) == 0)
      return &(*it);
  return 0;
}

const XMLNS* XMLNames::FinfName(const char* name) const
{
  XMLNSList::const_iterator it;
  for (it = m_names.begin(); it != m_names.end(); ++it)
    if (it->name.compare(name) == 0)
      return &(*it);
  return 0;
}
