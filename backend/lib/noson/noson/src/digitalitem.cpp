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

#include "digitalitem.h"
#include "private/builtin.h"

#include <vector>

using namespace NSROOT;

inline void __tokenize(const std::string& str, const char *delimiters, std::vector<std::string>& tokens, bool trimnull = false)
{
  std::string::size_type pa = 0, pb = 0;
  unsigned n = 0;
  // Counter n will break infinite loop. Max count is 255 tokens
  while ((pb = str.find_first_of(delimiters, pb)) != std::string::npos && ++n < 255)
  {
    tokens.push_back(str.substr(pa, pb - pa));
    do
    {
      pa = ++pb;
    }
    while (trimnull && str.find_first_of(delimiters, pb) == pb);
  }
  tokens.push_back(str.substr(pa));
}

const char* DigitalItem::TypeTable[Type_unknown + 1] = {
  "container", "item", ""
};

const char* DigitalItem::SubTypeTable[SubType_unknown + 1] = {
  "playlistContainer", "album", "genre", "person", "channelGroup", "epgContainer", "storageSystem", "storageVolume", "storageFolder", "bookmarkFolder",
  "audioItem", "videoItem", "imageItem", "playlistItem", "textItem", "bookmarkItem", "epgItem", ""
};

DigitalItem::DigitalItem(Type_t _type, SubType_t _subType)
: m_type(_type)
, m_subType(_subType)
, m_restricted(false)
, m_objectID("")
, m_parentID("")
{
  ElementPtr _class(new Element("upnp:class"));
  _class->assign("object");
  if (m_type != Type_unknown)
  {
    _class->append(".").append(TypeTable[m_type]);
    if (m_subType != SubType_unknown)
      _class->append(".").append(SubTypeTable[m_subType]);
  }
  m_vars.push_back(_class);
}

DigitalItem::DigitalItem(const std::string& objectID, const std::string& parentID, bool restricted, const ElementList& vars)
: m_type(Type_unknown)
, m_subType(SubType_unknown)
, m_restricted(restricted)
, m_objectID(objectID)
, m_parentID(parentID)
, m_vars(vars)
{
  ElementList::const_iterator it;
  if ((it = vars.FinKey("upnp:class")) != vars.end())
  {
    std::vector<std::string> tokens;
    __tokenize((*it)->c_str(), ".", tokens);
    if (tokens.size() >= 2 && tokens[0] == "object")
    {
      if (tokens[1] == TypeTable[Type_container])
        m_type = Type_container;
      else
        m_type = Type_item;
      if (tokens.size() >= 3)
      {
        for (unsigned i = 0; i < SubType_unknown; ++i)
        {
          if (tokens[2] != SubTypeTable[i])
            continue;
          m_subType = (SubType_t)i;
          break;
        }
      }
    }
  }
}

const ElementPtr DigitalItem::GetProperty(const std::string& key) const
{
  ElementList::const_iterator it = m_vars.FinKey(key);
  if (it != m_vars.end())
    return *it;
  else
    return ElementPtr(NULL);
}

std::vector<ElementPtr> DigitalItem::GetCollection(const std::string& key) const
{
  std::vector<ElementPtr> list;
  ElementList::const_iterator it = m_vars.FinKey(key);
  if (it != m_vars.end())
  {
    do {
      list.push_back(*it);
    } while (m_vars.FindKey(key, ++it) != m_vars.end());
  }
  return list;
}

const ElementPtr& DigitalItem::SetProperty(const ElementPtr& var)
{
  if (var)
  {
    ElementList::iterator it = m_vars.FinKey(var->GetKey());
    if (it != m_vars.end())
      return *it = var;
    m_vars.push_back(var);
    return m_vars.back();
  }
  return var;
}

std::string DigitalItem::DIDL() const
{
  std::string xml;
  xml.append(
    "<DIDL-Lite xmlns:dc=\"http://purl.org/dc/elements/1.1/\" "
    "xmlns:upnp=\"urn:schemas-upnp-org:metadata-1-0/upnp/\" "
    "xmlns:r=\"urn:schemas-rinconnetworks-com:metadata-1-0/\" "
    "xmlns=\"urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/\">");
  if (m_type != Type_unknown)
  {
    xml.append("<").append(TypeTable[m_type]).append(" id=\"").append(m_objectID).append("\" parentID=\"").append(m_parentID).append("\"")
            .append(" restricted=\"").append((m_restricted ? "true" : "false")).append("\">");
    for (ElementList::const_iterator it = m_vars.begin(); it != m_vars.end(); ++it)
    {
      if (*it)
        xml.append((*it)->XML());
    }
    xml.append("</").append(TypeTable[m_type]).append(">");
  }
  xml.append("</DIDL-Lite>");
  return xml;
}
