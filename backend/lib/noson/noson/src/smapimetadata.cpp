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

SMAPIItemList SMAPIMetadata::GetItems()
{
  SMAPIItemList list;
  if (!m_valid)
    return list;
  unsigned count = 0;
  for (ElementList::const_iterator it = m_list.begin(); it != m_list.end(); ++it)
  {
    const Element& media = **it;
    const std::string& itemType = media.GetAttribut("itemType");
    const std::string& mimeType = media.GetAttribut("mimeType");
    DBG(DBG_PROTO, "%s: [%u] %s (%s)(%s)\n", __FUNCTION__, count++, media.GetKey().c_str(), itemType.c_str(), mimeType.c_str());

    // initialize the item
    SMAPIItem data;
    data.displayType = SMAPIItem::Grid;

    if (itemType == "track")
      data.item.reset(new DigitalItem(DigitalItem::Type_item, DigitalItem::SubType_audioItem));
    else if (itemType == "stream")
      data.item.reset(new DigitalItem(DigitalItem::Type_item, DigitalItem::SubType_audioItem));
    else if (itemType == "program")
      data.item.reset(new DigitalItem(DigitalItem::Type_item, DigitalItem::SubType_audioItem));
    else if (itemType == "show")
      data.item.reset(new DigitalItem(DigitalItem::Type_container, DigitalItem::SubType_playlistContainer));
    else if (itemType == "album")
      data.item.reset(new DigitalItem(DigitalItem::Type_container, DigitalItem::SubType_album));
    else if (itemType == "albumList")
      data.item.reset(new DigitalItem(DigitalItem::Type_container, DigitalItem::SubType_storageFolder));
    else if (itemType == "artist")
      data.item.reset(new DigitalItem(DigitalItem::Type_container, DigitalItem::SubType_person));
    else if (itemType == "artistTrackList")
      data.item.reset(new DigitalItem(DigitalItem::Type_container, DigitalItem::SubType_playlistContainer));
    else if (itemType == "genre")
      data.item.reset(new DigitalItem(DigitalItem::Type_container, DigitalItem::SubType_genre));
    else if (itemType == "playlist")
      data.item.reset(new DigitalItem(DigitalItem::Type_container, DigitalItem::SubType_playlistContainer));
    else if (media.GetKey() == "mediaCollection")
      data.item.reset(new DigitalItem(DigitalItem::Type_container, DigitalItem::SubType_storageFolder));
    else
      data.item.reset(new DigitalItem(DigitalItem::Type_item, DigitalItem::SubType_unknown));

    switch (data.item->subType())
    {
    // container
    case DigitalItem::SubType_playlistContainer:
      data.item->SetProperty(DIDL_QNAME_DC "title", media.GetAttribut("title"));
      data.item->SetProperty(DIDL_QNAME_UPNP "albumArtURI", media.GetAttribut("albumArtURI"));
      data.item->SetProperty(DIDL_QNAME_RINC "description", media.GetAttribut("summary"));
      data.displayType = SMAPIItem::List;
      break;
    case DigitalItem::SubType_storageFolder:
      data.item->SetProperty(DIDL_QNAME_DC "title", media.GetAttribut("title"));
      data.item->SetProperty(DIDL_QNAME_UPNP "albumArtURI", media.GetAttribut("albumArtURI"));
      data.item->SetProperty(DIDL_QNAME_RINC "description", media.GetAttribut("summary"));
      break;
    case DigitalItem::SubType_album:
      data.item->SetProperty(DIDL_QNAME_DC "title", media.GetAttribut("title"));
      data.item->SetProperty(DIDL_QNAME_UPNP "albumArtURI", media.GetAttribut("albumArtURI"));
      data.item->SetProperty(DIDL_QNAME_DC "creator", media.GetAttribut("author"));
      data.item->SetProperty(DIDL_QNAME_DC "contributor", media.GetAttribut("artist"));
      data.displayType = SMAPIItem::List;
      break;
    case DigitalItem::SubType_genre:
      data.item->SetProperty(DIDL_QNAME_DC "title", media.GetAttribut("title"));
      data.item->SetProperty(DIDL_QNAME_UPNP "albumArtURI", media.GetAttribut("albumArtURI"));
      break;
    case DigitalItem::SubType_person:
      data.item->SetProperty(DIDL_QNAME_DC "title", media.GetAttribut("title"));
      data.item->SetProperty(DIDL_QNAME_UPNP "albumArtURI", media.GetAttribut("albumArtURI"));
      break;
    // item
    case DigitalItem::SubType_audioItem:
      data.item->SetProperty(DIDL_QNAME_DC "title", media.GetAttribut("title"));
      if (itemType == "track")
      {
        data.item->SetProperty(DIDL_QNAME_UPNP "albumArtURI", media.GetAttribut("albumArtURI"));
        data.item->SetProperty(DIDL_QNAME_DC "creator", media.GetAttribut("composer"));
        data.item->SetProperty(DIDL_QNAME_DC "contributor", media.GetAttribut("artist"));
        data.item->SetProperty(DIDL_QNAME_UPNP "album", media.GetAttribut("album"));
        data.item->SetProperty(DIDL_QNAME_UPNP "originalTrackNumber", media.GetAttribut("trackNumber"));
      }
      else
      {
        data.item->SetProperty(DIDL_QNAME_UPNP "albumArtURI", media.GetAttribut("logo"));
        data.item->SetProperty(DIDL_QNAME_RINC "description", media.GetAttribut("summary"));
      }
      break;
    case DigitalItem::SubType_unknown:
    default:
      data.item->SetProperty(DIDL_QNAME_DC "title", media.GetAttribut("title"));
      data.item->SetProperty(DIDL_QNAME_UPNP "albumArtURI", media.GetAttribut("albumArtURI"));
    }

    data.item->SetObjectID(media.GetAttribut("id"));
    data.item->SetParentID(m_root);

    // overriding default display type
    const std::string& itemDisplayType = media.GetAttribut("displayType");
    if (itemDisplayType == "List")
      data.displayType = SMAPIItem::List;
    else if (itemDisplayType == "Hero")
      data.displayType = SMAPIItem::Hero;
    else if (itemDisplayType == "Editorial")
      data.displayType = SMAPIItem::Editorial;

    if (media.GetAttribut("canPlay") == "true")
    {
      if (itemType == "stream")
      {
        // clone the item as skeleton for uri metadata
        data.uriMetadata.reset(new DigitalItem(DigitalItem::Type_unknown, DigitalItem::SubType_unknown));
        data.item->Clone(*(data.uriMetadata));
        // tag <res>
        std::string rval(ProtocolTable[Protocol_xSonosApiStream]);
        rval.append(":").append(data.item->GetObjectID()).append("?sid=").append(m_service->GetId());
        rval.append("&sn=").append(m_service->GetAccount()->GetSerialNum());
        ElementPtr res(new Element("res", rval));
        data.uriMetadata->SetProperty(res);
        // tag <desc>
        ElementPtr desc(new Element("desc", m_service->GetServiceDesc()));
        desc->SetAttribut("id", "cdudn");
        desc->SetAttribut("nameSpace", DIDL_XMLNS_RINC);
        data.uriMetadata->SetProperty(desc);
      }
      else if (itemType == "track")
      {
        if (mimeType == "audio/vnd.radiotime")
        {
          // clone the item as skeleton for uri metadata
          data.uriMetadata.reset(new DigitalItem(DigitalItem::Type_unknown, DigitalItem::SubType_unknown));
          data.item->Clone(*(data.uriMetadata));
          // tag <res>
          std::string rval(ProtocolTable[Protocol_xSonosApiRTRecent]);
          rval.append(":").append(data.item->GetObjectID()).append("?sid=").append(m_service->GetId());
          rval.append("&sn=").append(m_service->GetAccount()->GetSerialNum());
          ElementPtr res(new Element("res", rval));
          data.uriMetadata->SetProperty(res);
          // tag <desc>
          ElementPtr desc(new Element("desc", m_service->GetServiceDesc()));
          desc->SetAttribut("id", "cdudn");
          desc->SetAttribut("nameSpace", DIDL_XMLNS_RINC);
          data.uriMetadata->SetProperty(desc);
          data.uriMetadata->SetObjectID(std::string("F00032020").append(data.item->GetObjectID()));
          data.uriMetadata->SetParentID(std::string("F000b2064").append(data.item->GetParentID()));
        }
        else
        {
          // clone the item as skeleton for uri metadata
          data.uriMetadata.reset(new DigitalItem(DigitalItem::Type_unknown, DigitalItem::SubType_unknown));
          data.item->Clone(*(data.uriMetadata));
          // tag <res>
          std::string rval(ProtocolTable[Protocol_xSonosHttp]);
          rval.append(":").append(data.item->GetObjectID()).append("?sid=").append(m_service->GetId());
          rval.append("&sn=").append(m_service->GetAccount()->GetSerialNum());
          ElementPtr res(new Element("res", rval));
          data.uriMetadata->SetProperty(res);
          // tag <desc>
          ElementPtr desc(new Element("desc", m_service->GetServiceDesc()));
          desc->SetAttribut("id", "cdudn");
          desc->SetAttribut("nameSpace", DIDL_XMLNS_RINC);
          data.uriMetadata->SetProperty(desc);
          data.uriMetadata->SetObjectID(std::string("00032020").append(data.item->GetObjectID()));
          data.uriMetadata->SetParentID(std::string("0004206c").append(data.item->GetParentID()));
        }
      }
      else if (itemType == "program")
      {
        // clone the item as skeleton for uri metadata
        data.uriMetadata.reset(new DigitalItem(DigitalItem::Type_unknown, DigitalItem::SubType_unknown));
        data.item->Clone(*(data.uriMetadata));
        // tag <res>
        std::string rval(ProtocolTable[Protocol_xSonosApiRadio]);
        rval.append(":").append(data.item->GetObjectID()).append("?sid=").append(m_service->GetId());
        rval.append("&sn=").append(m_service->GetAccount()->GetSerialNum());
        ElementPtr res(new Element("res", rval));
        data.uriMetadata->SetProperty(res);
        // tag <desc>
        ElementPtr desc(new Element("desc", m_service->GetServiceDesc()));
        desc->SetAttribut("id", "cdudn");
        desc->SetAttribut("nameSpace", DIDL_XMLNS_RINC);
        data.uriMetadata->SetProperty(desc);
        data.uriMetadata->SetObjectID(std::string("000c206c").append(data.item->GetObjectID()));
        data.uriMetadata->SetParentID("0");
      }
      else
        DBG(DBG_DEBUG, "%s: playable type (%s) isn't handled\n", __FUNCTION__, itemType.c_str());
    }
    list.push_back(data);
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
    else if (XMLNS::NameEqual(felem->Name(), "displayType") && felem->GetText())
      ptr->SetAttribut("displayType", felem->GetText());
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
    else if (XMLNS::NameEqual(felem->Name(), "displayType") && felem->GetText())
      ptr->SetAttribut("displayType", felem->GetText());
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
