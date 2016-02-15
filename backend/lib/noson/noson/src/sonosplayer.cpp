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

#include "sonosplayer.h"
#include "avtransport.h"
#include "deviceproperties.h"
#include "renderingcontrol.h"
#include "contentdirectory.h"
#include "private/builtin.h"
#include "private/cppdef.h"
#include "private/debug.h"
#include "private/uriparser.h"

using namespace NSROOT;

Player::Player(const ZonePtr& zone, EventHandler& eventHandler, void* CBHandle, EventCB eventCB)
: m_valid(false)
, m_uuid()
, m_host()
, m_port(0)
, m_eventHandler(eventHandler)
, m_CBHandle(CBHandle)
, m_eventCB(eventCB)
, m_eventSignaled(false)
, m_eventMask(0)
, m_AVTransport(0)
, m_deviceProperties(0)
, m_contentDirectory(0)
{
  if (!zone)
    DBG(DBG_ERROR, "%s: invalid zone\n", __FUNCTION__);
  else
  {
    ZonePlayerPtr cinfo = zone->GetCoordinator();
    if (cinfo)
    {
      if (cinfo->IsValid())
      {
        DBG(DBG_DEBUG, "%s: initialize player '%s' as coordinator (%s:%u)\n", __FUNCTION__, cinfo->c_str(), cinfo->GetHost().c_str(), cinfo->GetPort());
        m_uuid = cinfo->GetUUID();
        m_host = cinfo->GetHost();
        m_port = cinfo->GetPort();
        Init(*zone);
      }
      else
        DBG(DBG_ERROR, "%s: invalid coordinator for zone '%s' (%s)\n", __FUNCTION__, zone->GetZoneName().c_str(), cinfo->GetLocation().c_str());
    }
    else
      DBG(DBG_ERROR, "%s: zone '%s' hasn't any coordinator\n", __FUNCTION__, zone->GetZoneName().c_str());
  }
}

Player::Player(const ZonePlayerPtr& zonePlayer, EventHandler& eventHandler, void* CBHandle, EventCB eventCB)
: m_valid(false)
, m_uuid()
, m_host()
, m_port(0)
, m_eventHandler(eventHandler)
, m_CBHandle(CBHandle)
, m_eventCB(eventCB)
, m_eventSignaled(false)
, m_eventMask(0)
, m_AVTransport(0)
, m_deviceProperties(0)
, m_contentDirectory(0)
{
  if (zonePlayer && zonePlayer->IsValid())
  {
    DBG(DBG_DEBUG, "%s: initialize player '%s' (%s:%u)\n", __FUNCTION__, zonePlayer->c_str(), zonePlayer->GetHost().c_str(), zonePlayer->GetPort());
    m_uuid = zonePlayer->GetUUID();
    m_host = zonePlayer->GetHost();
    m_port = zonePlayer->GetPort();
    Zone fake;
    fake.push_back(zonePlayer);
    Init(fake);
  }
  else
    DBG(DBG_ERROR, "%s: invalid zone player\n", __FUNCTION__);
}

Player::Player(const ZonePlayerPtr& zonePlayer)
: m_valid(false)
, m_uuid()
, m_host()
, m_port(0)
, m_eventHandler()
, m_CBHandle(0)
, m_eventCB(0)
, m_eventSignaled(false)
, m_eventMask(0)
, m_AVTransport(0)
, m_deviceProperties(0)
, m_contentDirectory(0)
{
  if (zonePlayer && zonePlayer->IsValid())
  {
    DBG(DBG_DEBUG, "%s: initialize player '%s' (%s:%u)\n", __FUNCTION__, zonePlayer->c_str(), zonePlayer->GetHost().c_str(), zonePlayer->GetPort());
    m_uuid = zonePlayer->GetUUID();
    m_host = zonePlayer->GetHost();
    m_port = zonePlayer->GetPort();

    SubordinateRC rc;
    rc.uuid = m_uuid;
    rc.name = *zonePlayer;
    rc.renderingControl = new RenderingControl(m_host, m_port);
    m_RCTable.push_back(rc);

    m_AVTransport = new AVTransport(m_host, m_port);
    m_contentDirectory = new ContentDirectory(m_host, m_port);
    m_deviceProperties = new DeviceProperties(m_host, m_port);

    m_queueURI.assign("x-rincon-queue:").append(m_uuid).append("#0");
    m_valid = true;
  }
  else
    DBG(DBG_ERROR, "%s: invalid zone player\n", __FUNCTION__);
}

Player::~Player()
{
  m_eventHandler.RevokeAllSubscriptions(this);
  SAFE_DELETE(m_contentDirectory);
  SAFE_DELETE(m_deviceProperties);
  SAFE_DELETE(m_AVTransport);
  for (RCTable::iterator it = m_RCTable.begin(); it != m_RCTable.end(); ++it)
    SAFE_DELETE(it->renderingControl);
}

void Player::SubordinateRC::FillSRProperty(SRProperty& srp) const
{
  srp.uuid = uuid;
  srp.subordinateName = name;
  if (renderingControl)
    srp.property = *(renderingControl->GetRenderingProperty().Get());
}

void Player::Init(const Zone& zone)
{
  unsigned subId = m_eventHandler.CreateSubscription(this);
  m_eventHandler.SubscribeForEvent(subId, EVENT_HANDLER_STATUS);

  for (Zone::const_iterator it = zone.begin(); it != zone.end(); ++it)
  {
    if ((*it)->IsValid())
    {
      SubordinateRC rc;
      rc.uuid = (*it)->GetUUID();
      rc.name = **it;
      rc.subscription = Subscription((*it)->GetHost(), (*it)->GetPort(), RenderingControl::EventURL, m_eventHandler.GetPort(), SUBSCRIPTION_TIMEOUT);
      rc.renderingControl = new RenderingControl((*it)->GetHost(), (*it)->GetPort(), m_eventHandler, rc.subscription, this, CB_RenderingControl);
      m_RCTable.push_back(rc);
    }
    else
      DBG(DBG_ERROR, "%s: invalid location for player '%s'\n", __FUNCTION__, (*it)->c_str());
  }

  m_AVTSubscription = Subscription(m_host, m_port, AVTransport::EventURL, m_eventHandler.GetPort(), SUBSCRIPTION_TIMEOUT);
  m_CDSubscription = Subscription(m_host, m_port, ContentDirectory::EventURL, m_eventHandler.GetPort(), SUBSCRIPTION_TIMEOUT);

  m_AVTransport = new AVTransport(m_host, m_port, m_eventHandler, m_AVTSubscription, this, CB_AVTransport);
  m_contentDirectory = new ContentDirectory(m_host, m_port, m_eventHandler, m_CDSubscription, this, CB_ContentDirectory);
  m_deviceProperties = new DeviceProperties(m_host, m_port);

  for (RCTable::iterator it = m_RCTable.begin(); it != m_RCTable.end(); ++it)
    it->subscription.Start();
  m_AVTSubscription.Start();
  m_CDSubscription.Start();

  m_queueURI.assign("x-rincon-queue:").append(m_uuid).append("#0");
  m_valid = true;
}

void Player::RenewSubscriptions()
{
  for (RCTable::iterator it = m_RCTable.begin(); it != m_RCTable.end(); ++it)
    it->subscription.AskRenewal();
  m_AVTSubscription.AskRenewal();
  m_CDSubscription.AskRenewal();
}

unsigned char Player::LastEvents()
{
  unsigned char mask;
  Locked<bool>::pointer _signaled = m_eventSignaled.Get();
  {
    Locked<unsigned char>::pointer _mask = m_eventMask.Get();
    mask = *_mask;
    *_mask = 0;
  }
  *_signaled = false;
  return mask;
}

bool Player::RenderingPropertyEmpty()
{
  for (RCTable::const_iterator it = m_RCTable.begin(); it != m_RCTable.end(); ++it)
  {
    if (it->renderingControl->Empty())
      return true;
  }
  return false;
}

SRPList Player::GetRenderingProperty()
{
  SRPList list;
  for (RCTable::const_iterator it = m_RCTable.begin(); it != m_RCTable.end(); ++it)
  {
    list.push_back(SRProperty());
    it->FillSRProperty(list.back());
  }
  return list;
}

bool Player::TransportPropertyEmpty()
{
  return m_AVTransport->Empty();
}

AVTProperty Player::GetTransportProperty()
{
  return *(m_AVTransport->GetAVTProperty().Get());
}

ContentProperty Player::GetContentProperty()
{
  return *(m_contentDirectory->GetContentProperty().Get());
}

bool Player::RefreshShareIndex()
{
  return m_contentDirectory->RefreshShareIndex();
}

bool Player::GetZoneInfo(ElementList& vars)
{
  return m_deviceProperties->GetZoneInfo(vars);
}

bool Player::GetZoneAttributes(ElementList& vars)
{
  return m_deviceProperties->GetZoneAttributes(vars);
}

bool Player::GetTransportInfo(ElementList& vars)
{
  return m_AVTransport->GetTransportInfo(vars);
}

bool Player::GetPositionInfo(ElementList& vars)
{
  return m_AVTransport->GetPositionInfo(vars);
}

bool Player::GetMediaInfo(ElementList& vars)
{
  return m_AVTransport->GetMediaInfo(vars);
}

bool Player::GetRemainingSleepTimerDuration(ElementList& vars)
{
  // RemainingSleepTimerDuration
  // CurrentSleepTimerGeneration
  return m_AVTransport->GetRemainingSleepTimerDuration(vars);
}

bool Player::GetVolume(const std::string& uuid, uint8_t* value)
{
  for (RCTable::const_iterator it = m_RCTable.begin(); it != m_RCTable.end(); ++it)
  {
    if (it->uuid == uuid)
      return it->renderingControl->GetVolume(value);
  }
  return false;
}

bool Player::SetVolume(const std::string& uuid, uint8_t value)
{
  for (RCTable::const_iterator it = m_RCTable.begin(); it != m_RCTable.end(); ++it)
  {
    if (it->uuid == uuid)
      return it->renderingControl->SetVolume(value);
  }
  return false;
}

bool Player::GetMute(const std::string& uuid, uint8_t* value)
{
  for (RCTable::const_iterator it = m_RCTable.begin(); it != m_RCTable.end(); ++it)
  {
    if (it->uuid == uuid)
      return it->renderingControl->GetMute(value);
  }
  return false;
}

bool Player::SetMute(const std::string& uuid, uint8_t value)
{
  for (RCTable::const_iterator it = m_RCTable.begin(); it != m_RCTable.end(); ++it)
  {
    if (it->uuid == uuid)
      return it->renderingControl->SetMute(value);
  }
  return false;
}

bool Player::SetCurrentURI(const DigitalItemPtr& item)
{
  // Check for radio service tuneIN
  if (item->GetObjectID().compare(0, 4, "R:0/") == 0)
  {
    ElementPtr var(new Element("desc", NetServiceDescTable[NetService_TuneIN]));
    var->SetAttribut("id", "cdudn");
    var->SetAttribut("nameSpace", "urn:schemas-rinconnetworks-com:metadata-1-0/");
    item->SetProperty(var);
  }
  return m_AVTransport->SetCurrentURI(item->GetValue("res"), item->DIDL());
}

bool Player::SetCurrentURI(const std::string& uri, const std::string& title)
{
  URIParser _uri(uri);
  if (_uri.Scheme())
  {
    std::string protocolInfo;
    if (strncmp(_uri.Scheme(), "http", 4) == 0)
      protocolInfo.assign(ProtocolTable[Protocol_httpGet]);
    else
      protocolInfo.assign(_uri.Scheme());
    protocolInfo.append(":*:*:*");
    // Setup the digital item
    DigitalItemPtr item(new DigitalItem(DigitalItem::Type_item));
    item->SetProperty(ElementPtr(new Element("dc:title", title)));
    ElementPtr res(new Element("res", uri));
    res->SetAttribut("protocolInfo", protocolInfo);
    item->SetProperty(res);
    return SetCurrentURI(item);
  }
  return false;
}

bool Player::PlayQueue(bool start)
{
  if (m_AVTransport->SetCurrentURI(m_queueURI, ""))
  {
    if (start)
      return m_AVTransport->Play();
    return true;
  }
  return false;
}

unsigned Player::AddURIToQueue(const DigitalItemPtr& item, unsigned position)
{
  return m_AVTransport->AddURIToQueue(item->GetValue("res"), item->DIDL(), position);
}

unsigned Player::AddMultipleURIsToQueue(const std::vector<DigitalItemPtr>& items)
{
  unsigned tno = 0;
  std::vector<std::string> uris;
  std::vector<std::string> metadatas;
  std::vector<DigitalItemPtr>::const_iterator it = items.begin();
  while (it != items.end())
  {
    while (uris.size() < 16 && it != items.end())
    {
      uris.push_back((*it)->GetValue("res"));
      metadatas.push_back((*it)->DIDL());
      ++it;
    }
    unsigned r = m_AVTransport->AddMultipleURIsToQueue(uris, metadatas);
    if (!r)
      break;
    if (!tno) // save first track number
      tno = r;
    uris.clear();
    metadatas.clear();
  }
  return tno;
}

bool Player::RemoveAllTracksFromQueue()
{
  return m_AVTransport->RemoveAllTracksFromQueue();
}

bool Player::RemoveTrackFromQueue(const std::string& objectID, unsigned containerUpdateID)
{
  return m_AVTransport->RemoveTrackFromQueue(objectID, containerUpdateID);
}

bool Player::ReorderTracksInQueue(unsigned startIndex, unsigned numTracks, unsigned insBefore, unsigned containerUpdateID)
{
  return m_AVTransport->ReorderTracksInQueue(startIndex, numTracks, insBefore, containerUpdateID);
}

bool Player::SaveQueue(const std::string& title)
{
  return m_AVTransport->SaveQueue(title);
}

bool Player::CreateSavedQueue(const std::string& title)
{
  return m_AVTransport->CreateSavedQueue(title);
}

unsigned Player::AddURIToSavedQueue(const std::string& SQObjectID, const DigitalItemPtr& item, unsigned containerUpdateID)
{
  return m_AVTransport->AddURIToSavedQueue(SQObjectID, item->GetValue("res"), item->DIDL(), containerUpdateID);
}

bool Player::ReorderTracksInSavedQueue(const std::string& SQObjectID, const std::string& trackList, const std::string& newPositionList, unsigned containerUpdateID)
{
  return m_AVTransport->ReorderTracksInSavedQueue(SQObjectID, trackList, newPositionList, containerUpdateID);
}

bool Player::DestroySavedQueue(const std::string& SQObjectID)
{
  return m_contentDirectory->DestroyObject(SQObjectID);
}

bool Player::SetPlayMode(PlayMode_t mode)
{
  return m_AVTransport->SetPlayMode(mode);
}

bool Player::Play()
{
  return m_AVTransport->Play();
}

bool Player::Stop()
{
  return m_AVTransport->Stop();
}

bool Player::Pause()
{
  return m_AVTransport->Pause();
}

bool Player::SeekTime(uint16_t reltime)
{
  return m_AVTransport->SeekTime(reltime);
}

bool Player::SeekTrack(unsigned tracknr)
{
  return m_AVTransport->SeekTrack(tracknr);
}

bool Player::Next()
{
  return m_AVTransport->Next();
}

bool Player::Previous()
{
  return m_AVTransport->Previous();
}

bool Player::ConfigureSleepTimer(unsigned seconds)
{
  return m_AVTransport->ConfigureSleepTimer(seconds);
}

bool Player::BecomeStandalone()
{
  return m_AVTransport->BecomeCoordinatorOfStandaloneGroup();
}

bool Player::JoinToGroup(const std::string& coordinatorUUID)
{
  std::string uri(ProtocolTable[Protocol_xRincon]);
  uri.append(":").append(coordinatorUUID);
  return m_AVTransport->SetAVTransportURI(uri, "");
}

bool Player::SwitchLineIN()
{
  std::string uri(ProtocolTable[Protocol_xRinconStream]);
  uri.append(":").append(m_uuid);
  return m_AVTransport->SetAVTransportURI(uri, "");
}

bool Player::SwitchTvSPDIF()
{
  std::string uri(ProtocolTable[Protocol_xSonosHtastream]);
  uri.append(":").append(m_uuid).append(":spdif");
  return m_AVTransport->SetAVTransportURI(uri, "");
}

ContentDirectory* Player::ContentDirectoryProvider(void* CBHandle, EventCB eventCB)
{
  return new ContentDirectory(GetHost(), GetPort(), m_eventHandler, m_CDSubscription, CBHandle, eventCB);
}

void Player::HandleEventMessage(EventMessagePtr msg)
{
  if (!msg)
    return;
  if (msg->event == EVENT_HANDLER_STATUS)
  {
    // @TODO: handle status
    DBG(DBG_DEBUG, "%s: %s\n", __FUNCTION__, msg->subject[0].c_str());
  }
}

void Player::CB_AVTransport(void* handle)
{
  Player* _handle = static_cast<Player*>(handle);
  Locked<unsigned char>::pointer _mask = _handle->m_eventMask.Get();
  *_mask |= SVCEvent_TransportChanged;
  if (_handle->m_eventCB && !_handle->m_eventSignaled.Load())
    _handle->m_eventCB(_handle->m_CBHandle);
}

void Player::CB_RenderingControl(void* handle)
{
  Player* _handle = static_cast<Player*>(handle);
  Locked<unsigned char>::pointer _mask = _handle->m_eventMask.Get();
  *_mask |= SVCEvent_RenderingControlChanged;
  if (_handle->m_eventCB && !_handle->m_eventSignaled.Load())
    _handle->m_eventCB(_handle->m_CBHandle);
}

void Player::CB_ContentDirectory(void* handle)
{
  Player* _handle = static_cast<Player*>(handle);
  Locked<unsigned char>::pointer _mask = _handle->m_eventMask.Get();
  *_mask |= SVCEvent_ContentDirectoryChanged;
  if (_handle->m_eventCB && !_handle->m_eventSignaled.Load())
    _handle->m_eventCB(_handle->m_CBHandle);
}
