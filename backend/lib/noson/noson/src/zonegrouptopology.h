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

#ifndef ZONEGROUPTOPOLOGY_H
#define ZONEGROUPTOPOLOGY_H

#include "local_config.h"
#include "service.h"
#include "eventhandler.h"
#include "subscription.h"
#include "sonoszone.h"
#include "locked.h"

namespace NSROOT
{

  class ZoneGroupTopology : public Service, public EventSubscriber
  {
  public:
    ZoneGroupTopology(const std::string& serviceHost, unsigned servicePort);
    ZoneGroupTopology(const std::string& serviceHost, unsigned servicePort, EventHandler& eventHandler, Subscription& subscription, void* CBHandle = 0, EventCB eventCB = 0);
    ~ZoneGroupTopology();

    static const std::string Name;
    static const std::string ControlURL;
    static const std::string EventURL;
    static const std::string SCPDURL;

    const std::string& GetName() const { return Name; }

    const std::string& GetControlURL() const { return ControlURL; }

    const std::string& GetEventURL() const { return EventURL; }

    const std::string& GetSCPDURL() const { return SCPDURL; }

    bool GetZoneGroupState();

    unsigned GetTopologyKey() const { return m_topologyKey; }

    Locked<ZoneList>& GetZoneList() { return m_zones; }

    Locked<ZonePlayerList>& GetZonePlayerList() { return m_zonePlayers; }

    // Implements EventSubscriber
    virtual void HandleEventMessage(EventMessagePtr msg);

  private:
    EventHandler m_eventHandler;
    Subscription m_subscription;
    void* m_CBHandle;
    EventCB m_eventCB;
    unsigned m_msgCount;
    unsigned m_topologyKey;

    Locked<ZoneList> m_zones;
    Locked<ZonePlayerList> m_zonePlayers;

    bool ParseZoneGroupState(const std::string& xml);

  };
}

#endif /* ZONEGROUPTOPOLOGY_H */

