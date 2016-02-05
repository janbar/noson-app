/*
 *      Copyright (C) 2015 Jean-Luc Barriere
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

#include "sonossystem.h"
#include "private/socket.h"
#include "private/wsresponse.h"

#include "private/os/threads/timeout.h"
#include "private/debug.h"
#include "private/uriparser.h"
#include "private/tinyxml2.h"
#include "private/os/threads/mutex.h"
#include "private/cppdef.h"

#include <cstdio> // for sscanf

#define PATH_TOPOLOGY "/status/topology"

using namespace SONOS;

System::System()
: m_mutex(new OS::CMutex)
, m_eventHandler(SONOS_LISTENER_PORT)
, m_subId(0)
{
  m_connectedZone.player.reset();
  m_connectedZone.zone.reset();

  m_subId = m_eventHandler.CreateSubscription(this);
  m_eventHandler.SubscribeForEvent(m_subId, EVENT_HANDLER_STATUS);
  m_eventHandler.SubscribeForEvent(m_subId, EVENT_UNKNOWN);
  if (!m_eventHandler.Start())
    DBG(DBG_ERROR, "%s: starting event handler failed\n", __FUNCTION__);
}

System::~System()
{
  m_mutex->Lock();
  SAFE_DELETE(m_mutex);
}

bool System::Discover()
{
  std::string url;
  if (!FindDeviceDescription(url))
    return false;
  URIParser uri(url);
  if (!uri.Scheme() || !uri.Host() || !uri.Port())
    return false;
  DBG(DBG_DEBUG, "%s: get topology from %s://%s:%u" PATH_TOPOLOGY "\n", __FUNCTION__, uri.Scheme(), uri.Host(), uri.Port());
  if (!GetTopology(uri.Host(), uri.Port()))
    return false;
  return true;
}

ZoneList System::GetZoneList() const
{
  OS::CLockGuard lock(*m_mutex);
  ZoneList list;
  for (ZoneList::const_iterator it = m_zones.begin(); it != m_zones.end(); ++it)
    if (it->second->GetCoordinator())
      list.insert(std::make_pair(it->first, it->second));
  return list;
}

ZonePlayerList System::GetZonePlayerList() const
{
  OS::CLockGuard lock(*m_mutex);
  return m_zonePlayers;
}

bool System::ConnectZone(const ZonePtr& zone, void* CBHandle, EventCB eventCB)
{
  OS::CLockGuard lock(*m_mutex);
  // Check listener
  if (!m_eventHandler.IsRunning() && !m_eventHandler.Start())
    return false;
  // Check zone
  if (!zone)
    return false;
  DBG(DBG_DEBUG, "%s: connect zone '%s'\n", __FUNCTION__, zone->GetZoneName().c_str());
  PlayerPtr player(new Player(zone, m_eventHandler, CBHandle, eventCB));
  if (player->IsValid())
  {
    m_connectedZone.player = player;
    m_connectedZone.zone = zone;
    return true;
  }
  return false;
}

bool System::ConnectZone(const ZonePlayerPtr& zonePlayer, void* CBHandle, EventCB eventCB)
{
  OS::CLockGuard lock(*m_mutex);
  // Check listener
  if (!m_eventHandler.IsRunning() && !m_eventHandler.Start())
    return false;
  // Check player
  if (!zonePlayer)
    return false;
  ZoneList::const_iterator zit = m_zones.find(zonePlayer->GetAttribut("group"));
  if (zit == m_zones.end())
    return false;
  return ConnectZone(zit->second, CBHandle, eventCB);
}

bool System::IsConnected() const
{
  OS::CLockGuard lock(*m_mutex);
  if (m_connectedZone.zone)
    return true;
  return false;
}

void System::HandleEventMessage(EventMessagePtr msg)
{
  if (msg->subject.size() > 0)
  {
    if (msg->subject[0] == "GET" && msg->subject[1] == "/stop")
      m_eventHandler.Stop();
  }
}

bool System::FindDeviceDescription(std::string& url)
{
#define MULTICAST_ADDR      "239.255.255.250"
#define MULTICAST_STRP      "1900"
#define MULTICAST_NUMP      1900
#define DISCOVER_TIMEOUT    5000
#define DISCOVER_ST         "urn:schemas-upnp-org:device:ZonePlayer:1"
#define HTTP_TOKEN_MAXSIZE  20

  static const char* msearch =
  "M-SEARCH * HTTP/1.1\r\n"
  "HOST: " MULTICAST_ADDR ":" MULTICAST_STRP "\r\n"
  "MAN: \"ssdp:discover\"\r\n"
  "MX: 1\r\n"
  "ST: " DISCOVER_ST "\r\n"
  "\r\n";

  static struct timeval socket_timeout = { 0, 500000 };

  bool ret = false;
  UdpSocket sock;
  sock.SetAddress(SOCKET_AF_INET4, MULTICAST_ADDR, MULTICAST_NUMP);
  sock.SetMulticastTTL(4);

  OS::CTimeout timeout(DISCOVER_TIMEOUT);
  while (!ret && timeout.TimeLeft() > 0)
  {
    sock.SendData(msearch, strlen(msearch));
    sock.SetTimeout(socket_timeout);
    std::string strread;
    size_t len = 0;
    unsigned _context = 0;
    while (!ret && WSResponse::ReadHeaderLine(&sock, "\r\n", strread, &len))
    {
      const char* line = strread.c_str();
      if (len == 15 && memcmp(line, "HTTP", 4) == 0)
      {
        int status = 0;
        if (1 == sscanf(line, "%*s %d", &status) && status == 200)
        {
          DBG(DBG_DEBUG, "%s: starting new context\n", __FUNCTION__);
          _context = 1; // new valid context
        }
        else
          _context = 0; // reset context
      }
      else if (_context)
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
            case 2:
              if (val && memcmp(token, "ST", token_len) == 0)
              {
                if (memcmp(val, DISCOVER_ST, value_len) == 0)
                {
                  DBG(DBG_DEBUG, "%s: search target matches (%s)\n", __FUNCTION__, val);
                  _context |= 0x2;
                }
                else
                {
                  DBG(DBG_DEBUG, "%s: search target not matches (%s)\n", __FUNCTION__, val);
                  _context = 0; // reset context
                }
              }
              break;
            case 6:
              if (val && memcmp(token, "SERVER", token_len) == 0)
              {
                DBG(DBG_DEBUG, "%s: server string found (%s)\n", __FUNCTION__, val);
                _context |= 0x4;
              }
              break;
            case 8:
              if (val && memcmp(token, "LOCATION", token_len) == 0)
              {
                DBG(DBG_DEBUG, "%s: location url found (%s)\n", __FUNCTION__, val);
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
          DBG(DBG_DEBUG, "%s: reseting context\n", __FUNCTION__);
          _context = 0; // reset context
        }
      }
      if (_context == 0xF)
        ret = true;
    }
  }
  return ret;
}

bool System::GetTopology(const std::string& host, unsigned port)
{
  WSRequest request(host, port);
  request.RequestService(PATH_TOPOLOGY, HRM_GET);
  WSResponse response(request);
  if (!response.IsSuccessful() || response.GetStatusCode() != 200)
    return false;
  tinyxml2::XMLDocument rootdoc;
  {
    std::string xml;
    xml.reserve(4096);
    size_t len;
    if (!(len = response.GetContentLength()))
      len = 16384;
    char buffer[256];
    memset(buffer, 0, sizeof(buffer));
    size_t l = 0;
    while ((l = response.ReadContent(buffer, (len > sizeof(buffer) - 1 ? sizeof(buffer) - 1 : len))))
    {
      xml.append(buffer, 0, l);
      len -= l;
    }
    // Parse xml content
    if (rootdoc.Parse(xml.c_str(), response.GetConsumed()) != tinyxml2::XML_SUCCESS)
    {
      DBG(DBG_ERROR, "%s: parse xml failed\n", __FUNCTION__);
      return false;
    }
  }
  tinyxml2::XMLElement* elem; // an element
  // Check for response: ZPSupportInfo
  if (!(elem = rootdoc.RootElement()) ||
          (strncmp(elem->Name(), "ZPSupportInfo", 13) != 0) ||
          !(elem = elem->FirstChildElement("ZonePlayers")))
  {
    DBG(DBG_ERROR, "%s: invalid or not supported response\n", __FUNCTION__);
    tinyxml2::XMLPrinter out;
    rootdoc.Accept(&out);
    DBG(DBG_ERROR, "%s\n", out.CStr());
    return false;
  }

  OS::CLockGuard lock(*m_mutex);
  m_zones.clear();
  m_zonePlayers.clear();
  elem = elem->FirstChildElement();
  while (elem)
  {
    DBG(DBG_DEBUG, "%s: new player '%s'\n", __FUNCTION__, elem->GetText());
    ZonePlayerPtr zp(new ZonePlayer(elem->GetText()));
    const tinyxml2::XMLAttribute* attr = elem->FirstAttribute();
    while (attr)
    {
      zp->SetAttribut(attr->Name(), attr->Value());
      attr = attr->Next();
    }
    m_zonePlayers.insert(std::make_pair(*zp, zp));
    const std::string& group = zp->GetAttribut("group");
    if (!group.empty())
    {
      ZoneList::iterator zit = m_zones.find(group);
      if (zit != m_zones.end())
        zit->second->push_back(zp);
      else
      {
        ZonePtr zone(new Zone(group));
        zone->push_back(zp);
        m_zones.insert(std::make_pair(group, zone));
      }
    }
    elem = elem->NextSiblingElement(NULL);
  }
  return true;
}
