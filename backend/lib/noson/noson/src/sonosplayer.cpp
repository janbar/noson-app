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
 *  along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
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
#include "didlparser.h"
#include "sonossystem.h"
#include "smapimetadata.h"

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
, m_musicServices(0)
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
, m_musicServices(0)
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
, m_musicServices(0)
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
    m_musicServices = new MusicServices(m_host, m_port);

    // fill avaialable music services
    m_smservices = m_musicServices->GetAvailableServices();

    m_queueURI.assign("x-rincon-queue:").append(m_uuid).append("#0");
    m_valid = true;
  }
  else
    DBG(DBG_ERROR, "%s: invalid zone player\n", __FUNCTION__);
}

Player::~Player()
{
  m_eventHandler.RevokeAllSubscriptions(this);
  SAFE_DELETE(m_musicServices);
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
  m_musicServices = new MusicServices(m_host, m_port);

  // fill available music services
  m_smservices = m_musicServices->GetAvailableServices();

  for (RCTable::iterator it = m_RCTable.begin(); it != m_RCTable.end(); ++it)
    it->subscription.Start();
  m_AVTSubscription.Start();
  m_CDSubscription.Start();

  m_queueURI.assign("x-rincon-queue:").append(m_uuid).append("#0");
  m_valid = true;
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

bool Player::GetHouseholdID(ElementList& vars)
{
  return m_deviceProperties->GetHouseholdID(vars);
}

bool Player::GetSessionId(const std::string& serviceId, const std::string& username, ElementList& vars)
{
  return m_musicServices->GetSessionId(serviceId, username, vars);
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
  // Fix items from 'My radios' haven't required tag desc
  SMServicePtr svc = GetServiceForMedia(item->GetValue("res"));
  if (svc && item->GetValue("desc").empty())
  {
    DigitalItem _item(DigitalItem::Type_unknown, DigitalItem::SubType_unknown);
    item->Clone(_item);
    ElementPtr var(new Element("desc", svc->GetServiceDesc()));
    var->SetAttribut("id", "cdudn");
    var->SetAttribut("nameSpace", DIDL_XMLNS_RINC);
    _item.SetProperty(var);
    return m_AVTransport->SetCurrentURI(_item.GetValue("res"), _item.DIDL());
  }
  return m_AVTransport->SetCurrentURI(item->GetValue("res"), item->DIDL());
}

bool Player::PlayStream(const std::string& streamURL, const std::string& title)
{
  URIParser _uri(streamURL);
  if (_uri.Scheme())
  {
    std::string val;
    std::string protocolInfo;
    val.assign(ProtocolTable[Protocol_xRinconMP3Radio]).append(streamURL.substr(streamURL.find(":")));
    protocolInfo.assign(ProtocolTable[Protocol_xRinconMP3Radio]).append(":*:*:*");
    // Setup the digital item
    DigitalItemPtr item(new DigitalItem(DigitalItem::Type_item, DigitalItem::SubType_audioItem));
    item->SetProperty(ElementPtr(new Element(DIDL_QNAME_DC "title", title)));
    ElementPtr res(new Element("res", val));
    res->SetAttribut("protocolInfo", protocolInfo);
    item->SetProperty(res);
    DBG(DBG_DEBUG, "%s: %s\n%s\n", __FUNCTION__, item->GetValue("res").c_str(), item->DIDL().c_str());
    return SetCurrentURI(item) && m_AVTransport->Play();
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

bool Player::AddURIToFavorites(const DigitalItemPtr& item, const std::string& description, const std::string& artURI)
{
  DigitalItemPtr favorite(new DigitalItem(DigitalItem::Type_item, DigitalItem::SubType_unknown));
  favorite->SetProperty(DIDL_QNAME_DC "title", item->GetValue(DIDL_QNAME_DC "title"));
  favorite->SetProperty(DIDL_QNAME_RINC "type", "instantPlay");
  favorite->SetProperty(item->GetProperty("res"));
  ElementPtr art = item->GetProperty(DIDL_QNAME_UPNP "albumArtURI");
  if (!art && !artURI.empty())
    art.reset(new Element(DIDL_QNAME_UPNP "albumArtURI", artURI));
  favorite->SetProperty(art);
  const std::string& album = item->GetValue(DIDL_QNAME_UPNP "album");
  const std::string& creator = item->GetValue(DIDL_QNAME_DC "creator");
  favorite->SetProperty(DIDL_QNAME_RINC "description", description.empty() ? album.empty() ? creator : album : description);
  // make r:resMD
  DigitalItem obj(DigitalItem::Type_item, DigitalItem::SubType_unknown);
  obj.SetObjectID(GetItemIdFromUriMetadata(item)); // get a valid item id
  obj.SetParentID("");
  obj.SetRestricted(item->GetRestricted());
  obj.SetProperty(item->GetProperty(DIDL_QNAME_UPNP "class"));
  obj.SetProperty(item->GetProperty(DIDL_QNAME_DC "title"));
  if (!item->GetValue("desc").empty())
    obj.SetProperty(item->GetProperty("desc"));
  else
  {
    ElementPtr desc(new Element("desc"));
    SMServicePtr svc = GetServiceForMedia(item->GetValue("res"));
    if (svc)
      desc->assign(svc->GetServiceDesc());
    else
      desc->assign(ServiceDescTable[ServiceDesc_default]);
    desc->SetAttribut("id", "cdudn");
    desc->SetAttribut("nameSpace", DIDL_XMLNS_RINC);
    obj.SetProperty(desc);
  }
  favorite->SetProperty(DIDL_QNAME_RINC "resMD", obj.DIDL());
  ContentSearch search(SearchFavorite, "");
  return m_contentDirectory->CreateObject(search.Root(), favorite);
}

bool Player::DestroyFavorite(const std::string& FVObjectID)
{
  return m_contentDirectory->DestroyObject(FVObjectID);
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

SMServiceList Player::GetEnabledServices()
{
  SMServiceList list;
  for (SMServiceList::iterator it = m_smservices.begin(); it != m_smservices.end(); ++it)
  {
    const std::string& auth = (*it)->GetPolicy()->GetAttribut("Auth");
    if ((*it)->GetContainerType() != "MService")
      continue;
    if (auth == "Anonymous" || auth == "UserId" || auth == "DeviceLink" || auth == "AppLink") {
      // Service is enabled when an account is available for the service type.
      // Otherwise 'TuneIn' is special case as it is always enabled and no account exists for it.
      if ((*it)->GetServiceType() == "65031") /* TuneIn */
      {
        list.push_back((*it)->Clone("0"));
      }
      else
      {
        SMAccountList la = SMAccount::CreateAccounts((*it)->GetServiceType());
        for (SMAccountList::iterator ita = la.begin(); ita != la.end(); ++ita)
        {
          SMServicePtr sm = (*it)->Clone((*ita)->GetSerialNum());
          sm->GetAccount()->SetCredentials((*ita)->GetCredentials());
          list.push_back(sm);
        }
      }

    }
  }
  return list;
}

SMServiceList Player::GetAvailableServices()
{
  SMServiceList list;
  for (SMServiceList::iterator it = m_smservices.begin(); it != m_smservices.end(); ++it)
  {
    const std::string& auth = (*it)->GetPolicy()->GetAttribut("Auth");
    if ((*it)->GetContainerType() != "MService")
      continue;
    if (auth == "Anonymous" || auth == "UserId" || auth == "DeviceLink" || auth == "AppLink")
      list.push_back(*it);
  }
  return list;
}

SMServicePtr Player::GetServiceForMedia(const std::string& mediaUri)
{
  SMServicePtr svc;
  URIParser parser(mediaUri);
  if (!parser.Scheme() || !parser.Path())
  {
    DBG(DBG_ERROR, "%s: invalid uri (%s)\n", __FUNCTION__, mediaUri.c_str());
    return svc;
  }
  const char* p = strchr(parser.Path(), '?');
  if (p)
  {
    std::vector<std::string> args;
    tokenize(++p, "&", args, true);
    std::string sid;
    std::string sn;
    for (std::vector<std::string>::const_iterator ita = args.begin(); ita != args.end(); ++ita)
    {
      std::vector<std::string> tokens;
      tokenize(*ita, "=", tokens);
      if (tokens.size() != 2)
        break;
      if (tokens[0] == "sid")
        sid.assign(tokens[1]);
      else if (tokens[0] == "sn")
        sn.assign(tokens[1]);
    }
    if (!sid.empty())
    {
      if (sn.empty())
        sn.assign("0"); // trying fake account

      // loop in services: no longer check the serial since commit 7a91d3ade3a428d69fe6eb97a98fc6f670f16351
      for (SMServiceList::iterator its = m_smservices.begin(); its != m_smservices.end(); ++its)
        if ((*its)->GetId() == sid /*&& (*its)->GetAccount()->GetSerialNum() == sn*/)
          return *its;

      DBG(DBG_WARN, "%s: not found a valid service for this uri (%s)\n", __FUNCTION__, mediaUri.c_str());
    }
  }
  return svc;
}

std::string Player::GetItemIdFromUriMetadata(const DigitalItemPtr& uriMetadata)
{
  if (!uriMetadata)
    return "";
  // the id should be here for many cases
  const std::string& itemId = uriMetadata->GetObjectID();

  if (itemId.compare(0, 2, "Q:") == 0 || itemId.compare(0, 3, "SQ:") == 0)
  {
    const std::string& uri = uriMetadata->GetValue("res");
    URIParser parser(uri);
    if (!parser.Scheme() || !parser.Path())
    {
      DBG(DBG_ERROR, "%s: invalid uri (%s)\n", __FUNCTION__, itemId.c_str());
      return "";
    }

    // check for library item
    if (strcmp(ProtocolTable[Protocol_xFileCifs], parser.Scheme()) == 0)
      return std::string("S://").append(parser.Host()).append("/").append(parser.Path());

    // check for service item
    SMServicePtr service = GetServiceForMedia(uri);
    if (service)
    {
      DigitalItemPtr meta;
      DigitalItemPtr fake(new DigitalItem(DigitalItem::Type_item, DigitalItem::SubType_audioItem));
      std::string path(parser.Path());
      path = path.substr(0, path.find('?')); // remove all from args
      fake->SetObjectID(path.substr(0, path.find_last_of('.'))); // remove mime extension
      SMAPIMetadata::MakeUriMetadata(service, SMAPIMetadata::track, fake, meta);
      return meta->GetObjectID();
    }
  }
  return itemId;
}
