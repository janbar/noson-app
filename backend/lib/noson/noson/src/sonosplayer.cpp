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

#include "sonosplayer.h"
#include "avtransport.h"
#include "deviceproperties.h"
#include "renderingcontrol.h"
#include "contentdirectory.h"
#include "private/builtin.h"
#include "private/cppdef.h"
#include "private/debug.h"
#include "private/uriparser.h"
#include "private/tokenizer.h"
#include "private/urlencoder.h"
#include "private/socket.h"
#include "didlparser.h"
#include "sonossystem.h"
#include "filestreamer.h"
#include "imageservice.h"
#ifdef HAVE_PULSEAUDIO
#include "pulsestreamer.h"
#endif

#include <cassert>

using namespace NSROOT;

Player::Player(const ZonePtr& zone, EventHandler& eventHandler, void* CBHandle, EventCB eventCB)
: m_valid(false)
, m_zone()
, m_eventHandler(eventHandler)
, m_uuid()
, m_deviceHost()
, m_devicePort(0)
, m_CBHandle(CBHandle)
, m_eventCB(eventCB)
, m_eventSignaled(false)
, m_eventMask(0)
, m_deviceProperties(0)
, m_AVTransport(0)
, m_contentDirectory(0)
{
  m_controllerLocalUri.assign(ProtocolTable[Protocol_http])
    .append("://").append(m_eventHandler.GetAddress())
    .append(":").append(std::to_string(m_eventHandler.GetPort()));
  m_valid = Init(zone);
}

Player::Player(const ZonePlayerPtr& zonePlayer)
: m_valid(false)
, m_zone()
, m_eventHandler()
, m_uuid()
, m_deviceHost()
, m_devicePort(0)
, m_CBHandle(0)
, m_eventCB(0)
, m_eventSignaled(false)
, m_eventMask(0)
, m_deviceProperties(0)
, m_AVTransport(0)
, m_contentDirectory(0)
{
  if (zonePlayer && zonePlayer->IsValid())
  {
    DBG(DBG_DEBUG, "%s: initialize player '%s' (%s:%u)\n", __FUNCTION__, zonePlayer->c_str(), zonePlayer->GetHost().c_str(), zonePlayer->GetPort());
    m_uuid = zonePlayer->GetUUID();
    m_deviceHost = zonePlayer->GetHost();
    m_devicePort = zonePlayer->GetPort();

    SubordinateRC rc;
    rc.uuid = m_uuid;
    rc.name = *zonePlayer;
    rc.renderingControl = new RenderingControl(m_deviceHost, m_devicePort);
    m_RCTable.push_back(rc);

    m_deviceProperties = new DeviceProperties(m_deviceHost, m_devicePort);
    m_AVTransport = new AVTransport(m_deviceHost, m_devicePort);
    m_contentDirectory= new ContentDirectory(m_deviceHost, m_devicePort);

    m_valid = true;
  }
  else
    DBG(DBG_ERROR, "%s: invalid zone player\n", __FUNCTION__);
}

Player::~Player()
{
  m_eventHandler.RevokeAllSubscriptions(this);
  SAFE_DELETE(m_contentDirectory);
  SAFE_DELETE(m_AVTransport);
  SAFE_DELETE(m_deviceProperties);
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

bool Player::Init(const ZonePtr& zone)
{
  if (!zone)
  {
    DBG(DBG_ERROR, "%s: invalid zone\n", __FUNCTION__);
    return false;
  }
  ZonePlayerPtr cinfo = zone->GetCoordinator();
  if (!cinfo || !cinfo->IsValid())
  {
    DBG(DBG_ERROR, "%s: invalid coordinator for zone '%s' (%s)\n", __FUNCTION__, zone->GetZoneName().c_str(), cinfo->GetLocation().c_str());
    return false;
  }
  DBG(DBG_DEBUG, "%s: initialize player '%s' as coordinator (%s:%u)\n", __FUNCTION__, cinfo->c_str(), cinfo->GetHost().c_str(), cinfo->GetPort());
  m_zone = zone;
  m_uuid = cinfo->GetUUID();
  m_deviceHost = cinfo->GetHost();
  m_devicePort = cinfo->GetPort();

  TcpSocket sock;
  sock.Connect(m_deviceHost.c_str(), m_devicePort, 0);
  m_controllerName = TcpSocket::GetMyHostName();
  m_controllerHost = sock.GetHostAddrInfo();
  sock.Disconnect();
  m_controllerUri.assign(ProtocolTable[Protocol_http])
      .append("://").append(m_controllerHost)
      .append(":").append(std::to_string(m_eventHandler.GetPort()));

  unsigned subId = m_eventHandler.CreateSubscription(this);
  m_eventHandler.SubscribeForEvent(subId, EVENT_HANDLER_STATUS);

  // initialize subordinates
  for (Zone::const_iterator it = zone->begin(); it != zone->end(); ++it)
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

  m_deviceProperties = new DeviceProperties(m_deviceHost, m_devicePort);

  m_AVTSubscription = Subscription(m_deviceHost, m_devicePort, AVTransport::EventURL, m_eventHandler.GetPort(), SUBSCRIPTION_TIMEOUT);
  m_AVTransport = new AVTransport(m_deviceHost, m_devicePort, m_eventHandler, m_AVTSubscription, this, CB_AVTransport);

  m_CDSubscription = Subscription(m_deviceHost, m_devicePort, ContentDirectory::EventURL, m_eventHandler.GetPort(), SUBSCRIPTION_TIMEOUT);
  m_contentDirectory = new ContentDirectory(m_deviceHost, m_devicePort, m_eventHandler, m_CDSubscription, this, CB_ContentDirectory);

  for (RCTable::iterator it = m_RCTable.begin(); it != m_RCTable.end(); ++it)
    it->subscription.Start();
  m_AVTSubscription.Start();
  m_CDSubscription.Start();
  return true;
}

void Player::CB_AVTransport(void* handle)
{
  Player* _handle = static_cast<Player*>(handle);
  assert(_handle);
  {
    // BEGIN CRITICAL SECTION
    Locked<unsigned char>::pointer _mask = _handle->m_eventMask.Get();
    *_mask |= SVCEvent_TransportChanged;
    // END CRITICAL SECTION
  }
  if (_handle->m_eventCB && !_handle->m_eventSignaled.Load())
    _handle->m_eventCB(_handle->m_CBHandle);
}

void Player::CB_RenderingControl(void* handle)
{
  Player* _handle = static_cast<Player*>(handle);
  assert(_handle);
  {
    // BEGIN CRITICAL SECTION
    Locked<unsigned char>::pointer _mask = _handle->m_eventMask.Get();
    *_mask |= SVCEvent_RenderingControlChanged;
    // END CRITICAL SECTION
  }
  if (_handle->m_eventCB && !_handle->m_eventSignaled.Load())
    _handle->m_eventCB(_handle->m_CBHandle);
}

void Player::CB_ContentDirectory(void* handle)
{
  Player* _handle = static_cast<Player*>(handle);
  assert(_handle);
  {
    // BEGIN CRITICAL SECTION
    Locked<unsigned char>::pointer _mask = _handle->m_eventMask.Get();
    *_mask |= SVCEvent_ContentDirectoryChanged;
    // END CRITICAL SECTION
  }
  if (_handle->m_eventCB && !_handle->m_eventSignaled.Load())
    _handle->m_eventCB(_handle->m_CBHandle);
}

void Player::RevokeSubscription()
{
  m_AVTSubscription.Stop();
  for (RCTable::iterator it = m_RCTable.begin(); it != m_RCTable.end(); ++it)
    it->subscription.Stop();
}

void Player::RenewSubscriptions()
{
  for (RCTable::iterator it = m_RCTable.begin(); it != m_RCTable.end(); ++it)
    it->subscription.AskRenewal();
  m_AVTSubscription.AskRenewal();
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

bool Player::GetZoneInfo(ElementList& vars)
{
  return m_deviceProperties->GetZoneInfo(vars);
}

bool Player::GetZoneAttributes(ElementList& vars)
{
  return m_deviceProperties->GetZoneAttributes(vars);
}

bool Player::GetHouseholdID(ElementList& vars)
{
  return m_deviceProperties->GetHouseholdID(vars);
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

bool Player::GetNightmode(const std::string &uuid, uint8_t *value)
{
  for (RCTable::const_iterator it = m_RCTable.begin(); it != m_RCTable.end(); ++it)
  {
    if (it->uuid == uuid)
      return it->renderingControl->GetNightmode(value);
  }
  return false;
}

bool Player::SetNightmode(const std::string &uuid, uint8_t value)
{
  for (RCTable::const_iterator it = m_RCTable.begin(); it != m_RCTable.end(); ++it)
  {
    if (it->uuid == uuid)
      return it->renderingControl->SetNightmode(value);
  }
  return false;
}

bool Player::GetLoudness(const std::string &uuid, uint8_t *value)
{
  for (RCTable::const_iterator it = m_RCTable.begin(); it != m_RCTable.end(); ++it)
  {
    if (it->uuid == uuid)
      return it->renderingControl->GetLoudness(value);
  }
  return false;
}

bool Player::SetLoudness(const std::string &uuid, uint8_t value)
{
  for (RCTable::const_iterator it = m_RCTable.begin(); it != m_RCTable.end(); ++it)
  {
    if (it->uuid == uuid)
      return it->renderingControl->SetLoudness(value);
  }
  return false;
}

bool Player::GetBass(const std::string &uuid, int8_t* value)
{
  for (RCTable::const_iterator it = m_RCTable.begin(); it != m_RCTable.end(); ++it)
  {
    if (it->uuid == uuid)
      return it->renderingControl->GetBass(value);
  }
  return false;
}

bool Player::SetBass(const std::string &uuid, int8_t value)
{
  for (RCTable::const_iterator it = m_RCTable.begin(); it != m_RCTable.end(); ++it)
  {
    if (it->uuid == uuid)
      return it->renderingControl->SetBass(value);
  }
  return false;
}

bool Player::GetTreble(const std::string &uuid, int8_t* value)
{
  for (RCTable::const_iterator it = m_RCTable.begin(); it != m_RCTable.end(); ++it)
  {
    if (it->uuid == uuid)
      return it->renderingControl->GetTreble(value);
  }
  return false;
}

bool Player::SetTreble(const std::string &uuid, int8_t value)
{
  for (RCTable::const_iterator it = m_RCTable.begin(); it != m_RCTable.end(); ++it)
  {
    if (it->uuid == uuid)
      return it->renderingControl->SetTreble(value);
  }
  return false;
}

bool Player::GetSupportsOutputFixed(const std::string &uuid, uint8_t* value)
{
  for (RCTable::const_iterator it = m_RCTable.begin(); it != m_RCTable.end(); ++it)
  {
    if (it->uuid == uuid)
      return it->renderingControl->GetSupportsOutputFixed(value);
  }
  return false;
}

bool Player::GetOutputFixed(const std::string &uuid, uint8_t* value)
{
  for (RCTable::const_iterator it = m_RCTable.begin(); it != m_RCTable.end(); ++it)
  {
    if (it->uuid == uuid)
      return it->renderingControl->GetOutputFixed(value);
  }
  return false;
}

bool Player::SetOutputFixed(const std::string &uuid, uint8_t value)
{
  for (RCTable::const_iterator it = m_RCTable.begin(); it != m_RCTable.end(); ++it)
  {
    if (it->uuid == uuid)
      return it->renderingControl->SetOutputFixed(value);
  }
  return false;
}

bool Player::SetCurrentURI(const DigitalItemPtr& item)
{
  if (!item)
    return false;
  return m_AVTransport->SetCurrentURI(item->GetValue("res"), item->DIDL());
}

bool Player::PlayPulse()
{
  RequestBroker::ResourcePtr res(nullptr);
#ifdef HAVE_PULSEAUDIO
  RequestBrokerPtr rb = m_eventHandler.GetRequestBroker(PULSESTREAMER_CNAME);
  if (rb)
    res = rb->GetResource(PULSESTREAMER_CNAME);
#endif
  if (res)
  {
    bool hasParam = (res->uri.find("?") != std::string::npos);
    // define the stream URL for the local handler
    std::string streamURL;
    streamURL.assign(m_controllerUri).append(res->uri)
        .append(hasParam ? "&" : "?")
        .append("acr=").append(m_controllerName).append(":").append(std::to_string(m_eventHandler.GetPort()));
    // define the icon URL for the local handler
    std::string iconURL;
    iconURL.assign(m_controllerUri).append(res->iconUri);

    // write my formatted name
    std::string _title = res->description;
    _title.replace(res->description.find("%s"), 2, m_controllerName);
    return PlayStream(streamURL, _title, iconURL);
  }
  DBG(DBG_ERROR, "%s: service unavaible\n", __FUNCTION__);
  return false;
}

bool Player::IsPulseStream(const std::string& streamURL)
{
  RequestBrokerPtr rb(nullptr);
#ifdef HAVE_PULSEAUDIO
  rb = m_eventHandler.GetRequestBroker(PULSESTREAMER_CNAME);
#endif
  if (rb && IsMyStream(streamURL))
  {
    RequestBroker::ResourceList list = rb->GetResourceList();
    for (RequestBroker::ResourceList::iterator it = list.begin(); it != list.end(); ++it)
      if (streamURL.find((*it)->uri) != std::string::npos)
        return true;
  }
  return false;
}

bool Player::IsMyStream(const std::string& streamURL)
{
  return (streamURL.find(m_controllerUri) == 0);
}

bool Player::PlayStream(const std::string& streamURL, const std::string& title, const std::string& iconURL)
{
  URIParser _uri(streamURL);
  if (_uri.Scheme() && _uri.Path())
  {
    std::string path(_uri.Path());
    std::string file = path.substr(0, path.find('?'));
    std::string mime = "";
    if (file.find('.') != std::string::npos)
      mime = file.substr(file.find_last_of('.'));
    /*
     * Configure an audio FLAC transfer for any resource with the corresponding extension
     */
    if (mime == ".flac")
    {
      std::string protocolInfo;
      protocolInfo.assign(ProtocolTable[Protocol_httpGet]).append(":*:audio/flac:*");
      // Setup the digital item
      DigitalItemPtr item(new DigitalItem(DigitalItem::Type_item, DigitalItem::SubType_audioItem));
      item->SetProperty(DIDL_QNAME_DC "title", title);
      item->SetProperty(DIDL_QNAME_RINC "streamContent", "");
      if (!iconURL.empty())
        item->SetProperty(DIDL_QNAME_UPNP "albumArtURI", iconURL);
      ElementPtr res(new Element("res", streamURL));
      res->SetAttribut("protocolInfo", protocolInfo);
      item->SetProperty(res);
      DBG(DBG_DEBUG, "%s: %s\n%s\n", __FUNCTION__, item->GetValue("res").c_str(), item->DIDL().c_str());
      return SetCurrentURI(item) && m_AVTransport->Play();
    }
    /*
     * Else configure as MP3Radio
     */
    else
    {
      std::string val;
      std::string protocolInfo;
      val.assign(ProtocolTable[Protocol_xRinconMP3Radio]).append(streamURL.substr(streamURL.find(":")));
      protocolInfo.assign(ProtocolTable[Protocol_xRinconMP3Radio]).append(":*:*:*");
      // Setup the digital item
      DigitalItemPtr item(new DigitalItem(DigitalItem::Type_item, DigitalItem::SubType_audioItem));
      item->SetProperty(DIDL_QNAME_DC "title", title);
      item->SetProperty(DIDL_QNAME_RINC "streamContent", "");
      if (!iconURL.empty())
        item->SetProperty(DIDL_QNAME_UPNP "albumArtURI", iconURL);
      ElementPtr res(new Element("res", val));
      res->SetAttribut("protocolInfo", protocolInfo);
      item->SetProperty(res);
      DBG(DBG_DEBUG, "%s: %s\n%s\n", __FUNCTION__, item->GetValue("res").c_str(), item->DIDL().c_str());
      return SetCurrentURI(item) && m_AVTransport->Play();
    }
  }
  return false;
}

bool Player::PlayStream(const std::string& streamURL, const std::string& title)
{
  return PlayStream(streamURL, title, "");
}

bool Player::PlayQueue(bool start)
{
  std::string uri;
  uri.append(ProtocolTable[Protocol_xRinconQueue]).append(":").append(m_uuid).append("#0");
  if (m_AVTransport->SetCurrentURI(uri, ""))
  {
    if (start)
      return m_AVTransport->Play();
    return true;
  }
  return false;
}

unsigned Player::AddURIToQueue(const DigitalItemPtr& item, unsigned position)
{
  if (item)
    return m_AVTransport->AddURIToQueue(item->GetValue("res"), item->DIDL(), position);
  return 0;
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
      if (*it)
      {
        uris.push_back((*it)->GetValue("res"));
        metadatas.push_back((*it)->DIDL());
      }
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
  if (item)
    return m_AVTransport->AddURIToSavedQueue(SQObjectID, item->GetValue("res"), item->DIDL(), containerUpdateID);
  return 0;
}

bool Player::ReorderTracksInSavedQueue(const std::string& SQObjectID, const std::string& trackList, const std::string& newPositionList, unsigned containerUpdateID)
{
  return m_AVTransport->ReorderTracksInSavedQueue(SQObjectID, trackList, newPositionList, containerUpdateID);
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

bool Player::PlayLineIN()
{
  std::string uri(ProtocolTable[Protocol_xRinconStream]);
  uri.append(":").append(m_uuid);
  return m_AVTransport->SetCurrentURI(uri, "") && m_AVTransport->Play();
}

bool Player::PlayDigitalIN()
{
  std::string uri(ProtocolTable[Protocol_xSonosHtaStream]);
  uri.append(":").append(m_uuid).append(":spdif");
  return m_AVTransport->SetCurrentURI(uri, "") && m_AVTransport->Play();
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

Protocol_t Player::GetURIProtocol(const std::string& uri)
{
  size_t pos = uri.find_first_of(":");
  if (pos == std::string::npos)
    return Protocol_unknown;
  std::string proto = uri.substr(0, pos);
  int i = 0;
  while (i < Protocol_unknown && proto != ProtocolTable[i])
    ++i;
  return static_cast<Protocol_t>(i);
}

std::string Player::MakeFilePictureUrl(const std::string& filePath)
{
  std::string pictureURL;
  RequestBrokerPtr imageService = m_eventHandler.GetRequestBroker(IMAGESERVICE_CNAME);
  if (imageService)
  {
    std::string pictureURI = static_cast<ImageService*>(imageService.get())->MakeFilePictureURI(filePath);
    if (!pictureURI.empty())
      pictureURL.assign(m_controllerUri).append(pictureURI);
  }
  return pictureURL;
}

std::string Player::MakeFilePictureLocalUrl(const std::string& filePath)
{
  std::string pictureURL;
  RequestBrokerPtr imageService = m_eventHandler.GetRequestBroker(IMAGESERVICE_CNAME);
  if (imageService)
  {
    std::string pictureURI = static_cast<ImageService*>(imageService.get())->MakeFilePictureURI(filePath);
    if (!pictureURI.empty())
      pictureURL.assign(m_controllerLocalUri).append(pictureURI);
  }
  return pictureURL;
}

DigitalItemPtr Player::MakeFileStreamItem(const std::string& filePath, const std::string& codec, const std::string& title, const std::string& album,
                                          const std::string& author, const std::string& duration, bool hasArt)
{
  // find the service else return null
  RequestBrokerPtr rbfs = m_eventHandler.GetRequestBroker(FILESTREAMER_CNAME);
  if (!rbfs)
  {
    DBG(DBG_ERROR, "%s: service unavaible\n", __FUNCTION__);
    return DigitalItemPtr();
  }
  FileStreamer * fileStreamer = static_cast<FileStreamer*>(rbfs.get());
  std::string streamURI = fileStreamer->MakeFileStreamURI(filePath, codec);
  if (streamURI.empty())
  {
    DBG(DBG_ERROR, "%s: file type not supported (%s)\n", __FUNCTION__, codec.c_str());
    return DigitalItemPtr();
  }
  // define the base URL for the local handler
  std::string streamURL;
  streamURL.assign(m_controllerUri).append(streamURI);
  std::string itemId("00092020"); // disabled seeking
  itemId.append(m_controllerName).append(":").append(streamURI);

  // define the picture URL for the local handler
  std::string pictureURL;
  if (hasArt)
    pictureURL = MakeFilePictureUrl(filePath);

  // build the digital item
  DigitalItemPtr item(new DigitalItem(DigitalItem::Type_item, DigitalItem::SubType_audioItem));
  item->SetParentID("-1");
  item->SetObjectID(itemId);
  item->SetProperty(DIDL_QNAME_DC "title", title);
  item->SetProperty(DIDL_QNAME_UPNP "album", album);
  item->SetProperty(DIDL_QNAME_DC "creator", author);
  item->SetProperty(DIDL_QNAME_UPNP "albumArtURI", pictureURL);

  std::string hms;
  unsigned hh, hm, hs;
  if (sscanf(duration.c_str(), "%u:%u:%u", &hh, &hm, &hs) == 3)
    hms.assign(duration);
  else
  {
    char str[9];
    unsigned val = 0;
    string_to_uint32(duration.c_str(), &val);
    if (val > 0)
    {
      unsigned hh, hm, hs;
      hh = val / 3600;
      hm = (val - (hh * 3600)) / 60;
      hs = val - (hh * 3600) - (hm * 60);
      snprintf(str, sizeof(str), "%02u:%02u:%02u", hh & 0x3f, hm & 0x3f, hs & 0x3f);
    }
    else
      strncpy(str, "00:59:59", sizeof(str)); // default by long duration
    hms.assign(str, sizeof(str) - 1);
  }

  const FileStreamer::codec_type * codecType = fileStreamer->GetCodec(codec);
  assert(codecType);

  std::string protocolInfo;
  protocolInfo.assign(ProtocolTable[Protocol_httpGet]).append(":*:").append(codecType->mime).append(":*");
  ElementPtr res(new Element("res", streamURL));
  res->SetAttribut("protocolInfo", protocolInfo);
  res->SetAttribut("duration", hms);
  item->SetProperty(res);
  DBG(DBG_DEBUG, "%s: %s\n%s\n", __FUNCTION__, item->GetValue("res").c_str(), item->DIDL().c_str());
  return item;
}
