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

#include "zonegrouptopology.h"
#include "private/builtin.h"
#include "private/debug.h"
#include "private/tinyxml2.h"

using namespace NSROOT;

const std::string ZoneGroupTopology::Name("ZoneGroupTopology");
const std::string ZoneGroupTopology::ControlURL("/ZoneGroupTopology/Control");
const std::string ZoneGroupTopology::EventURL("/ZoneGroupTopology/Event");
const std::string ZoneGroupTopology::SCPDURL("/xml/ZoneGroupTopology1.xml");

ZoneGroupTopology::ZoneGroupTopology(const std::string& serviceHost, unsigned servicePort)
: Service(serviceHost, servicePort)
, m_eventHandler()
, m_subscription()
, m_CBHandle(0)
, m_eventCB(0)
, m_msgCount(0)
, m_zones(ZoneList())
, m_zonePlayers(ZonePlayerList())
{
}

ZoneGroupTopology::ZoneGroupTopology(const std::string& serviceHost, unsigned servicePort, EventHandler& eventHandler, Subscription& subscription, void* CBHandle, EventCB eventCB)
: Service(serviceHost, servicePort)
, m_eventHandler(eventHandler)
, m_subscription(subscription)
, m_CBHandle(CBHandle)
, m_eventCB(eventCB)
, m_msgCount(0)
, m_zones(ZoneList())
, m_zonePlayers(ZonePlayerList())
{
  unsigned subId = m_eventHandler.CreateSubscription(this);
  m_eventHandler.SubscribeForEvent(subId, EVENT_UPNP_PROPCHANGE);
}

ZoneGroupTopology::~ZoneGroupTopology()
{
  m_eventHandler.RevokeAllSubscriptions(this);
}

bool ZoneGroupTopology::GetZoneGroupState()
{
  ElementList args;
  ElementList vars = Request("GetZoneGroupState", args);
  if (!vars.empty() && vars[0]->compare("u:GetZoneGroupStateResponse") == 0)
    return ParseZoneGroupState(vars.GetValue("ZoneGroupState"));
  return false;
}

void ZoneGroupTopology::HandleEventMessage(EventMessagePtr msg)
{
  if (!msg)
    return;
  if (msg->event == EVENT_UPNP_PROPCHANGE)
  {
    if (m_subscription.GetSID() == msg->subject[0] && msg->subject[2] == "PROPERTY")
    {
      DBG(DBG_DEBUG, "%s: %s SEQ=%s %s\n", __FUNCTION__, msg->subject[0].c_str(), msg->subject[1].c_str(), msg->subject[2].c_str());
      std::vector<std::string>::const_iterator it = msg->subject.begin();
      while (it != msg->subject.end())
      {
        if (*it == "ZoneGroupState")
        {
          ParseZoneGroupState(*++it);
          break;
        }
        ++it;
      }
      // Signal
      ++m_msgCount;
      if (m_eventCB)
        m_eventCB(m_CBHandle);
    }
  }
}

bool ZoneGroupTopology::ParseZoneGroupState(const std::string& xml)
{
  tinyxml2::XMLDocument rootdoc;
  // Parse xml content
  if (rootdoc.Parse(xml.c_str(), xml.size()) != tinyxml2::XML_SUCCESS)
  {
    DBG(DBG_ERROR, "%s: parse xml failed\n", __FUNCTION__);
    return false;
  }
  tinyxml2::XMLElement* elem; // an element
  // Check for response: ZPSupportInfo
  if (!(elem = rootdoc.RootElement()) || strncmp(elem->Name(), "ZoneGroups", 10) != 0)
  {
    DBG(DBG_ERROR, "%s: invalid or not supported content\n", __FUNCTION__);
    tinyxml2::XMLPrinter out;
    rootdoc.Accept(&out);
    DBG(DBG_ERROR, "%s\n", out.CStr());
    return false;
  }

  Locked<ZoneList>::pointer zones = m_zones.Get();
  Locked<ZonePlayerList>::pointer zonePlayers = m_zonePlayers.Get();
  zones->clear();
  zonePlayers->clear();
  elem = elem->FirstChildElement();
  while (elem)
  {
    const tinyxml2::XMLAttribute* attr = elem->FirstAttribute();
    Element zoneGroup(elem->Name());
    while (attr)
    {
      zoneGroup.SetAttribut(attr->Name(), attr->Value());
      attr = attr->Next();
    }
    ZonePtr zone(new Zone(zoneGroup.GetAttribut("ID")));
    const std::string& cuuid = zoneGroup.GetAttribut("Coordinator");
    DBG(DBG_INFO, "%s: new group '%s' with coordinator '%s'\n", __FUNCTION__, zone->GetGroup().c_str(), cuuid.c_str());
    // browse childs
    tinyxml2::XMLElement* child = elem->FirstChildElement();
    while (child)
    {
      if (strncmp(child->Name(), "ZoneGroupMember", 15) == 0)
      {
        const tinyxml2::XMLAttribute* cattr = child->FirstAttribute();
        Element zoneGroupMember(child->Name());
        while (cattr)
        {
          zoneGroupMember.SetAttribut(cattr->Name(), cattr->Value());
          cattr = cattr->Next();
        }
        ZonePlayerPtr zp(new ZonePlayer(zoneGroupMember.GetAttribut("ZoneName")));
        const std::string& muuid = zoneGroupMember.GetAttribut("UUID");
        zp->SetAttribut(ZP_UUID, muuid);
        if (muuid == cuuid)
          zp->SetAttribut(ZP_COORDINATOR, "true");
        else
          zp->SetAttribut(ZP_COORDINATOR, "false");
        zp->SetAttribut(ZP_LOCATION, zoneGroupMember.GetAttribut("Location"));
        zp->SetAttribut(ZP_ICON, zoneGroupMember.GetAttribut("Icon"));
        zp->SetAttribut(ZP_VERSION, zoneGroupMember.GetAttribut("SoftwareVersion"));
        zp->SetAttribut(ZP_MCVERSION, zoneGroupMember.GetAttribut("MinCompatibleVersion"));
        zp->SetAttribut(ZP_LCVERSION, zoneGroupMember.GetAttribut("LegacyCompatibleVersion"));
        DBG(DBG_INFO, "%s: new group member '%s' (%s)\n", __FUNCTION__, muuid.c_str(), zp->c_str());
        zonePlayers->insert(std::make_pair(*zp, zp));
        zone->push_back(zp);
      }
      child = child->NextSiblingElement(NULL);
    }
    zone->Revamp();
    zones->insert(std::make_pair(zone->GetGroup(), zone));
    elem = elem->NextSiblingElement(NULL);
  }
  return true;
}
