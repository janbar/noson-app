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

#include "smapimetadata.h"
#include "private/debug.h"
#include "private/builtin.h"
#include "private/tinyxml2.h"
#include "private/xmldict.h"
#include "didlparser.h"
#include "sonostypes.h"

using namespace NSROOT;

SMAPIMetadata::SMAPIMetadata()
: m_startIndex(0)
, m_itemCount(0)
, m_totalCount(0)
, m_valid(false)
{
}

SMAPIMetadata::SMAPIMetadata(const SMServicePtr& svc, const std::string& xml, const std::string& root)
: m_startIndex(0)
, m_itemCount(0)
, m_totalCount(0)
, m_valid(false)
, m_root(root)
, m_service(svc)
{
  if (m_service)
    m_valid = ParseMessage(xml);
}

void SMAPIMetadata::Reset(const SMServicePtr& svc, const std::string& xml, const std::string& root)
{
  m_valid = false;
  m_service = svc;
  m_list.clear();
  m_startIndex = m_itemCount = m_totalCount = 0;
  m_root.assign(root);
  if (m_service)
    m_valid = ParseMessage(xml);
}

DigitalItemList SMAPIMetadata::GetItems()
{
  DigitalItemList list;
  if (!m_valid)
    return list;
  unsigned c = 0;
  for (ElementList::const_iterator it = m_list.begin(); it != m_list.end(); ++it)
  {
    const Element& media = **it;
    const std::string& itemType = media.GetAttribut("itemType");
    DBG(DBG_PROTO, "%s: [%u] %s (%s)\n", __FUNCTION__, c++, media.GetKey().c_str(), itemType.c_str());
    // initialize the digital item
    DigitalItemPtr item;
    if (itemType == "track")
      item.reset(new DigitalItem(DigitalItem::Type_item, DigitalItem::SubType_audioItem));
    else if (itemType == "stream")
      item.reset(new DigitalItem(DigitalItem::Type_item, DigitalItem::SubType_audioItem));
    else if (itemType == "show")
      item.reset(new DigitalItem(DigitalItem::Type_item, DigitalItem::SubType_audioItem));
    else if (itemType == "program")
      item.reset(new DigitalItem(DigitalItem::Type_item, DigitalItem::SubType_audioItem));
    else if (itemType == "album")
      item.reset(new DigitalItem(DigitalItem::Type_container, DigitalItem::SubType_album));
    else if (itemType == "albumList")
      item.reset(new DigitalItem(DigitalItem::Type_container, DigitalItem::SubType_album));
    else if (itemType == "artist")
      item.reset(new DigitalItem(DigitalItem::Type_container, DigitalItem::SubType_person));
    else if (itemType == "artistTrackList")
      item.reset(new DigitalItem(DigitalItem::Type_container, DigitalItem::SubType_person));
    else if (itemType == "genre")
      item.reset(new DigitalItem(DigitalItem::Type_container, DigitalItem::SubType_genre));
    else if (itemType == "playlist")
      item.reset(new DigitalItem(DigitalItem::Type_container, DigitalItem::SubType_playlistContainer));
    else if (media.GetKey() == "mediaCollection")
      item.reset(new DigitalItem(DigitalItem::Type_container, DigitalItem::SubType_storageFolder));
    else
      item.reset(new DigitalItem(DigitalItem::Type_item, DigitalItem::SubType_unknown));

    switch (item->subType())
    {
    // container
    case DigitalItem::SubType_playlistContainer:
    case DigitalItem::SubType_storageFolder:
      item->SetProperty(DIDL_QNAME_DC "title", media.GetAttribut("title"));
      item->SetProperty(DIDL_QNAME_UPNP "albumArtURI", media.GetAttribut("albumArtURI"));
      item->SetProperty(DIDL_QNAME_RINC "description", media.GetAttribut("summary"));
      break;
    case DigitalItem::SubType_album:
      item->SetProperty(DIDL_QNAME_DC "title", media.GetAttribut("title"));
      item->SetProperty(DIDL_QNAME_UPNP "albumArtURI", media.GetAttribut("albumArtURI"));
      item->SetProperty(DIDL_QNAME_DC "creator", media.GetAttribut("author"));
      item->SetProperty(DIDL_QNAME_DC "contributor", media.GetAttribut("artist"));
      break;
    case DigitalItem::SubType_genre:
      item->SetProperty(DIDL_QNAME_DC "title", media.GetAttribut("title"));
      break;
    case DigitalItem::SubType_person:
      item->SetProperty(DIDL_QNAME_DC "title", media.GetAttribut("title"));
      break;
    // item
    case DigitalItem::SubType_audioItem:
      item->SetProperty(DIDL_QNAME_DC "title", media.GetAttribut("title"));
      if (itemType == "track")
      {
        item->SetProperty(DIDL_QNAME_UPNP "albumArtURI", media.GetAttribut("albumArtURI"));
        item->SetProperty(DIDL_QNAME_DC "creator", media.GetAttribut("composer"));
        item->SetProperty(DIDL_QNAME_DC "contributor", media.GetAttribut("artist"));
        item->SetProperty(DIDL_QNAME_UPNP "album", media.GetAttribut("album"));
        item->SetProperty(DIDL_QNAME_UPNP "originalTrackNumber", media.GetAttribut("trackNumber"));
      }
      else
      {
        item->SetProperty(DIDL_QNAME_UPNP "albumArtURI", media.GetAttribut("logo"));
        item->SetProperty(DIDL_QNAME_RINC "description", media.GetAttribut("summary"));
      }
      break;
    case DigitalItem::SubType_unknown:
    default:
      item->SetProperty(DIDL_QNAME_DC "title", media.GetAttribut("title"));
      item->SetProperty(DIDL_QNAME_UPNP "albumArtURI", media.GetAttribut("albumArtURI"));
    }

    item->SetObjectID(media.GetAttribut("id"));
    item->SetParentID(m_root);

    if (media.GetAttribut("canPlay") == "true")
    {
      if (itemType == "stream")
      {
        // tag <res>
        std::string rval(ProtocolTable[Protocol_xSonosApiStream]);
        rval.append(":").append(item->GetObjectID()).append("?sid=").append(m_service->GetId());
        rval.append("&sn=").append(m_service->GetAccount()->GetSerialNum());
        ElementPtr res(new Element("res", rval));
        item->SetProperty(res);
        // tag <desc>
        ElementPtr desc(new Element("desc", m_service->GetServiceDesc()));
        desc->SetAttribut("id", "cdudn");
        desc->SetAttribut("nameSpace", DIDL_XMLNS_RINC);
        item->SetProperty(desc);
      }
      else if (itemType == "track")
      {
        // tag <res>
        std::string rval(ProtocolTable[Protocol_xSonosHttp]);
        rval.append(":").append(item->GetObjectID()).append("?sid=").append(m_service->GetId());
        rval.append("&sn=").append(m_service->GetAccount()->GetSerialNum());
        ElementPtr res(new Element("res", rval));
        item->SetProperty(res);
        // tag <desc>
        ElementPtr desc(new Element("desc", m_service->GetServiceDesc()));
        desc->SetAttribut("id", "cdudn");
        desc->SetAttribut("nameSpace", DIDL_XMLNS_RINC);
        item->SetProperty(desc);
      }
    }
    list.push_back(item);
  }
  return list;
}

bool SMAPIMetadata::ParseMessage(const std::string& data)
{
  // Parse xml content
  tinyxml2::XMLDocument rootdoc;
  if (rootdoc.Parse(data.c_str(), data.length()) != tinyxml2::XML_SUCCESS)
  {
    DBG(DBG_ERROR, "%s: parse xml failed\n", __FUNCTION__);
    return false;
  }
  const tinyxml2::XMLElement* elem; // an element
  // Check for element
  if (!(elem = rootdoc.RootElement()))
  {
    DBG(DBG_ERROR, "%s: invalid or not supported response\n", __FUNCTION__);
    tinyxml2::XMLPrinter out;
    rootdoc.Accept(&out);
    DBG(DBG_ERROR, "%s\n", out.CStr());
    return false;
  }

  if (XMLNS::NameEqual(elem->Name(), "getMetadataResult") || XMLNS::NameEqual(elem->Name(), "searchResult"))
  {
    const tinyxml2::XMLElement* felem = elem->FirstChildElement(NULL);
    while (felem)
    {
      if (XMLNS::NameEqual(felem->Name(), "index") && felem->GetText())
        string_to_uint32(felem->GetText(), &m_startIndex);
      else if (XMLNS::NameEqual(felem->Name(), "count") && felem->GetText())
        string_to_uint32(felem->GetText(), &m_itemCount);
      else if (XMLNS::NameEqual(felem->Name(), "total") && felem->GetText())
        string_to_uint32(felem->GetText(), &m_totalCount);
      else if (XMLNS::NameEqual(felem->Name(), "mediaCollection"))
      {
        ElementPtr media = ParseMediaCollection(felem);
        if (media)
          m_list.push_back(media);
      }
      else if (XMLNS::NameEqual(felem->Name(), "mediaMetadata"))
      {
        ElementPtr media = ParseMediaMetadata(felem);
        if (media)
          m_list.push_back(media);
      }
      felem = felem->NextSiblingElement(NULL);
    }
  }
  else if (XMLNS::NameEqual(elem->Name(), "getMediaMetadataResult"))
  {
    m_startIndex = 0;
    m_itemCount = m_totalCount = 1;
    ElementPtr media = ParseMediaMetadata(elem);
    if (media)
      m_list.push_back(media);
  }
  else
  {
    DBG(DBG_ERROR, "%s: invalid or not supported response (%s)\n", __FUNCTION__, elem->Name());
    return false;
  }
  return true;
}

ElementPtr SMAPIMetadata::ParseMediaCollection(const void* _elem)
{
  ElementPtr ptr;
  const tinyxml2::XMLElement* elem = static_cast<const tinyxml2::XMLElement*>(_elem);
  if (!elem)
    return ptr;
  ptr.reset(new Element("mediaCollection"));
  const tinyxml2::XMLElement* felem = elem->FirstChildElement(NULL);
  while (felem)
  {
    if (XMLNS::NameEqual(felem->Name(), "id") && felem->GetText())
      ptr->SetAttribut("id", felem->GetText());
    else if (XMLNS::NameEqual(felem->Name(), "title") && felem->GetText())
      ptr->SetAttribut("title", felem->GetText());
    else if (XMLNS::NameEqual(felem->Name(), "itemType") && felem->GetText())
      ptr->SetAttribut("itemType", felem->GetText());
    else if (XMLNS::NameEqual(felem->Name(), "canPlay") && felem->GetText())
      ptr->SetAttribut("canPlay", felem->GetText());
    else if (XMLNS::NameEqual(felem->Name(), "canEnumerate") && felem->GetText())
      ptr->SetAttribut("canEnumerate", felem->GetText());
    else if (XMLNS::NameEqual(felem->Name(), "canScroll") && felem->GetText())
      ptr->SetAttribut("canScroll", felem->GetText());
    else if (XMLNS::NameEqual(felem->Name(), "canSkip") && felem->GetText())
      ptr->SetAttribut("canSkip", felem->GetText());
    else if (XMLNS::NameEqual(felem->Name(), "canAddToFavorites") && felem->GetText())
      ptr->SetAttribut("canAddToFavorites", felem->GetText());
    else if (XMLNS::NameEqual(felem->Name(), "isFavorite") && felem->GetText())
      ptr->SetAttribut("isFavorite", felem->GetText());
    else if (XMLNS::NameEqual(felem->Name(), "containsFavorite") && felem->GetText())
      ptr->SetAttribut("containsFavorite", felem->GetText());
    else if (XMLNS::NameEqual(felem->Name(), "summary") && felem->GetText())
      ptr->SetAttribut("summary", felem->GetText());
    else if (XMLNS::NameEqual(felem->Name(), "albumArtURI") && felem->GetText())
      ptr->SetAttribut("albumArtURI", felem->GetText());
    else if (XMLNS::NameEqual(felem->Name(), "artist") && felem->GetText())
      ptr->SetAttribut("artist", felem->GetText());
    else if (XMLNS::NameEqual(felem->Name(), "artistId") && felem->GetText())
      ptr->SetAttribut("artistId", felem->GetText());
    else if (XMLNS::NameEqual(felem->Name(), "author") && felem->GetText())
      ptr->SetAttribut("author", felem->GetText());
    else if (XMLNS::NameEqual(felem->Name(), "authorId") && felem->GetText())
      ptr->SetAttribut("authorId", felem->GetText());
    felem = felem->NextSiblingElement(NULL);
  }
  return ptr;
}

ElementPtr SMAPIMetadata::ParseMediaMetadata(const void* _elem)
{
  ElementPtr ptr;
  const tinyxml2::XMLElement* elem = static_cast<const tinyxml2::XMLElement*>(_elem);
  if (!elem)
    return ptr;
  ptr.reset(new Element("mediaMetadata"));
  const tinyxml2::XMLElement* felem = elem->FirstChildElement(NULL);
  while (felem)
  {
    if (XMLNS::NameEqual(felem->Name(), "id") && felem->GetText())
      ptr->SetAttribut("id", felem->GetText());
    else if (XMLNS::NameEqual(felem->Name(), "title") && felem->GetText())
      ptr->SetAttribut("title", felem->GetText());
    else if (XMLNS::NameEqual(felem->Name(), "itemType") && felem->GetText())
      ptr->SetAttribut("itemType", felem->GetText());
    else if (XMLNS::NameEqual(felem->Name(), "mimeType") && felem->GetText())
      ptr->SetAttribut("mimeType", felem->GetText());
    else if (XMLNS::NameEqual(felem->Name(), "isFavorite") && felem->GetText())
      ptr->SetAttribut("isFavorite", felem->GetText());
    else if (XMLNS::NameEqual(felem->Name(), "summary") && felem->GetText())
      ptr->SetAttribut("summary", felem->GetText());
    else if (XMLNS::NameEqual(felem->Name(), "streamMetadata"))
      ParseStreamMetadata(felem, *ptr);
    else if (XMLNS::NameEqual(felem->Name(), "trackMetadata"))
      ParseTrackMetadata(felem, *ptr);
    felem = felem->NextSiblingElement(NULL);
  }
  return ptr;
}

bool SMAPIMetadata::ParseStreamMetadata(const void* _elem, Element& stream)
{
  const tinyxml2::XMLElement* elem = static_cast<const tinyxml2::XMLElement*>(_elem);
  if (!elem)
    return false;
  const tinyxml2::XMLElement* felem = elem->FirstChildElement(NULL);
  stream.SetAttribut("canPlay", "true");
  while (felem)
  {
    if (XMLNS::NameEqual(felem->Name(), "bitrate") && felem->GetText())
      stream.SetAttribut("bitrate", felem->GetText());
    else if (XMLNS::NameEqual(felem->Name(), "currentHost") && felem->GetText())
      stream.SetAttribut("currentHost", felem->GetText());
    else if (XMLNS::NameEqual(felem->Name(), "currentShow") && felem->GetText())
      stream.SetAttribut("currentShow", felem->GetText());
    else if (XMLNS::NameEqual(felem->Name(), "currentShowId") && felem->GetText())
      stream.SetAttribut("currentShowId", felem->GetText());
    else if (XMLNS::NameEqual(felem->Name(), "logo") && felem->GetText())
      stream.SetAttribut("logo", felem->GetText());
    felem = felem->NextSiblingElement(NULL);
  }
  return true;
}

bool SMAPIMetadata::ParseTrackMetadata(const void* _elem, Element& track)
{
  const tinyxml2::XMLElement* elem = static_cast<const tinyxml2::XMLElement*>(_elem);
  if (!elem)
    return false;
  const tinyxml2::XMLElement* felem = elem->FirstChildElement(NULL);
  track.SetAttribut("canAddToFavorites", "true");
  track.SetAttribut("canPlay", "true");
  track.SetAttribut("canSeek", "true");
  track.SetAttribut("canSkip", "true");
  while (felem)
  {
    if (XMLNS::NameEqual(felem->Name(), "album") && felem->GetText())
      track.SetAttribut("album", felem->GetText());
    else if (XMLNS::NameEqual(felem->Name(), "albumId") && felem->GetText())
      track.SetAttribut("albumId", felem->GetText());
    else if (XMLNS::NameEqual(felem->Name(), "artist") && felem->GetText())
      track.SetAttribut("artist", felem->GetText());
    else if (XMLNS::NameEqual(felem->Name(), "artistId") && felem->GetText())
      track.SetAttribut("artistId", felem->GetText());
    else if (XMLNS::NameEqual(felem->Name(), "genre") && felem->GetText())
      track.SetAttribut("genre", felem->GetText());
    else if (XMLNS::NameEqual(felem->Name(), "genreId") && felem->GetText())
      track.SetAttribut("genreId", felem->GetText());
    else if (XMLNS::NameEqual(felem->Name(), "composer") && felem->GetText())
      track.SetAttribut("composer", felem->GetText());
    else if (XMLNS::NameEqual(felem->Name(), "composerId") && felem->GetText())
      track.SetAttribut("composerId", felem->GetText());
    else if (XMLNS::NameEqual(felem->Name(), "trackNumber") && felem->GetText())
      track.SetAttribut("trackNumber", felem->GetText());
    else if (XMLNS::NameEqual(felem->Name(), "albumArtURI") && felem->GetText())
      track.SetAttribut("albumArtURI", felem->GetText());
    else if (XMLNS::NameEqual(felem->Name(), "canAddToFavorites") && felem->GetText())
      track.SetAttribut("canAddToFavorites", felem->GetText());
    else if (XMLNS::NameEqual(felem->Name(), "canPlay") && felem->GetText())
      track.SetAttribut("canPlay", felem->GetText());
    else if (XMLNS::NameEqual(felem->Name(), "canResume") && felem->GetText())
      track.SetAttribut("canResume", felem->GetText());
    else if (XMLNS::NameEqual(felem->Name(), "canSeek") && felem->GetText())
      track.SetAttribut("canSeek", felem->GetText());
    else if (XMLNS::NameEqual(felem->Name(), "canSkip") && felem->GetText())
      track.SetAttribut("canSkip", felem->GetText());
    felem = felem->NextSiblingElement(NULL);
  }
  return true;
}
