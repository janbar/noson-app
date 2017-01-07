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
#include "subscription.h"

#include <string>

#define SONOS_LISTENER_PORT 1400

namespace NSROOT
{
  namespace OS
  {
    class CMutex;
    class CEvent;
  }

  class ZoneGroupTopology;

  class System : private EventSubscriber
  {
  public:
    System(void* CBHandle, EventCB eventCB);
    ~System();

    bool IsListening() { return m_eventHandler.IsRunning(); }

    bool Discover();

    void RenewSubscriptions();

    ZoneList GetZoneList() const;

    ZonePlayerList GetZonePlayerList() const;

    bool ConnectZone(const ZonePtr& zone, void* CBHandle = 0, EventCB eventCB = 0);

    bool ConnectZone(const ZonePlayerPtr& zonePlayer, void* CBHandle = 0, EventCB eventCB = 0);

    bool IsConnected() const;

    const ZonePtr& GetConnectedZone() const { return m_connectedZone.zone; }
    
    const PlayerPtr& GetPlayer() const { return m_connectedZone.player; }

    // Implements EventSubscriber
    virtual void HandleEventMessage(EventMessagePtr msg);

    // helpers
    static bool ExtractObjectFromFavorite(const DigitalItemPtr& favorite, DigitalItemPtr& item);
    static bool CanQueueItem(const DigitalItemPtr& item);
    static bool IsItemFromService(const DigitalItemPtr& item);
    static std::string MakeItemIdFromMediaUri(const std::string& mediaUri);

    /**
     * Request logo for a given music service and placement
     * @param service The music service
     * @param placement small|medium|large|x-large|square:x-small|square:small|square|square:x-large
     * @return uri string for the requested logo
     */
    static std::string GetLogoForService(const SMServicePtr& service, const std::string& placement);

  private:
    mutable OS::CMutex* m_mutex;
    OS::CEvent* m_cbzgt;
    EventHandler m_eventHandler;
    unsigned m_subId;
    Subscription m_ZGTSubscription;
    ZoneGroupTopology* m_groupTopology;
    void* m_CBHandle;
    EventCB m_eventCB;

    struct
    {
      ZonePtr zone;
      PlayerPtr player;
    } m_connectedZone;

    static bool FindDeviceDescription(std::string& url);

    static void CBZGTopology(void* handle);

    static bool LoadMSLogo(ElementList& logos);
  };
}


#endif	/* SONOSSYSTEM_H */

