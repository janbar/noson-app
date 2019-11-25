/*
 *      Copyright (C) 2014-2016 Jean-Luc Barriere
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

#include "zonegrouptopology.h"
#include "private/builtin.h"
#include "private/debug.h"
#include "private/tinyxml2.h"
#include "private/xmldict.h"

using namespace NSROOT;

const std::string ZoneGroupTopology::Name("ZoneGroupTopology");
const std::string ZoneGroupTopology::ControlURL("/ZoneGroupTopology/Control");
const std::string ZoneGroupTopology::EventURL("/ZoneGroupTopology/Event");
const std::string ZoneGroupTopology::SCPDURL("/xml/ZoneGroupTopology1.xml");

inline uint_fast32_t __hashvalue(uint_fast32_t maxsize, const char *value)
{
  uint_fast32_t h = 0, g;

  while (*value)
  {
    h = (h << 4) + *value++;
    if ((g = h & 0xF0000000L))
    {
      h ^= g >> 24;
    }
    h &= ~g;
  }

  return h % maxsize;
}

ZoneGroupTopology::ZoneGroupTopology(const std::string& serviceHost, unsigned servicePort)
: Service(serviceHost, servicePort)
, m_subscriptionPool()
, m_subscription()
, m_CBHandle(nullptr)
, m_eventCB(nullptr)
, m_msgCount(0)
, m_topologyKey(0)
, m_zones(ZoneList())
, m_zonePlayers(ZonePlayerList())
{
}

ZoneGroupTopology::ZoneGroupTopology(const std::string& serviceHost, unsigned servicePort, SubscriptionPoolPtr& subscriptionPool, void* CBHandle, EventCB eventCB)
: Service(serviceHost, servicePort)
, m_subscriptionPool(subscriptionPool)
, m_subscription()
, m_CBHandle(CBHandle)
, m_eventCB(eventCB)
, m_msgCount(0)
, m_topologyKey(0)
, m_zones(ZoneList())
, m_zonePlayers(ZonePlayerList())
{
  unsigned subId = m_subscriptionPool->GetEventHandler().CreateSubscription(this);
  m_subscriptionPool->GetEventHandler().SubscribeForEvent(subId, EVENT_UPNP_PROPCHANGE);
  m_subscription = m_subscriptionPool->SubscribeEvent(serviceHost, servicePort, EventURL);
  m_subscription.Start();
}

ZoneGroupTopology::~ZoneGroupTopology()
{
  if (m_subscriptionPool)
  {
    m_subscriptionPool->UnsubscribeEvent(m_subscription);
    m_subscriptionPool->GetEventHandler().RevokeAllSubscriptions(this);
  }
}

bool ZoneGroupTopology::GetZoneGroupState()
{
  ElementList args;
  ElementList vars = Request("GetZoneGroupState", args);
  if (!vars.empty() && vars[0]->compare("GetZoneGroupStateResponse") == 0)
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
      unsigned _key = m_topologyKey;
      while (it != msg->subject.end())
      {
        if (*it == "ZoneGroupState")
        {
          // BEGIN CRITICAL SECTION
          ParseZoneGroupState(*++it);
          // END CRITICAL SECTION
          break;
        }
        ++it;
      }
      // Event is signaled only on first or any change
      if (m_msgCount && _key == m_topologyKey)
        return;
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

  const tinyxml2::XMLElement* elem; // an element
  if ((elem = rootdoc.RootElement()))
  {
    // since API version 10.2 (build 50163230): the root element is 'ZoneGroupState'
    if (XMLNS::NameEqual(elem->Name(), "ZoneGroupState"))
      elem = elem->FirstChildElement();
  }
  else
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

  while (elem)
  {
    // VanishedDevices: Added in API version 10.2 (build 50163230)
    // ZoneGroups
    if (XMLNS::NameEqual(elem->Name(), "ZoneGroups"))
    {
      const tinyxml2::XMLElement* gelem = elem->FirstChildElement();
      while (gelem)
      {
        const tinyxml2::XMLAttribute* attr = gelem->FirstAttribute();
        Element zoneGroup(gelem->Name());
        while (attr)
        {
          zoneGroup.SetAttribut(attr->Name(), attr->Value());
          attr = attr->Next();
        }
        ZonePtr zone(new Zone(zoneGroup.GetAttribut("ID")));
        const std::string& cuuid = zoneGroup.GetAttribut("Coordinator");
        DBG(DBG_INFO, "%s: new group '%s' with coordinator '%s'\n", __FUNCTION__, zone->GetGroup().c_str(), cuuid.c_str());
        // browse childs
        const tinyxml2::XMLElement* child = gelem->FirstChildElement();
        while (child)
        {
          if (XMLNS::NameEqual(child->Name(), "ZoneGroupMember"))
          {
            const tinyxml2::XMLAttribute* cattr = child->FirstAttribute();
            Element zoneGroupMember(child->Name());
            while (cattr)
            {
              zoneGroupMember.SetAttribut(cattr->Name(), cattr->Value());
              cattr = cattr->Next();
            }
            if (zoneGroupMember.GetAttribut("Invisible") == "1")
            {
              const std::string& mname = zoneGroupMember.GetAttribut("ZoneName");
              const std::string& muuid = zoneGroupMember.GetAttribut("UUID");
              DBG(DBG_INFO, "%s: discard invisible group member '%s' (%s)\n", __FUNCTION__, muuid.c_str(), mname.c_str());
            }
            else
            {
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
          }
          child = child->NextSiblingElement(NULL);
        }
        if (!zone->empty())
        {
          zone->Revamp();
          zones->insert(std::make_pair(zone->GetGroup(), zone));
        }
        gelem = gelem->NextSiblingElement(NULL);
      }
    }
    elem = elem->NextSiblingElement(NULL);
  }
  // compute a key for this state
  std::string keyStr;
  keyStr.reserve(zones->size() << 5);
  for (ZoneList::const_iterator it = zones->begin(); it != zones->end(); ++it)
    keyStr.append(it->first);
  m_topologyKey = __hashvalue(0xFFFFFFFF, keyStr.c_str());
  DBG(DBG_INFO, "%s: topology key %u\n", __FUNCTION__, m_topologyKey);
  return (!zones->empty());
}
