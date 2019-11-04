/*
 *      Copyright (C) 2014-2018 Jean-Luc Barriere
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

#ifndef SONOSSYSTEM_H
#define	SONOSSYSTEM_H

#include "local_config.h"
#include "sonoszone.h"
#include "sonosplayer.h"
#include "eventhandler.h"
#include "subscription.h"
#include "alarmclock.h"

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
  class AlarmClock;

  class System : private EventSubscriber
  {
  public:
    System(void* CBHandle, EventCB eventCB);
    ~System();

    bool IsListening() { return m_eventHandler.IsRunning(); }

    bool Discover();
    bool Discover(const std::string& url);

    unsigned char LastEvents();

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

    // Alarm clock
    AlarmList GetAlarmList() const;

    bool CreateAlarm(Alarm& alarm);

    bool UpdateAlarm(Alarm& alarm);

    bool DestroyAlarm(const std::string& id);

    // Customized request broker
    void RegisterRequestBroker(RequestBrokerPtr rb);
    void UnregisterRequestBroker(const std::string& name);
    RequestBrokerPtr GetRequestBroker(const std::string& name);
    const std::string& GetSystemLocalUri() { return m_systemLocalUri; }

    // helpers
    static bool ExtractObjectFromFavorite(const DigitalItemPtr& favorite, DigitalItemPtr& item);
    static bool CanQueueItem(const DigitalItemPtr& item);
    static bool IsItemFromService(const DigitalItemPtr& item);

    /**
     * Request logo for a given music service and placement
     * @param service The music service
     * @param placement small|medium|large|x-large|square:x-small|square:small|square|square:x-large
     * @return uri string for the requested logo
     */
    static std::string GetLogoForService(const SMServicePtr& service, const std::string& placement);

    /**
     * Register auth data for a third part service
     * @param type The service type
     * @param sn The serial of account
     * @param key The key or password required to authenticate
     * @param token The current token for AppLink policy
     * @param username The user name for Login policy
     */
    static void AddServiceOAuth(const std::string& type, const std::string& sn, const std::string& key, const std::string& token, const std::string& username);

    /**
     * Remove auth data of a registered service
     * @param type The service type
     * @param sn The serial of account
     */
    static void DeleteServiceOAuth(const std::string& type, const std::string& sn);

    /**
     * Check the PulseAudio feature.
     * @return true if the feature is enabled
     */
#ifdef HAVE_PULSEAUDIO
    static bool HavePulseAudio() { return true; }
#else
    static bool HavePulseAudio() { return false; }
#endif

  private:
    mutable OS::CMutex* m_mutex;
    OS::CEvent* m_cbzgt;
    EventHandler m_eventHandler;
    std::string m_systemLocalUri;
    unsigned m_subId;
    Subscription m_ZGTSubscription;
    ZoneGroupTopology* m_groupTopology;
    Subscription m_AlarmClockSubscription;
    AlarmClock* m_alarmClock;
    void* m_CBHandle;
    EventCB m_eventCB;
    Locked<bool> m_eventSignaled;
    Locked<unsigned char> m_eventMask;

    struct
    {
      ZonePtr zone;
      PlayerPtr player;
    } m_connectedZone;

    static bool FindDeviceDescription(std::string& url);

    static void CB_ZGTopology(void* handle);
    static void CB_AlarmClock(void* handle);

    static bool LoadMSLogo(ElementList& logos);
  };
}


#endif	/* SONOSSYSTEM_H */

