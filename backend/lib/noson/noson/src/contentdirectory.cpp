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

#include "contentdirectory.h"
#include "private/builtin.h"
#include "private/debug.h"
#include "eventhandler.h"
#include "didlparser.h"
#include "private/cppdef.h"

#include <list>

using namespace NSROOT;

const std::string ContentDirectory::Name("ContentDirectory");
const std::string ContentDirectory::ControlURL("/MediaServer/ContentDirectory/Control");
const std::string ContentDirectory::EventURL("/MediaServer/ContentDirectory/Event");
const std::string ContentDirectory::SCPDURL("/xml/ContentDirectory1.xml");

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

ContentDirectory::ContentDirectory(const std::string& serviceHost, unsigned servicePort)
: Service(serviceHost, servicePort)
, m_eventHandler()
, m_subscription()
, m_CBHandle(0)
, m_eventCB(0)
, m_property(ContentProperty())
{
}

ContentDirectory::ContentDirectory(const std::string& serviceHost, unsigned servicePort, EventHandler& eventHandler, Subscription& subscription, void* CBHandle, EventCB eventCB)
: Service(serviceHost, servicePort)
, m_eventHandler(eventHandler)
, m_subscription(subscription)
, m_CBHandle(CBHandle)
, m_eventCB(eventCB)
, m_property(ContentProperty())
{
  unsigned subId = m_eventHandler.CreateSubscription(this);
  m_eventHandler.SubscribeForEvent(subId, EVENT_UPNP_PROPCHANGE);
}

ContentDirectory::~ContentDirectory()
{
  m_eventHandler.RevokeAllSubscriptions(this);
}

bool ContentDirectory::Browse(const std::string& objectId, unsigned index, unsigned count, ElementList &vars)
{
  char buf[11];
  ElementList args;
  args.push_back(ElementPtr(new Element("ObjectID", objectId)));
  args.push_back(ElementPtr(new Element("BrowseFlag", "BrowseDirectChildren")));
  args.push_back(ElementPtr(new Element("Filter", "*")));
  uint32_to_string((uint32_t)index, buf);
  args.push_back(ElementPtr(new Element("StartingIndex", buf)));
  uint32_to_string((uint32_t)count, buf);
  args.push_back(ElementPtr(new Element("RequestedCount", buf)));
  args.push_back(ElementPtr(new Element("SortCriteria", "")));
  vars = Request("Browse", args);
  if (!vars.empty() && vars[0]->compare("u:BrowseResponse") == 0)
    return true;
  return false;
}

bool ContentDirectory::RefreshShareIndex()
{
  ElementList vars;
  ElementList args;
  args.push_back(ElementPtr(new Element("AlbumArtistDisplayOption", "")));
  vars = Request("RefreshShareIndex", args);
  if (!vars.empty() && vars[0]->compare("u:RefreshShareIndexResponse") == 0)
    return true;
  return false;
}

bool ContentDirectory::DestroyObject(const std::string& objectID)
{
  ElementList vars;
  ElementList args;
  args.push_back(ElementPtr(new Element("ObjectID", objectID)));
  vars = Request("DestroyObject", args);
  if (!vars.empty() && vars[0]->compare("u:DestroyObjectResponse") == 0)
    return true;
  return false;
}

bool ContentDirectory::CreateObject(const std::string& containerID, const DigitalItemPtr& element)
{
  ElementList vars;
  ElementList args;
  args.push_back(ElementPtr(new Element("ContainerID", containerID)));
  args.push_back(ElementPtr(new Element("Elements", element->DIDL())));
  vars = Request("CreateObject", args);
  if (!vars.empty() && vars[0]->compare("u:CreateObjectResponse") == 0)
    return true;
  return false;
}

void ContentDirectory::HandleEventMessage(EventMessagePtr msg)
{
  if (!msg)
    return;
  if (msg->event == EVENT_UPNP_PROPCHANGE)
  {
    if (m_subscription.GetSID() == msg->subject[0] && msg->subject[2] == "PROPERTY")
    {
      Locked<ContentProperty>::pointer prop = m_property.Get();

      DBG(DBG_DEBUG, "%s: %s SEQ=%s %s\n", __FUNCTION__, msg->subject[0].c_str(), msg->subject[1].c_str(), msg->subject[2].c_str());
      std::vector<std::string>::const_iterator it = msg->subject.begin();
      while (it != msg->subject.end())
      {
        if (*it == "SystemUpdateID")
          prop->SystemUpdateID.assign(*++it);
        else if (*it == "ContainerUpdateIDs")
        {
          prop->ContainerUpdateIDs.clear();
          std::vector<std::string> tokens;
          __tokenize((*++it).c_str(), ",", tokens);
          std::vector<std::string>::const_iterator itt = tokens.begin();
          while (itt != tokens.end())
          {
            const std::string& str = *itt;
            if (++itt != tokens.end())
            {
              uint32_t num;
              if (string_to_uint32(itt->c_str(), &num) == 0)
                prop->ContainerUpdateIDs.push_back(std::make_pair(str, num));
            }
          }
        }
        else if (*it == "UserRadioUpdateID")
          prop->UserRadioUpdateID.assign(*++it);
        else if (*it == "SavedQueuesUpdateID")
          prop->SavedQueuesUpdateID.assign(*++it);
        else if (*it == "ShareListUpdateID")
          prop->ShareListUpdateID.assign(*++it);
        else if (*it == "RecentlyPlayedUpdateID")
          prop->RecentlyPlayedUpdateID.assign(*++it);
        else if (*it == "RadioFavoritesUpdateID")
          prop->RadioFavoritesUpdateID.assign(*++it);
        else if (*it == "RadioLocationUpdateID")
          prop->RadioLocationUpdateID.assign(*++it);
        else if (*it == "FavoritesUpdateID")
          prop->FavoritesUpdateID.assign(*++it);
        else if (*it == "FavoritePresetsUpdateID")
          prop->FavoritePresetsUpdateID.assign(*++it);

        ++it;
      }
      // Signal
      if (m_eventCB)
        m_eventCB(m_CBHandle);
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
////
//// ContentSearch
////

std::string ContentSearch::Root() const
{
  std::string objectId;
  switch (m_search)
  {
  case SearchArtist:
    objectId.assign("A:ALBUMARTIST");
    break;
  case SearchAlbum:
    objectId.assign("A:ALBUM");
    break;
  case SearchGenre:
    objectId.assign("A:GENRE");
    break;
  case SearchTrack:
    objectId.assign("A:TRACKS");
    break;
  case SearchComposer:
    objectId.assign("A:COMPOSER");
    break;
  case SearchContributor:
    objectId.assign("A:ARTIST");
    break;
  case SearchPlaylist:
    objectId.assign("A:PLAYLISTS");
    break;
  case SearchRadio:
    objectId.assign("R:0");
    break;
  case SearchQueue:
    objectId.assign("Q:0");
    break;
  case SearchSonosPlaylist:
    objectId.assign("SQ:");
    break;
  case SearchShare:
    objectId.assign("S:");
    break;
  case SearchFavorite:
    objectId.assign("FV:2");
    break;
  case SearchCategory:
  default:
    objectId.assign("A:");
  }
  if (!m_string.empty())
    objectId.append(":").append(m_string);
  return objectId;
}

///////////////////////////////////////////////////////////////////////////////
////
//// ContentList
////

ContentList::ContentList(ContentDirectory& service, const ContentSearch& search, unsigned bulksize)
: m_succeeded(false)
, m_service(service)
, m_bulkSize(BROWSE_COUNT)
, m_root(search.Root())
, m_baseUpdateID(0)
, m_totalCount(0)
, m_browsedCount(0)
, m_lastUpdateID(0)
{
  if (bulksize > 0 && bulksize < BROWSE_COUNT)
    m_bulkSize = bulksize;
  BrowseContent(0, m_bulkSize, m_list.begin());
  m_baseUpdateID = m_lastUpdateID; // save baseline ID for this content
}

ContentList::ContentList(ContentDirectory& service, const std::string& objectID, unsigned bulksize)
: m_succeeded(false)
, m_service(service)
, m_bulkSize(BROWSE_COUNT)
, m_root(objectID)
, m_baseUpdateID(0)
, m_totalCount(0)
, m_browsedCount(0)
, m_lastUpdateID(0)
{
  if (bulksize > 0 && bulksize < BROWSE_COUNT)
    m_bulkSize = bulksize;
  BrowseContent(0, m_bulkSize, m_list.begin());
  m_baseUpdateID = m_lastUpdateID; // save baseline ID for this content
}

bool ContentList::Next(List::iterator& i)
{
  const List::iterator e(m_list.end());
  if (i != e)
  {
    bool r = true;
    List::iterator n = i;
    if (++n == e)
      r = BrowseContent(m_browsedCount, m_bulkSize, n);
    ++i; // On failure i becomes end
    return r;
  }
  return false;
}

bool ContentList::Previous(List::iterator& i)
{
  if (i != m_list.begin())
  {
    --i;
    return true;
  }
  return false;
}

bool ContentList::BrowseContent(unsigned startingIndex, unsigned count, List::iterator position)
{
  DBG(DBG_PROTO, "%s: browse %u from %u\n", __FUNCTION__, count, startingIndex);
  ElementList vars;
  ElementList::const_iterator it;
  if ((m_succeeded = m_service.Browse(m_root, startingIndex, count, vars)) && (it = vars.FindKey("Result")) != vars.end())
  {
    uint32_t updateID = 0;
    if (string_to_uint32(vars.GetValue("UpdateID").c_str(), &updateID) == 0)
      m_lastUpdateID = updateID; // set update ID for this chunk of data
    uint32_t totalcount = 0;
    if (string_to_uint32(vars.GetValue("TotalMatches").c_str(), &totalcount) == 0)
      m_totalCount = totalcount; // reset total count
    uint32_t count = 0;
    string_to_uint32(vars.GetValue("NumberReturned").c_str(), &count);
    DIDLParser didl((*it)->c_str(), count);
    if (didl.IsValid())
    {
      m_list.insert(position, didl.GetItems().begin(), didl.GetItems().end());
      m_browsedCount += didl.GetItems().size();
      DBG(DBG_PROTO, "%s: count %u\n", __FUNCTION__, didl.GetItems().size());
      return true;
    }
  }
  return false;
}

///////////////////////////////////////////////////////////////////////////////
////
//// ContentBrowser
////

ContentBrowser::ContentBrowser(ContentDirectory& service, const ContentSearch& search, unsigned count)
: m_service(service)
, m_root(search.Root())
, m_baseUpdateID(0)
, m_totalCount(0)
, m_startingIndex(0)
, m_lastUpdateID(0)
{
  BrowseContent(m_startingIndex, count, m_table.begin());
  m_baseUpdateID = m_lastUpdateID; // save baseline ID for this content
}

ContentBrowser::ContentBrowser(ContentDirectory& service, const std::string& objectID, unsigned count)
: m_service(service)
, m_root(objectID)
, m_baseUpdateID(0)
, m_totalCount(0)
, m_startingIndex(0)
, m_lastUpdateID(0)
{
  BrowseContent(m_startingIndex, count, m_table.begin());
  m_baseUpdateID = m_lastUpdateID; // save baseline ID for this content
}

bool ContentBrowser::Browse(unsigned index, unsigned count)
{
  if (index >= m_totalCount)
  {
    m_table.clear();
    m_startingIndex = m_totalCount;
    return false;
  }

  unsigned size = (unsigned) m_table.size();
  if (m_totalCount < index + count)
    count = m_totalCount - index;

  if (index == m_startingIndex)
  {
    if (count == size)
      return true;
    if (count < size)
    {
      m_table.resize(count);
    }
    return BrowseContent(m_startingIndex + size, count - size, m_table.end());
  }

  if (index > m_startingIndex && index + count <= m_startingIndex + size)
  {
    Table tmp;
    tmp.insert(tmp.begin(), m_table.begin() + index - m_startingIndex, m_table.begin() + index - m_startingIndex + count);
    m_table = tmp;
    m_startingIndex = index;
    return true;
  }
  
  m_table.clear();
  m_startingIndex = index;
  return BrowseContent(m_startingIndex, count, m_table.begin());
}

bool ContentBrowser::BrowseContent(unsigned startingIndex, unsigned count, Table::iterator position)
{
  DBG(DBG_PROTO, "%s: browse %u from %u\n", __FUNCTION__, count, startingIndex);
  ElementList vars;
  ElementList::const_iterator it;
  if (m_service.Browse(m_root, startingIndex, count, vars) && (it = vars.FindKey("Result")) != vars.end())
  {
    uint32_t updateID = 0;
    if (string_to_uint32(vars.GetValue("UpdateID").c_str(), &updateID) == 0)
      m_lastUpdateID = updateID; // set update ID for this chunk of data
    uint32_t totalcount = 0;
    if (string_to_uint32(vars.GetValue("TotalMatches").c_str(), &totalcount) == 0)
      m_totalCount = totalcount; // reset total count
    uint32_t count = 0;
    string_to_uint32(vars.GetValue("NumberReturned").c_str(), &count);
    DIDLParser didl((*it)->c_str(), count);
    if (didl.IsValid())
    {
      m_table.insert(position, didl.GetItems().begin(), didl.GetItems().end());
      DBG(DBG_PROTO, "%s: count %u\n", __FUNCTION__, didl.GetItems().size());
      return true;
    }
  }
  return false;
}
