/*
 *      Copyright (C) 2014-2019 Jean-Luc Barriere
 *
 *  This file is part of Noson
 *
 *  Noson is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Noson is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "sonossystem.h"
#include "zonegrouptopology.h"
#include "deviceproperties.h"
#include "contentdirectory.h"
#include "alarmclock.h"
#include "digitalitem.h"
#include "didlparser.h"
#include "smapimetadata.h"
#include "private/socket.h"
#include "private/wsresponse.h"
#include "private/os/threads/timeout.h"
#include "private/debug.h"
#include "private/builtin.h"
#include "private/uriparser.h"
#include "private/tokenizer.h"
#include "private/tinyxml2.h"
#include "private/os/threads/mutex.h"
#include "private/os/threads/event.h"
#include "private/cppdef.h"
#include "private/xmldict.h"

#include <cassert>
#include <set>

#define CB_TIMEOUT    3000
#define PATH_TOPOLOGY "/status/topology"
#define URI_MSLOGO    "http://update-services.sonos.com/services/mslogo.xml"

using namespace SONOS;

System::System(void* CBHandle, EventCB eventCB)
: m_mutex(new OS::CMutex)
, m_cbzgt(new OS::CEvent)
, m_connected(false)
, m_subId(0)
, m_eventHandler(SONOS_LISTENER_PORT)
, m_deviceHost()
, m_devicePort(0)
, m_CBHandle(CBHandle)
, m_eventCB(eventCB)
, m_eventSignaled(false)
, m_eventMask(0)
, m_groupTopology(nullptr)
, m_deviceProperties(nullptr)
, m_alarmClock(nullptr)
, m_contentDirectory(nullptr)
, m_musicServices(nullptr)
, m_players(PlayerMap())
, m_subscriptionPool()
{
  m_subId = m_eventHandler.CreateSubscription(this);
  m_eventHandler.SubscribeForEvent(m_subId, EVENT_HANDLER_STATUS);
  if (!m_eventHandler.Start())
    DBG(DBG_ERROR, "%s: starting event handler failed\n", __FUNCTION__);
  else
  {
    m_systemLocalUri.assign(ProtocolTable[Protocol_http])
        .append("://").append(m_eventHandler.GetAddress())
        .append(":").append(std::to_string(m_eventHandler.GetPort()));
  }
  m_subscriptionPool = SubscriptionPoolPtr(new SubscriptionPool(m_eventHandler));
}

System::~System()
{
  m_mutex->Lock();
  SAFE_DELETE(m_musicServices);
  SAFE_DELETE(m_contentDirectory);
  SAFE_DELETE(m_alarmClock);
  SAFE_DELETE(m_deviceProperties);
  SAFE_DELETE(m_groupTopology);
  SAFE_DELETE(m_cbzgt);
  m_eventHandler.RevokeAllSubscriptions(this);
  SAFE_DELETE(m_mutex);
}

void System::Debug(int level)
{
  SONOS::DBGLevel(level);
}

bool System::Discover()
{
  std::string url;
  if (!FindDeviceDescription(url))
    return false;
  return Discover(url);
}

bool System::Discover(const std::string& url)
{
  bool ret = false;
  URIParser uri(url);
  if (!uri.Scheme() || !uri.Host() || !uri.Port())
    return ret;

  OS::CLockGuard lock(*m_mutex);
  m_connected = false;
  m_deviceHost.assign(uri.Host());
  m_devicePort = uri.Port();

  // close all subscriptions
  SAFE_DELETE(m_musicServices);
  SAFE_DELETE(m_contentDirectory);
  SAFE_DELETE(m_alarmClock);
  SAFE_DELETE(m_deviceProperties);
  SAFE_DELETE(m_groupTopology);

  // subscribe to ZoneGroupTopology events
  m_groupTopology = new ZoneGroupTopology(uri.Host(), uri.Port(), m_subscriptionPool, this, CB_ZGTopology);

  // Wait event notification
  lock.Unlock();
  ret = m_cbzgt->Wait(CB_TIMEOUT);
  lock.Lock();

  if (!ret)
  {
    DBG(DBG_WARN, "%s: notification wasn't received after timeout: fall back on manual call\n", __FUNCTION__);
    if ((ret = m_groupTopology->GetZoneGroupState()))
      CB_ZGTopology(this); // ring the listener manually
    else
      return ret;
  }

  // DeviceProperties
  m_deviceProperties = new DeviceProperties(m_deviceHost, m_devicePort);
  ElementList vars;
  m_deviceProperties->GetHouseholdID(vars);
  m_householdID = vars.GetValue("CurrentHouseholdID");
  m_deviceProperties->GetZoneInfo(vars);
  m_serialNumber = vars.GetValue("SerialNumber");
  m_softwareVersion = vars.GetValue("SoftwareVersion");

  // music services
  m_musicServices = new MusicServices(uri.Host(), uri.Port());
  m_smservices = m_musicServices->GetAvailableServices();

  // subscribe to AlarmClock events
  m_alarmClock = new AlarmClock(uri.Host(), uri.Port(), m_subscriptionPool, this, CB_AlarmClock);

  // subscribe to ContentDirectory events
  m_contentDirectory = new ContentDirectory(uri.Host(), uri.Port(), m_subscriptionPool, this, CB_ContentDirectory);

  m_connected = ret;
  return ret;
}

unsigned char System::LastEvents()
{
  unsigned char mask;
  Locked<bool>::pointer _signaled = m_eventSignaled.Get();
  {
    Locked<unsigned char>::pointer _mask = m_eventMask.Get();
    mask = *_mask;
    *_mask = 0;
  }
  *_signaled = false;
  return mask;
}

void System::RenewSubscriptions()
{
  m_subscriptionPool->RenewSubscriptions();
}

ZoneList System::GetZoneList() const
{
  OS::CLockGuard lock(*m_mutex);
  ZoneList list;
  if (m_groupTopology)
  {
    Locked<ZoneList>::pointer zones = m_groupTopology->GetZoneList().Get();
    for (ZoneList::const_iterator it = zones->begin(); it != zones->end(); ++it)
      if (it->second->GetCoordinator())
        list.insert(std::make_pair(it->first, it->second));
  }
  return list;
}

ZonePlayerList System::GetZonePlayerList() const
{
  OS::CLockGuard lock(*m_mutex);
  if (m_groupTopology)
    return *(m_groupTopology->GetZonePlayerList().Get());
  return ZonePlayerList();
}

PlayerPtr System::GetPlayer(const ZonePtr& zone, void* CBHandle, EventCB eventCB)
{
  DBG(DBG_DEBUG, "%s: %s\n", __FUNCTION__, zone->GetZoneName().c_str());
  Locked<PlayerMap>::pointer players = m_players.Get();
  PlayerMap::iterator pit = players->find(zone->GetGroup());
  // The group ID is that of its coordinator. Therefore it is necessary to
  // compare the names to avoid confusion.
  if (pit != players->end() && pit->second->GetZone()->GetZoneName() == zone->GetZoneName())
    return pit->second;

  // Check listener
  if (!m_eventHandler.IsRunning() && !m_eventHandler.Start())
    return PlayerPtr();
  // Check requirements
  if (!zone)
    return PlayerPtr();
  DBG(DBG_DEBUG, "%s: connect zone [%u] '%s'\n", __FUNCTION__, players->size(), zone->GetZoneName().c_str());
  PlayerPtr player(new Player(zone, this, CBHandle, eventCB));
  if (player->IsValid())
  {
    players->insert(std::make_pair(zone->GetGroup(), player));
    return player;
  }
  return PlayerPtr();
}

PlayerPtr System::GetPlayer(const ZonePlayerPtr& zonePlayer, void* CBHandle, EventCB eventCB)
{
  DBG(DBG_DEBUG, "%s: %s\n", __FUNCTION__, zonePlayer->GetUUID().c_str());
  // Find the zone of the player
  ZonePtr zone;
  {
    OS::CLockGuard lock(*m_mutex);
    if (!m_groupTopology || !zonePlayer)
      return PlayerPtr();
    Locked<ZoneList>::pointer zones = m_groupTopology->GetZoneList().Get();
    ZoneList::iterator zit = zones->find(zonePlayer->GetAttribut("group"));
    if (zit != zones->end())
      zone = zit->second;
  }
  return GetPlayer(zone, CBHandle, eventCB);
}

bool System::IsConnected() const
{
  OS::CLockGuard lock(*m_mutex);
  return m_connected;
}

void System::HandleEventMessage(EventMessagePtr msg)
{
  if (!msg)
    return;
  if (msg->event == EVENT_HANDLER_STATUS)
  {
    // @TODO: handle status
    DBG(DBG_DEBUG, "%s: %s\n", __FUNCTION__, msg->subject[0].c_str());
  }
}

AlarmList System::GetAlarmList() const
{
  AlarmList alarms;
  if (m_alarmClock)
    m_alarmClock->ListAlarms(alarms);
  return alarms;
}

bool System::CreateAlarm(Alarm& alarm)
{
  if (!m_alarmClock)
    return false;
  return m_alarmClock->CreateAlarm(alarm);
}

bool System::UpdateAlarm(Alarm& alarm)
{
  if (!m_alarmClock)
    return false;
  return m_alarmClock->UpdateAlarm(alarm);
}

bool System::DestroyAlarm(const std::string& id)
{
  if (!m_alarmClock)
    return false;
  return m_alarmClock->DestroyAlarm(id);
}

ContentProperty System::GetContentProperty()
{
  return *(m_contentDirectory->GetContentProperty().Get());
}

bool System::RefreshShareIndex()
{
  return m_contentDirectory->RefreshShareIndex();
}

std::string System::GetObjectIDFromUriMetadata(const DigitalItemPtr& uriMetadata)
{
  if (!uriMetadata)
    return "";
  // the id should be here for many cases
  const std::string& itemId = uriMetadata->GetObjectID();

  if (itemId.compare(0, 2, "Q:") == 0 || itemId.compare(0, 3, "SQ:") == 0)
  {
    const std::string& uri = uriMetadata->GetValue("res");
    URIParser parser(uri);
    if (!parser.Scheme() || !parser.Path())
    {
      DBG(DBG_ERROR, "%s: invalid uri (%s)\n", __FUNCTION__, itemId.c_str());
      return "";
    }

    // check for library item
    if (strcmp(ProtocolTable[Protocol_xFileCifs], parser.Scheme()) == 0)
      return std::string("S://").append(parser.Host()).append("/").append(parser.Path());

    // check for service item
    SMServicePtr service = GetServiceForMedia(uri);
    if (service)
    {
      DigitalItemPtr meta;
      DigitalItemPtr fake(new DigitalItem(DigitalItem::Type_item, DigitalItem::SubType_audioItem));
      std::string path(parser.Path());
      path = path.substr(0, path.find('?')); // remove all from args
      fake->SetObjectID(path.substr(0, path.find_last_of('.'))); // remove mime extension
      SMAPIMetadata::MakeUriMetadata(service, SMAPIMetadata::track, fake, meta);
      return meta->GetObjectID();
    }
  }
  return itemId;
}

bool System::DestroySavedQueue(const std::string& SQObjectID)
{
  return m_contentDirectory->DestroyObject(SQObjectID);
}

bool System::AddURIToFavorites(const DigitalItemPtr& item, const std::string& description, const std::string& artURI)
{
  if (!item)
    return false;
  DigitalItemPtr favorite(new DigitalItem(DigitalItem::Type_item, DigitalItem::SubType_unknown));
  favorite->SetProperty(DIDL_QNAME_DC "title", item->GetValue(DIDL_QNAME_DC "title"));
  favorite->SetProperty(DIDL_QNAME_RINC "type", "instantPlay");
  favorite->SetProperty(item->GetProperty("res"));
  ElementPtr art = item->GetProperty(DIDL_QNAME_UPNP "albumArtURI");
  if (!art && !artURI.empty())
    art.reset(new Element(DIDL_QNAME_UPNP "albumArtURI", artURI));
  favorite->SetProperty(art);
  const std::string& album = item->GetValue(DIDL_QNAME_UPNP "album");
  const std::string& creator = item->GetValue(DIDL_QNAME_DC "creator");
  favorite->SetProperty(DIDL_QNAME_RINC "description", description.empty() ? album.empty() ? creator : album : description);
  // make r:resMD
  DigitalItem obj(DigitalItem::Type_item, DigitalItem::SubType_unknown);
  obj.SetObjectID(GetObjectIDFromUriMetadata(item)); // get a valid item id
  obj.SetParentID("");
  obj.SetRestricted(item->GetRestricted());
  obj.SetProperty(item->GetProperty(DIDL_QNAME_UPNP "class"));
  obj.SetProperty(item->GetProperty(DIDL_QNAME_DC "title"));
  if (!item->GetValue("desc").empty())
    obj.SetProperty(item->GetProperty("desc"));
  else
  {
    ElementPtr desc(new Element("desc"));
    SMServicePtr svc = GetServiceForMedia(item->GetValue("res"));
    if (svc)
      desc->assign(svc->GetServiceDesc());
    else
      desc->assign(ServiceDescTable[ServiceDesc_default]);
    desc->SetAttribut("id", "cdudn");
    desc->SetAttribut("nameSpace", DIDL_XMLNS_RINC);
    obj.SetProperty(desc);
  }
  favorite->SetProperty(DIDL_QNAME_RINC "resMD", obj.DIDL());
  ContentSearch search(SearchFavorite, "");
  return m_contentDirectory->CreateObject(search.Root(), favorite);
}

bool System::DestroyFavorite(const std::string& FVObjectID)
{
  return m_contentDirectory->DestroyObject(FVObjectID);
}

bool System::ExtractObjectFromFavorite(const DigitalItemPtr& favorite, DigitalItemPtr& item)
{
  const std::string& str = favorite->GetValue(DIDL_QNAME_RINC "resMD");
  if (str.empty())
    return false;
  DIDLParser didl(str.c_str());
  if (didl.IsValid() && !didl.GetItems().empty())
  {
    item.swap(didl.GetItems()[0]);
    item->SetProperty(favorite->GetProperty("res"));
    return true;
  }
  DigitalItemPtr ptr(new DigitalItem(DigitalItem::Type_item, DigitalItem::SubType_unknown));
  ptr->SetProperty(favorite->GetProperty("res"));
  ptr->SetProperty(favorite->GetProperty(DIDL_QNAME_DC "title"));
  ptr->SetProperty(favorite->GetProperty(DIDL_QNAME_UPNP "albumArtURI"));
  ptr->SetObjectID("-1");
  ptr->SetParentID("-1");
  item.swap(ptr);
  return true;
}

SMServiceList System::GetEnabledServices()
{
  SMServiceList list;
  for (SMServiceList::iterator it = m_smservices.begin(); it != m_smservices.end(); ++it)
  {
    const std::string& auth = (*it)->GetPolicy()->GetAttribut("Auth");
    if ((*it)->GetContainerType() != "MService")
      continue;
    if (auth == "Anonymous" || auth == "UserId" || auth == "DeviceLink" || auth == "AppLink") {
      // Service is enabled when an account is available for the service type.
      // Otherwise 'TuneIn' is special case as it is always enabled and no account exists for it.
      if ((*it)->GetServiceType() == "65031") /* TuneIn */
      {
        list.push_back((*it)->Clone("0"));
      }
      else
      {
        SMAccountList la = SMAccount::CreateAccounts((*it)->GetServiceType());
        for (SMAccountList::iterator ita = la.begin(); ita != la.end(); ++ita)
        {
          SMServicePtr sm = (*it)->Clone((*ita)->GetSerialNum());
          sm->GetAccount()->SetCredentials((*ita)->GetCredentials());
          list.push_back(sm);
        }
      }

    }
  }
  return list;
}

SMServiceList System::GetAvailableServices()
{
  SMServiceList list;
  for (SMServiceList::iterator it = m_smservices.begin(); it != m_smservices.end(); ++it)
  {
    const std::string& auth = (*it)->GetPolicy()->GetAttribut("Auth");
    if ((*it)->GetContainerType() != "MService")
      continue;
    if (auth == "Anonymous" || auth == "UserId" || auth == "DeviceLink" || auth == "AppLink")
      list.push_back(*it);
  }
  return list;
}

bool System::GetSessionId(const std::string& serviceId, const std::string& username, ElementList& vars)
{
  return m_musicServices->GetSessionId(serviceId, username, vars);
}

SMServicePtr System::GetServiceForMedia(const std::string& mediaUri)
{
  SMServicePtr svc;
  URIParser parser(mediaUri);
  if (!parser.Scheme() || !parser.Path())
  {
    DBG(DBG_ERROR, "%s: invalid uri (%s)\n", __FUNCTION__, mediaUri.c_str());
    return svc;
  }
  const char* p = strchr(parser.Path(), '?');
  if (p)
  {
    std::vector<std::string> args;
    tokenize(++p, "&", args, true);
    std::string sid;
    std::string sn;
    for (std::vector<std::string>::const_iterator ita = args.begin(); ita != args.end(); ++ita)
    {
      std::vector<std::string> tokens;
      tokenize(*ita, "=", tokens);
      if (tokens.size() != 2)
        break;
      if (tokens[0] == "sid")
        sid.assign(tokens[1]);
      else if (tokens[0] == "sn")
        sn.assign(tokens[1]);
    }
    if (!sid.empty())
    {
      if (sn.empty())
        sn.assign("0"); // trying fake account

      // loop in services: no longer check the serial since commit 7a91d3ade3a428d69fe6eb97a98fc6f670f16351
      for (SMServiceList::iterator its = m_smservices.begin(); its != m_smservices.end(); ++its)
        if ((*its)->GetId() == sid /*&& (*its)->GetAccount()->GetSerialNum() == sn*/)
          return *its;

      DBG(DBG_WARN, "%s: not found a valid service for this uri (%s)\n", __FUNCTION__, mediaUri.c_str());
    }
  }
  return svc;
}

bool System::CanQueueItem(const DigitalItemPtr& item)
{
  if (item)
  {
    // check parent
    const std::string& parent = item->GetParentID();
    if (    parent.compare(0, 2, "A:") == 0 ||
            parent.compare(0, 3, "SQ:") == 0)
      return true;

    // check protocol from tag <res>
    URIParser parser(item->GetValue("res"));
    if (parser.Scheme())
    {
      if (strcmp(ProtocolTable[Protocol_file], parser.Scheme()) == 0)
        return true;
      if (strcmp(ProtocolTable[Protocol_xFileCifs], parser.Scheme()) == 0)
        return true;
      if (strcmp(ProtocolTable[Protocol_xSonosHttp], parser.Scheme()) == 0)
        return true;
      if (strcmp(ProtocolTable[Protocol_xSonosSpotify], parser.Scheme()) == 0)
        return true;
      if (strcmp(ProtocolTable[Protocol_xSonosMms], parser.Scheme()) == 0)
        return true;
      if (strcmp(ProtocolTable[Protocol_xSonosApiRTRecent], parser.Scheme()) == 0)
        return true;
      if (strcmp(ProtocolTable[Protocol_xRinconCpcontainer], parser.Scheme()) == 0)
        return true;
    }
  }
  return false;
}

bool System::IsItemFromService(const DigitalItemPtr& item)
{
  if (!item)
    return false;
  const std::string& desc = item->GetValue("desc");
  if (desc.empty())
  {
    const std::string& res = item->GetValue("res");
    // any source with parameter 'sid'
    if (res.find("sid=") != std::string::npos)
      return true;
    // any source supplied by remote server
    if (res.compare(0, 4, "http") == 0)
      return true;
    return false;
  }
  if (desc != ServiceDescTable[ServiceDesc_default])
    return true;
  return false;
}

std::string System::GetLogoForService(const SMServicePtr& service, const std::string& placement)
{
  static Locked<unsigned> cc(0); ///< zero to refill cache
  static ElementList logos; ///< cached container for logos

  // hold count until return
  Locked<unsigned>::pointer ccPtr = cc.Get();

  // on first call we fill the container requesting sonos service
  if ((*ccPtr)++ == 0 && !LoadMSLogo(logos))
    DBG(DBG_ERROR, "%s: cache for service images cannot be filled\n", __FUNCTION__);

  const std::string& typeId = service->GetServiceType();
  for (ElementList::const_iterator it = logos.begin(); it != logos.end(); ++it)
    if ((*it)->GetKey() == typeId && (*it)->GetAttribut("placement") == placement)
      return (**it);
  return Element::Nil();
}

void System::AddServiceOAuth(const std::string& type, const std::string& sn, const std::string& key, const std::string& token, const std::string& username)
{
  SMOAKeyring::Store(type, sn, key, token, username);
}

void System::DeleteServiceOAuth(const std::string& type, const std::string& sn)
{
  SMOAKeyring::Purge(type, sn);
}

bool System::HavePulseAudio()
{
#ifdef HAVE_PULSEAUDIO
  return true;
#else
  return false;
#endif
}

void System::RegisterRequestBroker(RequestBrokerPtr rb)
{
  m_eventHandler.RegisterRequestBroker(rb);
}

void System::UnregisterRequestBroker(const std::string& name)
{
  m_eventHandler.UnregisterRequestBroker(name);
}

RequestBrokerPtr System::GetRequestBroker(const std::string &name)
{
  return m_eventHandler.GetRequestBroker(name);
}

bool System::FindDeviceDescription(std::string& url)
{
#define IPBC_ADDR           "255.255.255.255"
#define SSDP_ADDR           "239.255.255.250"
#define SSDP_STRP           "1900"
#define SSDP_NUMP           1900
#define DISCOVER_TIMEOUT    5000
#define DISCOVER_ST         "urn:schemas-upnp-org:device:ZonePlayer:1"
#define HTTP_TOKEN_MAXSIZE  20

  static const char* msearch =
  "M-SEARCH * HTTP/1.1\r\n"
  "HOST: " SSDP_ADDR ":" SSDP_STRP "\r\n"
  "MAN: \"ssdp:discover\"\r\n"
  "MX: 1\r\n"
  "ST: " DISCOVER_ST "\r\n"
  "\r\n";

  static struct timeval socket_timeout = { 0, 500000 };

  bool ret = false;
  std::list<std::pair<std::string, unsigned> > laddr;
  laddr.push_back(std::make_pair(IPBC_ADDR, SSDP_NUMP));
  laddr.push_back(std::make_pair(SSDP_ADDR, SSDP_NUMP));
  UdpSocket sock;
  sock.Open(SOCKET_AF_INET4, true);
  sock.SetMulticastTTL(4);

  OS::CTimeout timeout(DISCOVER_TIMEOUT);
  while (!ret && timeout.TimeLeft() > 0 && !laddr.empty())
  {
    std::pair<std::string, unsigned> addr = laddr.front();
    laddr.pop_front();
    if (sock.SetAddress(addr.first.c_str(), addr.second))
    {
      if (!sock.SendData(msearch, strlen(msearch)))
        DBG(DBG_ERROR, "%s: send data failed (%d)(%s:%d)\n", __FUNCTION__, sock.GetErrNo(), addr.first.c_str(), addr.second);
      laddr.push_back(addr);
    }
    sock.SetTimeout(socket_timeout);
    std::string strread;
    size_t len = 0;
    unsigned _context = 0;
    while (_context != 0xF && WSResponse::ReadHeaderLine(&sock, "\r\n", strread, &len))
    {
      const char* line = strread.c_str();
      if (_context == 0 && strstr(line, "HTTP/1."))
      {
        DBG(DBG_INFO, "%s: starting new context\n", __FUNCTION__);
        _context = 1; // new valid context
      }
      else if (_context > 0)
      {
        /*
         * Each header field consists of a name followed by a colon (":") and the
         * field value. Field names are case-insensitive. The field value MAY be
         * preceded by any amount of LWS, though a single SP is preferred.
         */
        const char* val = NULL;
        char token[21];
        int token_len = 0;
        int value_len = 0;
        if ((val = strchr(line, ':')))
        {
          int p;
          if ((token_len = val - line) > HTTP_TOKEN_MAXSIZE)
            token_len = HTTP_TOKEN_MAXSIZE;
          for (p = 0; p < token_len; ++p)
            token[p] = toupper(line[p]);
          token[token_len] = 0;
          while ((value_len = len - (val - line)) && *(++val) == ' ');
        }

        if (token_len)
        {
          switch (token_len)
          {
            case 6:
              if (val && memcmp(token, "SERVER", token_len) == 0)
              {
                DBG(DBG_INFO, "%s: server string found (%s)\n", __FUNCTION__, val);
                _context |= 0x2;
                if (strstr(val, "Sonos/"))
                {
                  DBG(DBG_INFO, "%s: search target matches\n", __FUNCTION__);
                  _context |= 0x4;
                }
                else
                {
                  DBG(DBG_INFO, "%s: search target not matches\n", __FUNCTION__);
                  _context = 0; // reset context
                }
              }
              break;
            case 8:
              if (val && memcmp(token, "LOCATION", token_len) == 0)
              {
                DBG(DBG_INFO, "%s: location url found (%s)\n", __FUNCTION__, val);
                _context |= 0x8;
                url.assign(val);
              }
              break;
            default:
              break;
          }
        }
        else if (len == 0)
        {
          // the message is ending by an empty line
          DBG(DBG_INFO, "%s: reseting context\n", __FUNCTION__);
          _context = 0; // reset context
        }
      }
    }
    ret = (_context == 0xF);
  }
  return ret;
}

void System::RevokePlayers()
{
  ZoneList zones = GetZoneList();
  Locked<PlayerMap>::pointer players = m_players.Get();
  std::list<PlayerMap::iterator> revoked;
  for (PlayerMap::iterator it = players->begin(); it != players->end(); ++it)
  {
    // The group ID is that of its coordinator. Therefore it is necessary to
    // compare the names to avoid confusion.
    ZoneList::iterator zit = zones.find(it->first);
    if (zit == zones.end() || zit->second->GetZoneName() != it->second->GetZone()->GetZoneName())
      revoked.push_back(it);
  }
  for (std::list<PlayerMap::iterator>::iterator it = revoked.begin(); it != revoked.end(); ++it)
  {
    DBG(DBG_INFO, "%s: %s\n", __FUNCTION__, (*it)->first.c_str());
    players->erase(*it);
  }
}

void System::CB_ZGTopology(void* handle)
{
  System* _handle = static_cast<System*>(handle);
  assert(_handle);

  _handle->RevokePlayers();

  {
    // BEGIN CRITICAL SECTION
    Locked<unsigned char>::pointer _mask = _handle->m_eventMask.Get();
    *_mask |= SVCEvent_ZGTopologyChanged;
    // END CRITICAL SECTION
  }
  _handle->m_cbzgt->Broadcast();
  if (_handle->m_eventCB && !_handle->m_eventSignaled.Load())
    _handle->m_eventCB(_handle->m_CBHandle);
}

void System::CB_AlarmClock(void* handle)
{
  System* _handle = static_cast<System*>(handle);
  assert(_handle);
  {
    // BEGIN CRITICAL SECTION
    Locked<unsigned char>::pointer _mask = _handle->m_eventMask.Get();
    *_mask |= SVCEvent_AlarmClockChanged;
    // END CRITICAL SECTION
  }
  if (_handle->m_eventCB && !_handle->m_eventSignaled.Load())
    _handle->m_eventCB(_handle->m_CBHandle);
}

void System::CB_ContentDirectory(void* handle)
{
  System* _handle = static_cast<System*>(handle);
  assert(_handle);
  {
    // BEGIN CRITICAL SECTION
    Locked<unsigned char>::pointer _mask = _handle->m_eventMask.Get();
    *_mask |= SVCEvent_ContentDirectoryChanged;
    // END CRITICAL SECTION
  }
  if (_handle->m_eventCB && !_handle->m_eventSignaled.Load())
    _handle->m_eventCB(_handle->m_CBHandle);
}

bool System::LoadMSLogo(ElementList& logos)
{
  WSRequest request(URIParser(URI_MSLOGO));
  WSResponse response(request);
  if (!response.IsSuccessful())
    return false;

  size_t len = 0, l = 0;
  std::string data;
  char buffer[4000];
  while ((l = response.ReadContent(buffer, sizeof(buffer))))
  {
    data.append(buffer, l);
    len += l;
  }

  tinyxml2::XMLDocument rootdoc;
  // Parse xml content
  if (rootdoc.Parse(data.c_str(), len) != tinyxml2::XML_SUCCESS)
  {
    DBG(DBG_ERROR, "%s: parse xml failed\n", __FUNCTION__);
    return false;
  }
  const tinyxml2::XMLElement* elem; // an element
  // Check for response: Services
  if (!(elem = rootdoc.RootElement()) || !XMLNS::NameEqual(elem->Name(), "images")
          || !(elem = elem->FirstChildElement("sized")))
  {
    DBG(DBG_ERROR, "%s: invalid or not supported content\n", __FUNCTION__);
    tinyxml2::XMLPrinter out;
    rootdoc.Accept(&out);
    DBG(DBG_ERROR, "%s\n", out.CStr());
    return false;
  }
  logos.clear();
  elem = elem->FirstChildElement("service");
  while (elem)
  {
    const tinyxml2::XMLElement* felem;
    const char* typeId = elem->Attribute("id");
    if (typeId)
    {
      felem = elem->FirstChildElement("image");
      while (felem)
      {
        const char* p = felem->Attribute("placement");
        if (p && felem->GetText())
        {
          ElementPtr logo(new Element(typeId, felem->GetText()));
          logo->SetAttribut("placement", p);
          logos.push_back(logo);
        }
        felem = felem->NextSiblingElement("image");
      }
    }
    elem = elem->NextSiblingElement("service");
  }
  return true;
}
