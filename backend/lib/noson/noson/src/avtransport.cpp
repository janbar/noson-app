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

#include "avtransport.h"
#include "private/builtin.h"
#include "private/cppdef.h"
#include "private/debug.h"
#include "private/os/threads/timeout.h"
#include "didlparser.h"

using namespace NSROOT;

const std::string AVTransport::Name("AVTransport");
const std::string AVTransport::ControlURL("/MediaRenderer/AVTransport/Control");
const std::string AVTransport::EventURL("/MediaRenderer/AVTransport/Event");
const std::string AVTransport::SCPDURL("/xml/AVTransport1.xml");

namespace NSROOT
{

  struct AVTransportLastInfo
  {
    AVTransportLastInfo() : expiry(), vars() { }
    OS::CTimeout expiry;
    ElementList vars;
  };
  
}

AVTransport::AVTransport(const std::string& serviceHost, unsigned servicePort)
: Service(serviceHost, servicePort)
, m_subscriptionPool()
, m_subscription()
, m_CBHandle(nullptr)
, m_eventCB(nullptr)
, m_msgCount(0)
, m_property(AVTProperty())
, m_lastPositionInfo(new AVTransportLastInfo())
{
}

AVTransport::AVTransport(const std::string& serviceHost, unsigned servicePort, SubscriptionPoolPtr& subscriptionPool, void* CBHandle, EventCB eventCB)
: Service(serviceHost, servicePort)
, m_subscriptionPool(subscriptionPool)
, m_subscription()
, m_CBHandle(CBHandle)
, m_eventCB(eventCB)
, m_msgCount(0)
, m_property(AVTProperty())
, m_lastPositionInfo(new AVTransportLastInfo())
{
  unsigned subId = m_subscriptionPool->GetEventHandler().CreateSubscription(this);
  m_subscriptionPool->GetEventHandler().SubscribeForEvent(subId, EVENT_UPNP_PROPCHANGE);
  m_subscription = m_subscriptionPool->SubscribeEvent(serviceHost, servicePort, EventURL);
  m_subscription.Start();
}

AVTransport::~AVTransport()
{
  if (m_subscriptionPool)
  {
    m_subscriptionPool->UnsubscribeEvent(m_subscription);
    m_subscriptionPool->GetEventHandler().RevokeAllSubscriptions(this);
  }
  delete m_lastPositionInfo.Load();
}

bool AVTransport::GetTransportInfo(ElementList& vars)
{
  ElementList args;
  args.push_back(ElementPtr(new Element("InstanceID", "0")));
  vars = Request("GetTransportInfo", args);
  if (!vars.empty() && vars[0]->compare("GetTransportInfoResponse") == 0)
    return true;
  return false;
}

bool AVTransport::GetPositionInfo(ElementList& vars)
{
  Locked<AVTransportLastInfo*>::pointer p = m_lastPositionInfo.Get();
  if ((*p)->expiry.TimeLeft() > 0)
  {
    vars = (*p)->vars;
    return true;
  }
  ElementList args;
  args.push_back(ElementPtr(new Element("InstanceID", "0")));
  vars = Request("GetPositionInfo", args);
  if (!vars.empty() && vars[0]->compare("GetPositionInfoResponse") == 0)
  {
    (*p)->vars = vars;
    (*p)->expiry.Set(1000); // expire in 1 second
    return true;
  }
  return false;
}

bool AVTransport::GetMediaInfo(ElementList& vars)
{
  ElementList args;
  args.push_back(ElementPtr(new Element("InstanceID", "0")));
  vars = Request("GetMediaInfo", args);
  if (!vars.empty() && vars[0]->compare("GetMediaInfoResponse") == 0)
    return true;
  return false;
}

bool AVTransport::GetRemainingSleepTimerDuration(ElementList& vars)
{
  ElementList args;
  args.push_back(ElementPtr(new Element("InstanceID", "0")));
  vars = Request("GetRemainingSleepTimerDuration", args);
  if (!vars.empty() && vars[0]->compare("GetRemainingSleepTimerDurationResponse") == 0)
    return true;
  return false;
}

bool AVTransport::SetPlayMode(PlayMode_t mode)
{
  const char* modeStr;
  switch (mode)
  {
    case PlayMode_NORMAL:
      modeStr = PlayModeTable[PlayMode_NORMAL];
      break;
    case PlayMode_REPEAT_ALL:
      modeStr = PlayModeTable[PlayMode_REPEAT_ALL];
      break;
    case PlayMode_SHUFFLE:
      modeStr = PlayModeTable[PlayMode_SHUFFLE];
      break;
    case PlayMode_SHUFFLE_NOREPEAT:
      modeStr = PlayModeTable[PlayMode_SHUFFLE_NOREPEAT];
      break;
    case PlayMode_REPEAT_ONE:
      modeStr = PlayModeTable[PlayMode_REPEAT_ONE];
      break;
    default:
      modeStr = "";
  }
  ElementList args;
  args.push_back(ElementPtr(new Element("InstanceID", "0")));
  args.push_back(ElementPtr(new Element("NewPlayMode", modeStr)));
  ElementList vars = Request("SetPlayMode", args);
  if (!vars.empty() && vars[0]->compare("SetPlayModeResponse") == 0)
    return true;
  return false;
}

bool AVTransport::Play()
{
  ElementList args;
  args.push_back(ElementPtr(new Element("InstanceID", "0")));
  args.push_back(ElementPtr(new Element("Speed", "1")));
  ElementList vars = Request("Play", args);
  if (!vars.empty() && vars[0]->compare("PlayResponse") == 0)
    return true;
  return false;
}

bool AVTransport::Stop()
{
  ElementList args;
  args.push_back(ElementPtr(new Element("InstanceID", "0")));
  args.push_back(ElementPtr(new Element("Speed", "1")));
  ElementList vars = Request("Stop", args);
  if (!vars.empty() && vars[0]->compare("StopResponse") == 0)
    return true;
  return false;
}

bool AVTransport::Pause()
{
  ElementList args;
  args.push_back(ElementPtr(new Element("InstanceID", "0")));
  args.push_back(ElementPtr(new Element("Speed", "1")));
  ElementList vars = Request("Pause", args);
  if (!vars.empty() && vars[0]->compare("PauseResponse") == 0)
    return true;
  return false;
}

bool AVTransport::SeekTime(uint16_t reltime)
{
  char buf[9];
  memset(buf, 0, sizeof (buf));
  sprintf(buf, "%.2u:%.2u:%.2u", (unsigned)(reltime / 3600),
          (unsigned)((reltime % 3600) / 60), (unsigned)(reltime % 60));
  ElementList args;
  args.push_back(ElementPtr(new Element("InstanceID", "0")));
  args.push_back(ElementPtr(new Element("Unit", "REL_TIME")));
  args.push_back(ElementPtr(new Element("Target", buf)));
  ElementList vars = Request("Seek", args);
  if (!vars.empty() && vars[0]->compare("SeekResponse") == 0)
    return true;
  return false;
}

bool AVTransport::SeekTrack(unsigned tracknr)
{
  ElementList args;
  args.push_back(ElementPtr(new Element("InstanceID", "0")));
  args.push_back(ElementPtr(new Element("Unit", "TRACK_NR")));
  args.push_back(ElementPtr(new Element("Target", std::to_string(tracknr))));
  ElementList vars = Request("Seek", args);
  if (!vars.empty() && vars[0]->compare("SeekResponse") == 0)
    return true;
  return false;
}

bool AVTransport::Next()
{
  ElementList args;
  args.push_back(ElementPtr(new Element("InstanceID", "0")));
  args.push_back(ElementPtr(new Element("Speed", "1")));
  ElementList vars = Request("Next", args);
  if (!vars.empty() && vars[0]->compare("NextResponse") == 0)
    return true;
  return false;
}

bool AVTransport::Previous()
{
  ElementList args;
  args.push_back(ElementPtr(new Element("InstanceID", "0")));
  args.push_back(ElementPtr(new Element("Speed", "1")));
  ElementList vars = Request("Previous", args);
  if (!vars.empty() && vars[0]->compare("PreviousResponse") == 0)
    return true;
  return false;
}

bool AVTransport::SetCurrentURI(const std::string& uri, const std::string& metadata)
{
  ElementList args;
  args.push_back(ElementPtr(new Element("InstanceID", "0")));
  args.push_back(ElementPtr(new Element("CurrentURI", uri)));
  args.push_back(ElementPtr(new Element("CurrentURIMetaData", metadata)));
  ElementList vars = Request("SetAVTransportURI", args);
  if (!vars.empty() && vars[0]->compare("SetAVTransportURIResponse") == 0)
    return true;
  return false;
}

bool AVTransport::SetNextURI(const std::string& uri, const std::string& metadata)
{
  ElementList args;
  args.push_back(ElementPtr(new Element("InstanceID", "0")));
  args.push_back(ElementPtr(new Element("NextURI", uri)));
  args.push_back(ElementPtr(new Element("NextURIMetaData", metadata)));
  ElementList vars = Request("SetNextAVTransportURI", args);
  if (!vars.empty() && vars[0]->compare("SetNextAVTransportURIResponse") == 0)
    return true;
  return false;
}

unsigned AVTransport::AddURIToQueue(const std::string& uri, const std::string& metadata, unsigned position)
{
  ElementList args;
  args.push_back(ElementPtr(new Element("InstanceID", "0")));
  args.push_back(ElementPtr(new Element("EnqueuedURI", uri)));
  args.push_back(ElementPtr(new Element("EnqueuedURIMetaData", metadata)));
  args.push_back(ElementPtr(new Element("DesiredFirstTrackNumberEnqueued", std::to_string(position))));
  args.push_back(ElementPtr(new Element("EnqueueAsNext", "1")));
  ElementList vars = Request("AddURIToQueue", args);
  if (!vars.empty() && vars[0]->compare("AddURIToQueueResponse") == 0)
  {
    uint32_t num;
    string_to_uint32(vars.GetValue("FirstTrackNumberEnqueued").c_str(), &num);
    return num;
  }
  return 0;
}

unsigned AVTransport::AddMultipleURIsToQueue(const std::vector<std::string>& uris, const std::vector<std::string>& metadatas)
{
  ElementList args;
  Element* elem;
  args.push_back(ElementPtr(new Element("InstanceID", "0")));
  args.push_back(ElementPtr(new Element("UpdateID", "0")));
  args.push_back(ElementPtr(new Element("NumberOfURIs", std::to_string(uris.size()))));
  elem = new Element("EnqueuedURIs", "");
  for (std::vector<std::string>::const_iterator it = uris.begin(); it != uris.end(); ++it)
  {
    if (it != uris.begin())
      elem->append(" ");
    elem->append(*it);
  }
  args.push_back(ElementPtr(elem));
  elem = new Element("EnqueuedURIsMetaData", "");
  for (std::vector<std::string>::const_iterator it = metadatas.begin(); it != metadatas.end(); ++it)
  {
    if (it != metadatas.begin())
      elem->append(" ");
    elem->append(*it);
  }
  args.push_back(ElementPtr(elem));
  args.push_back(ElementPtr(new Element("ContainerURI", "")));
  args.push_back(ElementPtr(new Element("ContainerMetadata", "")));
  args.push_back(ElementPtr(new Element("DesiredFirstTrackNumberEnqueued", "0")));
  args.push_back(ElementPtr(new Element("EnqueueAsNext", "0")));
  ElementList vars = Request("AddMultipleURIsToQueue", args);
  if (!vars.empty() && vars[0]->compare("AddMultipleURIsToQueueResponse") == 0)
  {
    uint32_t num;
    string_to_uint32(vars.GetValue("FirstTrackNumberEnqueued").c_str(), &num);
    return num;
  }
  return 0;
}

bool AVTransport::ReorderTracksInQueue(unsigned startIndex, unsigned numTracks, unsigned insBefore, unsigned containerUpdateID)
{
  ElementList args;
  args.push_back(ElementPtr(new Element("InstanceID", "0")));
  args.push_back(ElementPtr(new Element("StartingIndex", std::to_string(startIndex))));
  args.push_back(ElementPtr(new Element("NumberOfTracks", std::to_string(numTracks))));
  args.push_back(ElementPtr(new Element("InsertBefore", std::to_string(insBefore))));
  args.push_back(ElementPtr(new Element("UpdateID", std::to_string(containerUpdateID))));
  ElementList vars = Request("ReorderTracksInQueue", args);
  if (!vars.empty() && vars[0]->compare("ReorderTracksInQueueResponse") == 0)
    return true;
  return false;
}

bool AVTransport::RemoveTrackFromQueue(const std::string& objectID, unsigned containerUpdateID)
{
  ElementList args;
  args.push_back(ElementPtr(new Element("InstanceID", "0")));
  args.push_back(ElementPtr(new Element("ObjectID", objectID)));
  args.push_back(ElementPtr(new Element("UpdateID", std::to_string(containerUpdateID))));
  ElementList vars = Request("RemoveTrackFromQueue", args);
  if (!vars.empty() && vars[0]->compare("RemoveTrackFromQueueResponse") == 0)
    return true;
  return false;
}

bool AVTransport::RemoveTrackRangeFromQueue(unsigned startIndex, unsigned numTracks, unsigned containerUpdateID)
{
  ElementList args;
  args.push_back(ElementPtr(new Element("InstanceID", "0")));
  args.push_back(ElementPtr(new Element("StartingIndex", std::to_string(startIndex))));
  args.push_back(ElementPtr(new Element("NumberOfTracks", std::to_string(numTracks))));
  args.push_back(ElementPtr(new Element("UpdateID", std::to_string(containerUpdateID))));
  ElementList vars = Request("RemoveTrackRangeFromQueue", args);
  if (!vars.empty() && vars[0]->compare("RemoveTrackRangeFromQueueResponse") == 0)
  {
    const std::string& val = vars.GetValue("NewUpdateID");
    if (val.empty())
      return false;
    return true;
  }
  return false;
}

bool AVTransport::RemoveAllTracksFromQueue()
{
  ElementList args;
  args.push_back(ElementPtr(new Element("InstanceID", "0")));
  ElementList vars = Request("RemoveAllTracksFromQueue", args);
  if (!vars.empty() && vars[0]->compare("RemoveAllTracksFromQueueResponse") == 0)
    return true;
  return false;
}

bool AVTransport::SaveQueue(const std::string& title)
{
  ElementList args;
  args.push_back(ElementPtr(new Element("InstanceID", "0")));
  args.push_back(ElementPtr(new Element("Title", title)));
  args.push_back(ElementPtr(new Element("ObjectID", "")));
  ElementList vars = Request("SaveQueue", args);
  if (!vars.empty() && vars[0]->compare("SaveQueueResponse") == 0)
    return true;
  return false;
}

bool AVTransport::CreateSavedQueue(const std::string& title)
{
  ElementList args;
  args.push_back(ElementPtr(new Element("InstanceID", "0")));
  args.push_back(ElementPtr(new Element("Title", title)));
  args.push_back(ElementPtr(new Element("EnqueuedURI", "")));
  args.push_back(ElementPtr(new Element("EnqueuedURIMetaData", "")));
  ElementList vars = Request("CreateSavedQueue", args);
  if (!vars.empty() && vars[0]->compare("CreateSavedQueueResponse") == 0)
    return true;
  return false;
}

unsigned AVTransport::AddURIToSavedQueue(const std::string& SQObjectID, const std::string& uri, const std::string& metadata, unsigned containerUpdateID)
{
  ElementList args;
  args.push_back(ElementPtr(new Element("InstanceID", "0")));
  args.push_back(ElementPtr(new Element("ObjectID", SQObjectID)));
  args.push_back(ElementPtr(new Element("EnqueuedURI", uri)));
  args.push_back(ElementPtr(new Element("EnqueuedURIMetaData", metadata)));
  args.push_back(ElementPtr(new Element("AddAtIndex", "4294967295"))); // An other value will failed
  args.push_back(ElementPtr(new Element("UpdateID", std::to_string(containerUpdateID))));
  ElementList vars = Request("AddURIToSavedQueue", args);
  if (!vars.empty() && vars[0]->compare("AddURIToSavedQueueResponse") == 0)
  {
    uint32_t num;
    string_to_uint32(vars.GetValue("NewUpdateID").c_str(), &num);
    return num;
  }
  return 0;
}

bool AVTransport::ReorderTracksInSavedQueue(const std::string& SQObjectID, const std::string& trackList, const std::string& newPositionList, unsigned containerUpdateID)
{
  ElementList args;
  args.push_back(ElementPtr(new Element("InstanceID", "0")));
  args.push_back(ElementPtr(new Element("ObjectID", SQObjectID)));
  args.push_back(ElementPtr(new Element("TrackList", trackList)));
  args.push_back(ElementPtr(new Element("NewPositionList", newPositionList)));
  args.push_back(ElementPtr(new Element("UpdateID", std::to_string(containerUpdateID))));
  ElementList vars = Request("ReorderTracksInSavedQueue", args);
  if (!vars.empty() && vars[0]->compare("ReorderTracksInSavedQueueResponse") == 0)
    return true;
  return false;
}

bool AVTransport::ConfigureSleepTimer(unsigned seconds)
{
  if (seconds >= 24 * 3600)
    return false;
  char buf[9];
  memset(buf, 0, sizeof (buf));
  if (seconds)
    sprintf(buf, "%.2u:%.2u:%.2u", (unsigned)(seconds / 3600),
            (unsigned)((seconds % 3600) / 60), (unsigned)(seconds % 60));
  ElementList args;
  args.push_back(ElementPtr(new Element("InstanceID", "0")));
  args.push_back(ElementPtr(new Element("NewSleepTimerDuration", buf)));
  ElementList vars = Request("ConfigureSleepTimer", args);
  if (!vars.empty() && vars[0]->compare("ConfigureSleepTimerResponse") == 0)
    return true;
  return false;
}

bool AVTransport::SetAVTransportURI(const std::string& currentURI, const std::string& CurrentURIMetaData)
{
  ElementList args;
  args.push_back(ElementPtr(new Element("InstanceID", "0")));
  args.push_back(ElementPtr(new Element("CurrentURI", currentURI)));
  args.push_back(ElementPtr(new Element("CurrentURIMetaData", CurrentURIMetaData)));
  ElementList vars = Request("SetAVTransportURI", args);
  if (!vars.empty() && vars[0]->compare("SetAVTransportURIResponse") == 0)
    return true;
  return false;
}

bool AVTransport::BecomeCoordinatorOfStandaloneGroup()
{
  ElementList args;
  args.push_back(ElementPtr(new Element("InstanceID", "0")));
  ElementList vars = Request("BecomeCoordinatorOfStandaloneGroup", args);
  if (!vars.empty() && vars[0]->compare("BecomeCoordinatorOfStandaloneGroupResponse") == 0)
    return true;
  return false;
}

void AVTransport::HandleEventMessage(EventMessagePtr msg)
{
  if (!msg)
    return;
  if (msg->event == EVENT_UPNP_PROPCHANGE)
  {
    if (m_subscription.GetSID() == msg->subject[0] && msg->subject[2] == "AVT")
    {
      {
        // BEGIN CRITICAL SECTION
        Locked<AVTProperty>::pointer prop = m_property.Get();

        DBG(DBG_DEBUG, "%s: %s SEQ=%s %s\n", __FUNCTION__, msg->subject[0].c_str(), msg->subject[1].c_str(), msg->subject[2].c_str());
        std::vector<std::string>::const_iterator it = msg->subject.begin();
        while (it != msg->subject.end())
        {
          uint32_t num;
          if (*it == "TransportState")
            prop->TransportState.assign(*++it);
          else if (*it == "CurrentPlayMode")
            prop->CurrentPlayMode.assign(*++it);
          else if (*it == "CurrentCrossfadeMode")
            prop->CurrentCrossfadeMode.assign(*++it);
          else if (*it == "NumberOfTracks")
          {
            string_to_uint32((*++it).c_str(), &num);
            prop->NumberOfTracks = (unsigned)num;
          }
          else if (*it == "CurrentTrack")
          {
            string_to_uint32((*++it).c_str(), &num);
            prop->CurrentTrack = (unsigned)num;
          }
          else if (*it == "CurrentSection")
          {
            string_to_uint32((*++it).c_str(), &num);
            prop->CurrentSection = (unsigned)num;
          }
          else if (*it == "CurrentTrackURI")
            prop->CurrentTrackURI.assign(*++it);
          else if (*it == "CurrentTrackDuration")
            prop->CurrentTrackDuration.assign(*++it);
          else if (*it == "CurrentTrackMetaData")
          {
            DIDLParser didl((*++it).c_str());
            if (didl.IsValid() && !didl.GetItems().empty())
              prop->CurrentTrackMetaData = didl.GetItems()[0];
            else
              prop->CurrentTrackMetaData.reset(new DigitalItem(DigitalItem::Type_unknown));
          }
          else if (*it == "r:NextTrackURI")
            prop->r_NextTrackURI.assign(*++it);
          else if (*it == "r:NextTrackMetaData")
          {
            DIDLParser didl((*++it).c_str());
            if (didl.IsValid() && !didl.GetItems().empty())
              prop->r_NextTrackMetaData = didl.GetItems()[0];
            else
              prop->r_NextTrackMetaData.reset(new DigitalItem(DigitalItem::Type_unknown));
          }
          else if (*it == "r:EnqueuedTransportURI")
            prop->r_EnqueuedTransportURI.assign(*++it);
          else if (*it == "r:EnqueuedTransportURIMetaData")
          {
            DIDLParser didl((*++it).c_str());
            if (didl.IsValid() && !didl.GetItems().empty())
              prop->r_EnqueuedTransportURIMetaData = didl.GetItems()[0];
            else
              prop->r_EnqueuedTransportURIMetaData.reset(new DigitalItem(DigitalItem::Type_unknown));
          }
          else if (*it == "PlaybackStorageMedium")
            prop->PlaybackStorageMedium.assign(*++it);
          else if (*it == "AVTransportURI")
            prop->AVTransportURI.assign(*++it);
          else if (*it == "AVTransportURIMetaData")
          {
            DIDLParser didl((*++it).c_str());
            if (didl.IsValid() && !didl.GetItems().empty())
              prop->AVTransportURIMetaData = didl.GetItems()[0];
            else
              prop->AVTransportURIMetaData.reset(new DigitalItem(DigitalItem::Type_unknown));
          }
          else if (*it == "NextAVTransportURI")
            prop->NextAVTransportURI.assign(*++it);
          else if (*it == "NextAVTransportURIMetaData")
            prop->NextAVTransportURIMetaData.assign(*++it);
          else if (*it == "CurrentTransportActions")
            prop->CurrentTransportActions.assign(*++it);
          else if (*it == "r:CurrentValidPlayModes")
            prop->r_CurrentValidPlayModes.assign(*++it);
          else if (*it == "r:MuseSessions")
            prop->r_MuseSessions.assign(*++it);
          else if (*it == "TransportStatus")
            prop->TransportStatus.assign(*++it);
          else if (*it == "r:SleepTimerGeneration")
            prop->r_SleepTimerGeneration.assign(*++it);
          else if (*it == "r:AlarmRunning")
            prop->r_AlarmRunning.assign(*++it);
          else if (*it == "r:AlarmIDRunning")
            prop->r_AlarmIDRunning.assign(*++it);
          else if (*it == "r:AlarmLoggedStartTime")
            prop->r_AlarmLoggedStartTime.assign(*++it);
          else if (*it == "r:AlarmState")
            prop->r_AlarmState.assign(*++it);
          else if (*it == "r:SnoozeRunning")
            prop->r_SnoozeRunning.assign(*++it);
          else if (*it == "r:RestartPending")
            prop->r_RestartPending.assign(*++it);
          else if (*it == "PossiblePlaybackStorageMedia")
            prop->PossiblePlaybackStorageMedia.assign(*++it);

          ++it;
        }
        // END CRITICAL SECTION
      }
      // Signal
      ++m_msgCount;
      if (m_eventCB)
        m_eventCB(m_CBHandle);
    }
  }
}
