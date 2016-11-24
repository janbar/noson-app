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

#include "didlparser.h"
#include "private/tinyxml2.h"
#include "private/xmlname.h"
#include "private/debug.h"

using namespace NSROOT;

DIDLParser::DIDLParser(const char* document, unsigned reserve)
: m_document(document)
, m_parsed(false)
{
  if (reserve)
    m_items.reserve(reserve);
  m_parsed = Parse();
}

bool DIDLParser::Parse()
{
  m_items.clear();
  tinyxml2::XMLDocument doc;
  if (doc.Parse(m_document) != tinyxml2::XML_SUCCESS)
    return false;
  const tinyxml2::XMLElement* elem;
  if ((elem = doc.RootElement()) && XMLName::XMLNameEqual(elem->Name(), "DIDL-Lite"))
  {
    elem = elem->FirstChildElement();
    while (elem)
    {
      const char* val;
      struct { const char* id; const char* parentID; bool restricted; } ref;
      if ((XMLName::XMLNameEqual(elem->Name(), "item") ||
              XMLName::XMLNameEqual(elem->Name(), "container")) &&
              (ref.id = elem->Attribute("id")) &&
              (ref.parentID = elem->Attribute("parentID")) &&
              (val = elem->Attribute("restricted")))
      {
        ref.restricted = (0 == strncmp(val, "true", 4) ? true : false);
        ElementList vars;
        const tinyxml2::XMLElement* velem = elem->FirstChildElement();
        while (velem)
        {
          if (velem->Name() && velem->GetText())
          {
            ElementPtr var(new Element(velem->Name(), velem->GetText()));
            const tinyxml2::XMLAttribute* vattr = velem->FirstAttribute();
            while (vattr && vattr->Name() && vattr->Value())
            {
              var->SetAttribut(vattr->Name(), vattr->Value());
              vattr = vattr->Next();
            }
            vars.push_back(var);
          }
          velem = velem->NextSiblingElement();
        }
        m_items.push_back(DigitalItemPtr(new DigitalItem(ref.id, ref.parentID, ref.restricted, vars)));
      }
      elem = elem->NextSiblingElement();
    }
    return true;
  }
  return false;
}
