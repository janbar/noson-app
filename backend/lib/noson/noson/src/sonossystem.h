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

#ifndef SONOSSYSTEM_H
#define	SONOSSYSTEM_H

#include <local_config.h>
#include "sonoszone.h"
#include "sonosplayer.h"
#include "eventhandler.h"

#include <string>
#include <map>

#define SONOS_LISTENER_PORT 1400

namespace NSROOT
{
  // List of ZonePlayer by name
  typedef std::map<std::string, ZonePlayerPtr> ZonePlayerList;
  // List of Zone by group
  typedef std::map<std::string, ZonePtr> ZoneList;

  namespace OS
  {
    class CMutex;
  }

  class System : private EventSubscriber
  {
  public:
     System();
    ~System();

    bool IsListening() { return m_eventHandler.IsRunning(); }

    bool Discover();

    ZoneList GetZoneList() const;

    ZonePlayerList GetZonePlayerList() const;

    bool ConnectZone(const ZonePtr& zone, void* CBHandle = 0, EventCB eventCB = 0);

    bool ConnectZone(const ZonePlayerPtr& zonePlayer, void* CBHandle = 0, EventCB eventCB = 0);

    bool IsConnected() const;

    const ZonePtr& GetConnectedZone() const { return m_connectedZone.zone; }
    
    const PlayerPtr& GetPlayer() const { return m_connectedZone.player; }

    // Implements EventSubscriber
    virtual void HandleEventMessage(EventMessagePtr msg);

  private:
    mutable OS::CMutex* m_mutex;
    EventHandler m_eventHandler;
    unsigned m_subId;

    ZonePlayerList m_zonePlayers;
    ZoneList m_zones;

    struct
    {
      ZonePtr zone;
      PlayerPtr player;
    } m_connectedZone;

    static bool FindDeviceDescription(std::string& url);

    bool GetTopology(const std::string& host, unsigned port);
  };
}


#endif	/* SONOSSYSTEM_H */

