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

#ifndef SONOSPLAYER_H
#define	SONOSPLAYER_H

#include <local_config.h>
#include "sharedptr.h"
#include "sonostypes.h"
#include "sonoszone.h"
#include "eventhandler.h"
#include "subscription.h"
#include "element.h"
#include "locked.h"

#include <string>
#include <vector>
#include <map>

namespace NSROOT
{
  class AVTransport;
  class DeviceProperties;
  class RenderingControl;
  class ContentDirectory;
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
     * Initialize a standalone player.
     * @param zonePlayer
     * @param eventHandler
     * @param CBHandle
     * @param eventCB
     */
    Player(const ZonePlayerPtr& zonePlayer, EventHandler& eventHandler, void* CBHandle = 0, EventCB eventCB = 0);

    /**
     * Initialize a basic player without any substription or callback.
     * WARNING: Properties won't be filled.
     * AV-transport or rendering status have to be manually requested using dedicated actions.
     * @param zonePlayer
     */
    Player(const ZonePlayerPtr& zonePlayer);

    virtual ~Player();

    bool IsValid() const { return m_valid; }
    const std::string& GetHost() const { return m_host; }
    unsigned GetPort() const { return m_port; }
    void RenewSubscriptions();
    unsigned char LastEvents();
    bool RenderingPropertyEmpty();
    SRPList GetRenderingProperty();
    bool TransportPropertyEmpty();
    AVTProperty GetTransportProperty();
    ContentProperty GetContentProperty();

    bool RefreshShareIndex();
    bool GetZoneInfo(ElementList& vars);
    bool GetZoneAttributes(ElementList& vars);
    bool GetTransportInfo(ElementList& vars);
    bool GetPositionInfo(ElementList &vars);
    bool GetMediaInfo(ElementList &vars);
    bool GetRemainingSleepTimerDuration(ElementList& vars);

    bool GetVolume(const std::string& uuid, uint8_t* value);
    bool SetVolume(const std::string& uuid, uint8_t value);
    bool GetMute(const std::string& uuid, uint8_t* value);
    bool SetMute(const std::string& uuid, uint8_t value);

    bool SetCurrentURI(const DigitalItemPtr& item);
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
    bool DestroySavedQueue(const std::string& SQObjectID);

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

    ContentDirectory* ContentDirectoryProvider(void* CBHandle = 0, EventCB eventCB = 0);

    // Implements EventSubscriber
    virtual void HandleEventMessage(EventMessagePtr msg);

  private:
    bool m_valid;
    std::string m_uuid;
    std::string m_host;
    unsigned m_port;
    EventHandler m_eventHandler;
    void* m_CBHandle;
    EventCB m_eventCB;
    Locked<bool> m_eventSignaled;
    Locked<unsigned char> m_eventMask;

    // special uri
    std::string m_queueURI;

    // services
    Subscription m_AVTSubscription;
    Subscription m_CDSubscription;

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

    AVTransport*        m_AVTransport;
    DeviceProperties*   m_deviceProperties;
    ContentDirectory*   m_contentDirectory;

    // cold startup
    void Init(const Zone& zone);

    // event callback
    static void CB_AVTransport(void* handle);
    static void CB_RenderingControl(void* handle);
    static void CB_ContentDirectory(void* handle);

    // prevent copy
    Player(const Player&);
    Player& operator=(const Player&);
  };
}

#endif	/* SONOSPLAYER_H */

