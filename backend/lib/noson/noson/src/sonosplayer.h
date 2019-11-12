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

#ifndef SONOSPLAYER_H
#define	SONOSPLAYER_H

#include "local_config.h"
#include "sharedptr.h"
#include "sonostypes.h"
#include "sonoszone.h"
#include "eventhandler.h"
#include "subscription.h"
#include "element.h"
#include "locked.h"
#include "musicservices.h"

#include <string>
#include <vector>
#include <map>

namespace NSROOT
{
  class AVTransport;
  class DeviceProperties;
  class RenderingControl;
  class ContentDirectory;
  class MusicServices;
  class Subscription;

  class Player;

  typedef SHARED_PTR<Player> PlayerPtr;
  typedef std::vector<SRProperty> SRPList;

  class Player : public EventSubscriber
  {
  public:

    /**
     * Initialize a zone player.
     * It will implement all needs to control the zone.
     * @param zone
     * @param eventHandler
     * @param CBHandle
     * @param eventCB
     */
    Player(const ZonePtr& zone, EventHandler& eventHandler, void* CBHandle = 0, EventCB eventCB = 0);

    /**
     * Initialize a basic player without any substription or callback.
     * WARNING: Properties won't be filled.
     * AV-transport or rendering status have to be manually requested using dedicated actions.
     * @param zonePlayer
     */
    Player(const ZonePlayerPtr& zonePlayer);

    virtual ~Player();

    bool IsValid() const { return m_valid; }
    const std::string& GetHost() const { return m_deviceHost; }
    unsigned GetPort() const { return m_devicePort; }
    const ZonePtr& GetZone() const { return m_zone; }

    void RevokeSubscription();
    void RenewSubscriptions();
    unsigned char LastEvents();
    bool RenderingPropertyEmpty();
    SRPList GetRenderingProperty();
    bool TransportPropertyEmpty();
    AVTProperty GetTransportProperty();

    bool GetZoneInfo(ElementList& vars);
    bool GetZoneAttributes(ElementList& vars);
    bool GetHouseholdID(ElementList& vars);
    bool GetTransportInfo(ElementList& vars);
    bool GetPositionInfo(ElementList &vars);
    bool GetMediaInfo(ElementList &vars);
    bool GetRemainingSleepTimerDuration(ElementList& vars);

    bool GetVolume(const std::string& uuid, uint8_t* value);
    bool SetVolume(const std::string& uuid, uint8_t value);
    bool GetMute(const std::string& uuid, uint8_t* value);
    bool SetMute(const std::string& uuid, uint8_t value);

    bool GetNightmode(const std::string& uuid, uint8_t* value);
    bool SetNightmode(const std::string& uuid, uint8_t value);
    bool GetLoudness(const std::string& uuid, uint8_t* value);
    bool SetLoudness(const std::string& uuid, uint8_t value);

    bool GetBass(const std::string& uuid, int8_t* value);
    bool SetBass(const std::string& uuid, int8_t value);

    bool GetTreble(const std::string& uuid, int8_t* value);
    bool SetTreble(const std::string& uuid, int8_t value);

    bool GetSupportsOutputFixed(const std::string& uuid, uint8_t* value);
    bool GetOutputFixed(const std::string& uuid, uint8_t* value);
    bool SetOutputFixed(const std::string& uuid, uint8_t value);

    bool SetCurrentURI(const DigitalItemPtr& item);
    bool PlayPulse();
    bool IsPulseStream(const std::string& streamURL);
    bool IsMyStream(const std::string& streamURL);
    bool PlayStream(const std::string& streamURL, const std::string& title, const std::string& iconURL);
    bool PlayStream(const std::string& streamURL, const std::string& title);
    bool PlayQueue(bool start);
    unsigned AddURIToQueue(const DigitalItemPtr& item, unsigned position);
    unsigned AddMultipleURIsToQueue(const std::vector<DigitalItemPtr>& items);
    bool RemoveAllTracksFromQueue();
    bool RemoveTrackFromQueue(const std::string& objectID, unsigned containerUpdateID);
    bool ReorderTracksInQueue(unsigned startIndex, unsigned numTracks, unsigned insBefore, unsigned containerUpdateID);

    bool SaveQueue(const std::string& title);
    bool CreateSavedQueue(const std::string& title);
    unsigned AddURIToSavedQueue(const std::string& SQObjectID, const DigitalItemPtr& item, unsigned containerUpdateID);
    bool ReorderTracksInSavedQueue(const std::string& SQObjectID, const std::string& trackList, const std::string& newPositionList, unsigned containerUpdateID);

    bool SetPlayMode(PlayMode_t mode);
    bool Play();
    bool Stop();
    bool Pause();
    bool SeekTime(uint16_t reltime);
    bool SeekTrack(unsigned tracknr);
    bool Next();
    bool Previous();

    bool ConfigureSleepTimer(unsigned seconds);

    bool BecomeStandalone();
    bool JoinToGroup(const std::string& coordinatorUUID);

    bool PlayLineIN();
    bool PlayDigitalIN();

    // Implements EventSubscriber
    virtual void HandleEventMessage(EventMessagePtr msg);

    // Helpers
    Protocol_t GetURIProtocol(const std::string& uri);
    const std::string& GetControllerUri() const { return m_controllerUri; }

    std::string MakeFilePictureUrl(const std::string& filePath);
    std::string MakeFilePictureLocalUrl(const std::string& filePath);
    DigitalItemPtr MakeFileStreamItem(const std::string& filePath, const std::string& codec, const std::string& title, const std::string& album,
                                      const std::string& author, const std::string& duration, bool hasArt);

  private:
    bool m_valid;
    ZonePtr m_zone;
    EventHandler m_eventHandler;
    std::string m_uuid;
    std::string m_deviceHost;
    unsigned m_devicePort;
    void* m_CBHandle;                       // callback handle
    EventCB m_eventCB;                      // callback on event
    Locked<bool> m_eventSignaled;           // cleared by calling LastEvents()
    Locked<unsigned char> m_eventMask;      // cleared by calling LastEvents()
    // Services API
    DeviceProperties*   m_deviceProperties;
    AVTransport*        m_AVTransport;

    // The name and address of this controller
    std::string m_controllerLocalUri;
    std::string m_controllerName;
    std::string m_controllerHost;
    std::string m_controllerUri;

    // Services subscriptions
    Subscription m_AVTSubscription;

    class SubordinateRC
    {
    public:
      std::string uuid;
      std::string name;
      Subscription subscription;
      RenderingControl* renderingControl;
      void FillSRProperty(SRProperty& srp) const;
    };

    typedef std::vector<SubordinateRC> RCTable;
    RCTable m_RCTable;

    // cold startup
    bool Init(const ZonePtr& zone);

    // event callback
    static void CB_AVTransport(void* handle);
    static void CB_RenderingControl(void* handle);

    // prevent copy
    Player(const Player&);
    Player& operator=(const Player&);
  };
}

#endif	/* SONOSPLAYER_H */

