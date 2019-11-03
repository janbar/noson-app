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

#ifndef AVTRANSPORT_H
#define	AVTRANSPORT_H

#include "local_config.h"
#include "service.h"
#include "eventhandler.h"
#include "subscription.h"
#include "locked.h"

#include <stdint.h>
#include <vector>

namespace NSROOT
{
  class Subscription;

  class AVTransport : public Service, public EventSubscriber
  {
  public:
    AVTransport(const std::string& serviceHost, unsigned servicePort);
    AVTransport(const std::string& serviceHost, unsigned servicePort, EventHandler& eventHandler, Subscription& subscription, void* CBHandle = 0, EventCB eventCB = 0);
    ~AVTransport();

    static const std::string Name;
    static const std::string ControlURL;
    static const std::string EventURL;
    static const std::string SCPDURL;

    const std::string& GetName() const { return Name; }

    const std::string& GetControlURL() const { return ControlURL; }

    const std::string& GetEventURL() const { return EventURL; }

    const std::string& GetSCPDURL() const { return SCPDURL; }

    bool GetTransportInfo(ElementList& vars);

    bool GetPositionInfo(ElementList& vars);

    bool GetMediaInfo(ElementList& vars);

    bool GetRemainingSleepTimerDuration(ElementList& vars);

    bool SetPlayMode(PlayMode_t mode);

    bool Play();

    bool Stop();

    bool Pause();

    bool SeekTime(uint16_t reltime);

    bool SeekTrack(unsigned tracknr);

    bool Next();

    bool Previous();

    bool SetCurrentURI(const std::string& uri, const std::string& metadata);

    bool SetNextURI(const std::string& uri, const std::string& metadata);

    unsigned AddURIToQueue(const std::string& uri, const std::string& metadata, unsigned position);

    // Max count of 16 URIs is allowed
    unsigned AddMultipleURIsToQueue(const std::vector<std::string>& uris, const std::vector<std::string>& metadatas);

    bool ReorderTracksInQueue(unsigned startIndex, unsigned numTracks, unsigned insBefore, unsigned containerUpdateID);

    bool RemoveTrackFromQueue(const std::string& objectID, unsigned containerUpdateID);

    bool RemoveTrackRangeFromQueue(unsigned startIndex, unsigned numTracks, unsigned containerUpdateID);

    bool RemoveAllTracksFromQueue();

    bool SaveQueue(const std::string& title);
    
    bool CreateSavedQueue(const std::string& title);

    unsigned AddURIToSavedQueue(const std::string& SQObjectID, const std::string& uri, const std::string& metadata, unsigned containerUpdateID);

    // to reorder: provide new positions for moved tracks, i.e: trackList = "1,6,9" newPositionList = "2,8,3"
    // to delete : provide range to delete in trackList, i.e: 2,5,7-8,10-15
    bool ReorderTracksInSavedQueue(const std::string& SQObjectID, const std::string& trackList, const std::string& newPositionList, unsigned containerUpdateID);

    bool ConfigureSleepTimer(unsigned seconds);

    bool SetAVTransportURI(const std::string& currentURI, const std::string& CurrentURIMetaData);

    // Remove this player from a group
    bool BecomeCoordinatorOfStandaloneGroup();

    // Implements EventSubscriber
    virtual void HandleEventMessage(EventMessagePtr msg);

    bool Empty() { return m_msgCount == 0; }

    Locked<AVTProperty>& GetAVTProperty() { return m_property; }

  private:
    EventHandler m_eventHandler;
    Subscription m_subscription;
    void* m_CBHandle;
    EventCB m_eventCB;
    unsigned m_msgCount;
    
    Locked<AVTProperty> m_property;
  };
}

#endif	/* AVTRANSPORT_H */

