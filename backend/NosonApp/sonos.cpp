/*
 *      Copyright (C) 2015-2016 Jean-Luc Barriere
 *
 *  This file is part of Noson-App
 *
 *  Noson-App is free software: you can redistribute it and/or modify
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

#include "sonos.h"
#include "private/debug.h"
#include <noson/contentdirectory.h>
#include "listmodel.h"
#include "alarmsmodel.h"

#include <QString>

#define JOB_THREADPOOL_SIZE 16

class ContentLoader : public SONOS::OS::CWorker
{
public:
  ContentLoader(Sonos& sonos, ListModel* payload)
  : m_sonos(sonos)
  , m_payload(payload) { }

  ContentLoader(Sonos& sonos) : m_sonos(sonos), m_payload(0) { }

  virtual void Process()
  {
    m_sonos.beginJob();
    if (m_payload)
      m_sonos.loadModel(m_payload);
    else
      m_sonos.loadEmptyModels();
    m_sonos.endJob();
  }
private:
  Sonos& m_sonos;
  ListModel* m_payload;
};

class CustomizedContentLoader : public SONOS::OS::CWorker
{
public:
  CustomizedContentLoader(Sonos& sonos, ListModel* payload, int id)
  : m_sonos(sonos)
  , m_payload(payload)
  , m_id(id) { }

  virtual void Process()
  {
    m_sonos.beginJob();
    if (m_payload)
      m_sonos.customizedLoadModel(m_payload, m_id);
    m_sonos.endJob();
  }
private:
  Sonos& m_sonos;
  ListModel* m_payload;
  int m_id;
};

Sonos::Sonos(QObject* parent)
: QObject(parent)
, m_library(ManagedContents())
, m_shareUpdateID(0)
, m_shareIndexInProgess(false)
, m_system(this, systemEventCB)
, m_threadpool(JOB_THREADPOOL_SIZE)
, m_jobCount(SONOS::LockedNumber<int>(0))
, m_locale("en_US")
{
  SONOS::DBGLevel(2);
}

Sonos::~Sonos()
{
  {
    ManagedContents left = m_library.Load();
    for (ManagedContents::iterator it = left.begin(); it != left.end(); ++it)
      unregisterModel(it->model);
  }
}

class InitWorker : public SONOS::OS::CWorker
{
public:
  InitWorker(Sonos& sonos, int debug) : m_sonos(sonos), m_debug(debug) { }

  virtual void Process()
  {
    m_sonos.beginJob();
    emit m_sonos.initDone(m_sonos.init(m_debug));
    m_sonos.endJob();
  }
private:
  Sonos& m_sonos;
  int m_debug;
};

bool Sonos::startInit(int debug)
{
  return m_threadpool.Enqueue(new InitWorker(*this, debug));
}

bool Sonos::init(int debug)
{
  SONOS::DBGLevel(debug > DBG_INFO ? debug : DBG_INFO);
  bool ret = m_system.Discover();
  SONOS::DBGLevel(debug);
  return ret;
}

void Sonos::setLocale(const QString& locale)
{
  m_locale.Store(locale);
}

QString Sonos::getLocale()
{
  return m_locale.Load();
}

QString Sonos::getLibVersion()
{
  return QString(LIBVERSION);
}

void Sonos::addServiceOAuth(const QString& type, const QString& sn, const QString& key, const QString& token, const QString& username)
{
  SONOS::System::AddServiceOAuth(type.toUtf8().constData(), sn.toUtf8().constData(), key.toUtf8().constData(), token.toUtf8().constData(), username.toUtf8().constData());
}

void Sonos::deleteServiceOAuth(const QString& type, const QString& sn)
{
  SONOS::System::DeleteServiceOAuth(type.toUtf8().constData(), sn.toUtf8().constData());
}

void Sonos::renewSubscriptions()
{
  m_system.RenewSubscriptions();
}

ZonesModel* Sonos::getZones()
{
  ZonesModel* model = new ZonesModel();
  model->init(this, true);
  return model;
}

bool Sonos::connectZone(const QString& zoneName)
{
  std::string name;
  SONOS::ZoneList zones = m_system.GetZoneList();
  // default the name by current coordinator
  if (zoneName.isEmpty() && m_system.IsConnected())
    name.append(*(m_system.GetConnectedZone()->GetCoordinator()));
  else
    name.append(zoneName.toUtf8().constData());

  // loop in zones
  for (SONOS::ZoneList::const_iterator it = zones.begin(); it != zones.end(); ++it)
  {
    if (name.empty() || name == it->second->GetZoneName())
    {
      return m_system.ConnectZone(it->second, this, playerEventCB);
    }
    // loop in group to search the player with the given name
    if (it->second->size() > 1)
    {
      for (std::vector<SONOS::ZonePlayerPtr>::const_iterator itp = it->second->begin(); itp != it->second->end(); ++itp)
      {
        if (name == **itp)
        {
          return m_system.ConnectZone(it->second, this, playerEventCB);
        }
      }
    }
  }
  return false;
}

QString Sonos::getZoneId() const
{
  if (m_system.IsConnected())
    return m_system.GetConnectedZone()->GetGroup().c_str();
  return "";
}

QString Sonos::getZoneName() const
{
  if (m_system.IsConnected())
    return m_system.GetConnectedZone()->GetZoneName().c_str();
  return "";
}

QString Sonos::getZoneShortName() const
{
  if (m_system.IsConnected())
    return m_system.GetConnectedZone()->GetZoneShortName().c_str();
  return "";
}

RoomsModel* Sonos::getZoneRooms()
{
  RoomsModel* model = new RoomsModel();
  model->load(this, getZoneId());
  return model;
}

bool Sonos::joinRoom(const QVariant& roomPayload, const QVariant& toZonePayload)
{
  SONOS::ZonePlayerPtr room = roomPayload.value<SONOS::ZonePlayerPtr>();
  SONOS::ZonePtr zone = toZonePayload.value<SONOS::ZonePtr>();
  if (room && room->IsValid() && zone && zone->GetCoordinator())
  {
    SONOS::Player player(room);
    return player.JoinToGroup(zone->GetCoordinator()->GetUUID());
  }
  return false;
}

bool Sonos::joinZone(const QVariant& zonePayload, const QVariant& toZonePayload)
{
  SONOS::ZonePtr zone = zonePayload.value<SONOS::ZonePtr>();
  SONOS::ZonePtr toZone = toZonePayload.value<SONOS::ZonePtr>();
  if (zone && toZone && toZone->GetCoordinator())
  {
    for (std::vector<SONOS::ZonePlayerPtr>::iterator it = zone->begin(); it != zone->end(); ++it)
    {
      SONOS::Player player(*it);
      player.JoinToGroup(toZone->GetCoordinator()->GetUUID());
    }
    return true;
  }
  return false;

}

bool Sonos::joinZones(const QVariantList& zonePayloads, const QVariant& toZonePayload)
{
  std::vector<SONOS::ZonePtr> zones;
  SONOS::ZonePtr toZone = toZonePayload.value<SONOS::ZonePtr>();
  for (QVariantList::const_iterator it = zonePayloads.begin(); it != zonePayloads.end(); ++it)
    zones.push_back(it->value<SONOS::ZonePtr>());
  if (toZone && toZone->GetCoordinator())
  {
    for (std::vector<SONOS::ZonePtr>::const_iterator it = zones.begin(); it != zones.end(); ++it)
    {
      if ((*it)->GetZoneName() == toZone->GetZoneName())
        continue;
      for (std::vector<SONOS::ZonePlayerPtr>::iterator itr = (*it)->begin(); itr != (*it)->end(); ++itr)
      {
        SONOS::Player player(*itr);
        player.JoinToGroup(toZone->GetCoordinator()->GetUUID());
      }
    }
    return true;
  }
  return false;
}

class JoinZonesWorker : public SONOS::OS::CWorker
{
public:
  JoinZonesWorker(Sonos& sonos, const QVariantList& zonePayloads, const QVariant& toZonePayload)
  : m_sonos(sonos)
  , m_zonePayloads(zonePayloads)
  , m_toZonePayload(toZonePayload)
  { }

  virtual void Process()
  {
    m_sonos.beginJob();
    m_sonos.joinZones(m_zonePayloads, m_toZonePayload);
    m_sonos.endJob();
  }
private:
  Sonos& m_sonos;
  QVariantList m_zonePayloads;
  QVariant m_toZonePayload;
};

bool Sonos::startJoinZones(const QVariantList& zonePayloads, const QVariant& toZonePayload)
{
  return m_threadpool.Enqueue(new JoinZonesWorker(*this, zonePayloads, toZonePayload));
}

bool Sonos::unjoinRoom(const QVariant& roomPayload)
{
  SONOS::ZonePlayerPtr room = roomPayload.value<SONOS::ZonePlayerPtr>();
  if (room && room->IsValid())
  {
    SONOS::Player player(room);
    return player.BecomeStandalone();
  }
  return false;
}

bool Sonos::unjoinRooms(const QVariantList& roomPayloads)
{
  for (QVariantList::const_iterator it = roomPayloads.begin(); it != roomPayloads.end(); ++it) {
    SONOS::ZonePlayerPtr room = it->value<SONOS::ZonePlayerPtr>();
    if (room && room->IsValid())
    {
      SONOS::Player player(room);
      return player.BecomeStandalone();
    }
    else
      return false;
  }
  return true;
}

class UnjoinRoomsWorker : public SONOS::OS::CWorker
{
public:
  UnjoinRoomsWorker(Sonos& sonos, const QVariantList& roomPayloads)
  : m_sonos(sonos)
  , m_roomPayloads(roomPayloads)
  { }

  virtual void Process()
  {
    m_sonos.beginJob();
    m_sonos.unjoinRooms(m_roomPayloads);
    m_sonos.endJob();
  }
private:
  Sonos& m_sonos;
  QVariantList m_roomPayloads;
};

bool Sonos::startUnjoinRooms(const QVariantList& roomPayloads)
{
  return m_threadpool.Enqueue(new UnjoinRoomsWorker(*this, roomPayloads));
}

bool Sonos::unjoinZone(const QVariant& zonePayload)
{
  SONOS::ZonePtr zone = zonePayload.value<SONOS::ZonePtr>();
  if (zone)
  {
    for (std::vector<SONOS::ZonePlayerPtr>::iterator it = zone->begin(); it != zone->end(); ++it)
    {
      SONOS::Player player(*it);
      player.BecomeStandalone();
    }
    return true;
  }
  return false;

}

class UnjoinZoneWorker : public SONOS::OS::CWorker
{
public:
  UnjoinZoneWorker(Sonos& sonos, const QVariant& zonePayload)
  : m_sonos(sonos)
  , m_zonePayload(zonePayload)
  { }

  virtual void Process()
  {
    m_sonos.beginJob();
    m_sonos.unjoinZone(m_zonePayload);
    m_sonos.endJob();
  }
private:
  Sonos& m_sonos;
  QVariant m_zonePayload;
};

bool Sonos::startUnjoinZone(const QVariant& zonePayload)
{
  return m_threadpool.Enqueue(new UnjoinZoneWorker(*this, zonePayload));
}

bool Sonos::createAlarm(const QVariant& alarmPayload)
{
  SONOS::AlarmPtr ptr = alarmPayload.value<SONOS::AlarmPtr>();
  if (ptr && m_system.CreateAlarm(*ptr))
  {
    return true;
  }
  return false;
}

bool Sonos::updateAlarm(const QVariant& alarmPayload)
{
  SONOS::AlarmPtr ptr = alarmPayload.value<SONOS::AlarmPtr>();
  if (ptr && m_system.UpdateAlarm(*ptr))
  {
    return true;
  }
  return false;
}

bool Sonos::destroyAlarm(const QString& id)
{
  return m_system.DestroyAlarm(id.toUtf8().constData());
}

bool Sonos::isItemFromService(const QVariant &itemPayload)
{
  SONOS::DigitalItemPtr ptr = itemPayload.value<SONOS::DigitalItemPtr>();
  if (ptr && m_system.IsItemFromService(ptr))
  {
    return true;
  }
  return false;
}

SONOS::System &Sonos::getSystem()
{
  return m_system;
}

const SONOS::PlayerPtr& Sonos::getPlayer() const
{
  return m_system.GetPlayer();
}

void Sonos::runLoader()
{
  m_threadpool.Enqueue(new ContentLoader(*this));
}

void Sonos::loadEmptyModels()
{
  QList<QPair<ListModel*, SONOS::LockGuard> > left;
  {
    SONOS::Locked<ManagedContents>::pointer mc = m_library.Get();
    for (ManagedContents::iterator it = mc->begin(); it != mc->end(); ++it)
      if (it->model->m_dataState == ListModel::NoData)
        left.push_back(qMakePair(it->model, SONOS::LockGuard(it->model->m_lock)));
  }
  emit loadingStarted();
  if (!left.empty())
  {
    while (!left.isEmpty())
    {
      QPair<ListModel*, SONOS::LockGuard> item = left.front();
      item.first->loadData();
      left.pop_front();
    }
  }
  emit loadingFinished();
}

void Sonos::runModelLoader(ListModel* model)
{
  if (model && !model->m_pending)
  {
    model->m_pending = true; // decline next request
    m_threadpool.Enqueue(new ContentLoader(*this, model));
  }
  else
    SONOS::DBG(DBG_ERROR, "%s: request has been declined (%p)\n", __FUNCTION__, model);
}

void Sonos::loadModel(ListModel* model)
{
  QPair<ListModel*, SONOS::LockGuard> item(0, SONOS::LockGuard());
  {
    SONOS::Locked<ManagedContents>::pointer mc = m_library.Get();
    for (ManagedContents::iterator it = mc->begin(); it != mc->end(); ++it)
      if (it->model == model)
      {
        item.first = it->model;
        item.second = SONOS::LockGuard(it->model->m_lock);
        break;
      }
  }
  if (item.first)
  {
    SONOS::DBG(DBG_DEBUG, "%s: %p (%s)\n", __FUNCTION__, item.first, item.first->m_root.toUtf8().constData());
    emit loadingStarted();
    item.first->m_pending = false; // accept add next request in queue
    item.first->loadData();
    emit loadingFinished();
  }
}

void Sonos::runCustomizedModelLoader(ListModel* model, int id)
{
  if (model && !model->m_pending)
  {
    model->m_pending = true; // decline next request
    m_threadpool.Enqueue(new CustomizedContentLoader(*this, model, id));
  }
  else
    SONOS::DBG(DBG_ERROR, "%s: request id %d has been declined (%p)\n", __FUNCTION__, id, model);
}

void Sonos::customizedLoadModel(ListModel *model, int id)
{
  SONOS::LockGuard guard(model->m_lock);
  model->m_pending = false; // accept add next request in queue
  model->customizedLoad(id);
}

void Sonos::registerModel(ListModel* model, const QString& root)
{
  if (model)
  {
    SONOS::DBG(DBG_DEBUG, "%s: %p (%s)\n", __FUNCTION__, model, root.toUtf8().constData());
    SONOS::LockGuard lock(model->m_lock);
    SONOS::Locked<ManagedContents>::pointer mc = m_library.Get();
    for (ManagedContents::iterator it = mc->begin(); it != mc->end(); ++it)
    {
      if (it->model == model)
      {
        it->root = root;
        return;
      }
    }
    mc->append(RegisteredContent(model, root));
  }
}

void Sonos::unregisterModel(ListModel* model)
{
  if (model)
  {
    SONOS::LockGuard lock(model->m_lock);
    QList<ManagedContents::iterator> left;
    SONOS::Locked<ManagedContents>::pointer mc = m_library.Get();
    for (ManagedContents::iterator it = mc->begin(); it != mc->end(); ++it)
      if (it->model == model)
        left.push_back(it);
    for (QList<ManagedContents::iterator>::iterator itl = left.begin(); itl != left.end(); ++itl)
    {
      SONOS::DBG(DBG_DEBUG, "%s: %p (%s)\n", __FUNCTION__, model, model->m_root.toUtf8().constData());
      model->m_provider = 0;
      model->m_root.clear();
      mc->erase(*itl);
    }
  }
}

bool Sonos::startJob(SONOS::OS::CWorker* worker)
{
  return m_threadpool.Enqueue(worker);
}

void Sonos::beginJob()
{
  m_jobCount.Add(1);
  emit jobCountChanged();
}

void Sonos::endJob()
{
  m_jobCount.Add(-1);
  emit jobCountChanged();
}

void Sonos::playerEventCB(void* handle)
{
  Sonos* sonos = static_cast<Sonos*>(handle);
  SONOS::PlayerPtr player = sonos->getPlayer();
  if (player)
  {
    // Read last event flags
    unsigned char events = player->LastEvents();

    if ((events & SONOS::SVCEvent_TransportChanged))
      emit sonos->transportChanged();
    if ((events & SONOS::SVCEvent_RenderingControlChanged))
      emit sonos->renderingControlChanged();
    if ((events & SONOS::SVCEvent_ContentDirectoryChanged))
    {
      SONOS::Locked<ManagedContents>::pointer cl = sonos->m_library.Get();
      SONOS::ContentProperty prop = player->GetContentProperty();
      for (std::vector<std::pair<std::string, unsigned> >::const_iterator uit = prop.ContainerUpdateIDs.begin(); uit != prop.ContainerUpdateIDs.end(); ++uit)
      {
        SONOS::DBG(DBG_DEBUG, "%s: container [%s] has being updated to %u\n", __FUNCTION__, uit->first.c_str(), uit->second);

        // Reload musical index on any update of shares
        bool shareUpdated = false;
        if (uit->first == "S:" && uit->second != sonos->m_shareUpdateID)
        {
          shareUpdated = true;
          sonos->m_shareUpdateID = uit->second; // Track current updateID
        }

        for (ManagedContents::iterator it = cl->begin(); it != cl->end(); ++it)
        {
          // find the base of the model from its root
          QString _base;
          int slash = it->model->m_root.indexOf("/");
          if (slash < 0)
            _base.append(it->model->m_root);
          else
            _base.append(it->model->m_root.left(slash));

          // need update ?
          bool _update = false;
          // same base
          if (it->model->m_updateID != uit->second && _base == uit->first.c_str())
            _update = true;
          // about shares
          else if (shareUpdated && _base.startsWith(QString::fromUtf8("A:")))
            _update = true;

          if (_update)
            it->model->handleDataUpdate();
        }
      }
      // Signal share index events
      if (prop.ShareIndexInProgress != sonos->m_shareIndexInProgess)
      {
        if (prop.ShareIndexInProgress)
          emit sonos->shareIndexInProgress();
        else
          emit sonos->shareIndexFinished();
        sonos->m_shareIndexInProgess = prop.ShareIndexInProgress;
      }
    }
  }
}

void Sonos::systemEventCB(void *handle)
{
  Sonos* sonos = static_cast<Sonos*>(handle);
  // Read last event flags
  unsigned char events = sonos->getSystem().LastEvents();

  if ((events & SONOS::SVCEvent_ZGTopologyChanged))
    emit sonos->topologyChanged();
  if ((events & SONOS::SVCEvent_AlarmClockChanged))
    emit sonos->alarmClockChanged();
}
