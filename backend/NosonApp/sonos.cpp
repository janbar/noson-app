/*
 *      Copyright (C) 2015-2019 Jean-Luc Barriere
 *
 *  This file is part of Noson-App
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
 *  along with Noson.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "sonos.h"
#include "listmodel.h"
#include "alarmsmodel.h"

#include <noson/requestbroker.h>
#include <noson/imageservice.h>
#include <noson/filestreamer.h>
#ifdef HAVE_PULSEAUDIO
#include <noson/pulsestreamer.h>
#endif

#include <QString>
#include <QDebug>

#define THREAD_EXPIRY_TIMEOUT  10000
#define DEFAULT_MAX_THREAD     16

using namespace nosonapp;

Sonos::Sonos(QObject* parent)
: QObject(parent)
, m_library(ManagedContents())
, m_shareUpdateID(0)
, m_shareIndexInProgess(false)
, m_savedQueuesUpdateID(0)
, m_system(this, systemEventCB)
, m_workerPool()
, m_jobCount(LockedNumber<int>(0))
, m_locale("en_US")
{
  SONOS::System::Debug(2);
  // Set the local URI of my http listener
  m_systemLocalURI = QString::fromUtf8(m_system.GetSystemLocalUri().c_str());
  // Register handlers to process remote request
  SONOS::RequestBrokerPtr imageService(new SONOS::ImageService());
  m_system.RegisterRequestBroker(imageService);
#ifdef HAVE_PULSEAUDIO
  m_system.RegisterRequestBroker(SONOS::RequestBrokerPtr(new SONOS::PulseStreamer(imageService.get())));
#endif
  m_system.RegisterRequestBroker(SONOS::RequestBrokerPtr(new SONOS::FileStreamer()));

  m_workerPool.setExpiryTimeout(THREAD_EXPIRY_TIMEOUT);
  m_workerPool.setMaxThreadCount(DEFAULT_MAX_THREAD);
}

Sonos::~Sonos()
{
  {
    Locked<ManagedContents>::pointer left = m_library.Get();
    for (ManagedContents::iterator it = left->begin(); it != left->end(); ++it)
    {
      LockGuard g(it->model->m_lock);
      unregisterContent(it->model);
    }
  }
  m_workerPool.clear();
}

void Sonos::debug(int debug)
{
  m_system.Debug(debug);
}

Future* Sonos::tryInit(int debug)
{
  return new Future(new PromiseInit(*this, debug), this);
}

Future* Sonos::tryRenewSubscriptions()
{
  return new Future(new PromiseRenewSubscriptions(*this), this);
}

Future* Sonos::tryJoinZones(const QVariantList& zonePayloads, const QVariant& toZonePayload)
{
  return new Future(new PromiseJoinZones(*this, zonePayloads, toZonePayload), this);
}

Future* Sonos::tryUnjoinZone(const QVariant& zonePayload)
{
  return new Future(new PromiseUnjoinZone(*this, zonePayload), this);
}

Future* Sonos::tryUnjoinRooms(const QVariantList& roomPayloads)
{
  return new Future(new PromiseUnjoinRooms(*this, roomPayloads), this);
}

Future* Sonos::tryCreateAlarm(const QVariant& alarmPayload)
{
  return new Future(new PromiseCreateAlarm(*this, alarmPayload), this);
}

Future* Sonos::tryUpdateAlarm(const QVariant& alarmPayload)
{
  return new Future(new PromiseUpdateAlarm(*this, alarmPayload), this);
}

Future* Sonos::tryDestroyAlarm(const QString& id)
{
  return new Future(new PromiseDestroyAlarm(*this, id), this);
}

Future* Sonos::tryRefreshShareIndex()
{
  return new Future(new PromiseRefreshShareIndex(*this), this);
}

Future* Sonos::tryDestroySavedQueue(const QString& SQid)
{
  return new Future(new PromiseDestroySavedQueue(*this, SQid), this);
}

Future* Sonos::tryAddItemToFavorites(const QVariant& payload, const QString& description, const QString& artURI)
{
  return new Future(new PromiseAddItemToFavorites(*this, payload, description, artURI), this);
}

Future* Sonos::tryDestroyFavorite(const QString& FVid)
{
  return new Future(new PromiseDestroyFavorite(*this, FVid), this);
}

bool Sonos::init(int debug /*= 0*/)
{
  SONOS::System::Debug(2);
  bool ret = m_system.Discover();
  SONOS::System::Debug(debug);
  emit initDone(ret);
  return ret;
}

bool Sonos::init(int debug, const QString& url)
{
  SONOS::System::Debug(2);
  bool ret = m_system.Discover(url.toStdString().c_str());
  SONOS::System::Debug(debug);
  emit initDone(ret);
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

QVariantList Sonos::getZones()
{
  ZonesModel model;
  model.init(this, true);
  model.resetModel();
  QVariantList list;
  for (int r = 0; r < model.rowCount(); ++r)
    list.append(model.get(r));
  return list;
}

bool Sonos::isConnected()
{
  return m_system.IsConnected();
}

QVariantList Sonos::getZoneRooms(const QString& zoneId)
{
  RoomsModel model;
  model.load(this, zoneId);
  QVariantList list;
  for (int r = 0; r < model.rowCount(); ++r)
    list.append(model.get(r));
  return list;
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

bool Sonos::refreshShareIndex()
{
  return m_system.RefreshShareIndex();
}

bool Sonos::destroySavedQueue(const QString& SQid)
{
  return m_system.DestroySavedQueue(SQid.toUtf8().constData());
}

bool Sonos::addItemToFavorites(const QVariant& payload, const QString& description, const QString& artURI)
{
  return m_system.AddURIToFavorites(payload.value<SONOS::DigitalItemPtr>(), description.toUtf8().constData(), artURI.toUtf8().constData());
}

bool Sonos::destroyFavorite(const QString& FVid)
{
  return m_system.DestroyFavorite(FVid.toUtf8().constData());
}

QString Sonos::getObjectIDFromUriMetadata(const QVariant& itemPayload)
{
  SONOS::DigitalItemPtr ptr = itemPayload.value<SONOS::DigitalItemPtr>();
  return QString::fromUtf8(m_system.GetObjectIDFromUriMetadata(ptr).c_str());
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

SONOS::ZonePtr Sonos::findZone(const QString& zoneName)
{
  std::string name = zoneName.toUtf8().constData();
  SONOS::ZoneList zones = m_system.GetZoneList();
  if (zones.empty())
    return SONOS::ZonePtr();
  // loop in zones
  for (SONOS::ZoneList::const_iterator it = zones.begin(); it != zones.end(); ++it)
  {
    if (name.empty() || name == it->second->GetZoneName())
      return it->second;
    // loop in group to search the player with the given name
    for (std::vector<SONOS::ZonePlayerPtr>::const_iterator itp = it->second->begin(); itp != it->second->end(); ++itp)
    {
      if (name == **itp)
        return it->second;
    }
  }
  return zones.begin()->second;
}

void Sonos::runLoader()
{
  m_workerPool.start(new ContentLoader<Sonos>(*this));
}

void Sonos::beforeLoad()
{
  beginJob();
}

void Sonos::afterLoad()
{
  endJob();
}

void Sonos::runContentLoader(ListModel<Sonos>* model)
{
  if (model && !model->m_pending)
  {
    model->m_pending = true; // decline next request
    m_workerPool.start(new ContentLoader<Sonos>(*this, model));
  }
  else
    qWarning("%s: request has been declined (%p)", __FUNCTION__, model);
}

void Sonos::loadContent(ListModel<Sonos>* model)
{
  Locked<ManagedContents>::pointer mc = m_library.Get();
  for (ManagedContents::iterator it = mc->begin(); it != mc->end(); ++it)
    if (it->model == model)
    {
      qDebug("%s: %p (%s)", __FUNCTION__, model, model->m_root.toUtf8().constData());
      emit loadingStarted();
      model->m_pending = false; // accept add next request in queue
      model->loadData();
      emit loadingFinished();
      break;
    }
}

void Sonos::loadAllContent()
{
  QList<ListModel<Sonos>*> left;
  {
    Locked<ManagedContents>::pointer mc = m_library.Get();
    for (ManagedContents::iterator it = mc->begin(); it != mc->end(); ++it)
      if (it->model->m_dataState == DataStatus::DataNotFound)
        left.push_back(it->model);
  }
  emit loadingStarted();
  if (!left.empty())
  {
    while (!left.isEmpty())
    {
      ListModel<Sonos>* model = left.front();
      model->loadData();
      left.pop_front();
    }
  }
  emit loadingFinished();
}

void Sonos::runContentLoaderForContext(ListModel<Sonos>* model, int id)
{
  if (model && !model->m_pending)
  {
    model->m_pending = true; // decline next request
    m_workerPool.start(new ContentForContextLoader<Sonos>(*this, model, id));
  }
  else
    qWarning("%s: request id %d has been declined (%p)", __FUNCTION__, id, model);
}

void Sonos::loadContentForContext(ListModel<Sonos>* model, int id)
{
  model->m_pending = false; // accept add next request in queue
  model->loadDataForContext(id);
}

const char* Sonos::getHost() const
{
  return m_system.GetHost().c_str();
}

unsigned Sonos::getPort() const
{
  return m_system.GetPort();
}

QString Sonos::getBaseUrl() const
{
  QString port;
  port.setNum(m_system.GetPort());
  QString url = "http://";
  url.append(m_system.GetHost().c_str()).append(":").append(port);
  return url;
}

void Sonos::registerContent(ListModel<Sonos>* model, const QString& root)
{
  if (model)
  {
    qDebug("%s: %p (%s)", __FUNCTION__, model, root.toUtf8().constData());
    Locked<ManagedContents>::pointer mc = m_library.Get();
    for (ManagedContents::iterator it = mc->begin(); it != mc->end(); ++it)
    {
      if (it->model == model)
      {
        it->root = root;
        return;
      }
    }
    mc->append(RegisteredContent<Sonos>(model, root));
  }
}

void Sonos::unregisterContent(ListModel<Sonos>* model)
{
  if (model)
  {
    QList<ManagedContents::iterator> left;
    Locked<ManagedContents>::pointer mc = m_library.Get();
    for (ManagedContents::iterator it = mc->begin(); it != mc->end(); ++it)
      if (it->model == model)
        left.push_back(it);
    for (QList<ManagedContents::iterator>::iterator itl = left.begin(); itl != left.end(); ++itl)
    {
      qDebug("%s: %p (%s)", __FUNCTION__, model, model->m_root.toUtf8().constData());
      model->m_provider = nullptr;
      mc->erase(*itl);
    }
  }
}

bool Sonos::startJob(QRunnable* worker)
{
  return m_workerPool.tryStart(worker);
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

void Sonos::systemEventCB(void *handle)
{
  Sonos* sonos = static_cast<Sonos*>(handle);
  Q_ASSERT(sonos);
  // Read last event flags
  unsigned char events = sonos->getSystem().LastEvents();

  if ((events & SONOS::SVCEvent_ZGTopologyChanged))
    emit sonos->topologyChanged();
  if ((events & SONOS::SVCEvent_AlarmClockChanged))
    emit sonos->alarmClockChanged();
  if ((events & SONOS::SVCEvent_ContentDirectoryChanged))
  {
    Locked<ManagedContents>::pointer cl = sonos->m_library.Get();
    SONOS::ContentProperty prop = sonos->getSystem().GetContentProperty();
    for (std::vector<std::pair<std::string, unsigned> >::const_iterator uit = prop.ContainerUpdateIDs.begin(); uit != prop.ContainerUpdateIDs.end(); ++uit)
    {
      qDebug("%s: container [%s] has being updated to %u", __FUNCTION__, uit->first.c_str(), uit->second);

      bool shareUpdated = false;
      bool savedQueuesUpdated = false;

      // Reload musical index on any update of shares
      if (uit->first == "S:" && uit->second != sonos->m_shareUpdateID)
      {
        shareUpdated = true;
        sonos->m_shareUpdateID = uit->second; // Track current updateID
      }
      // Reload saved queues on any update
      else if (uit->first == "SQ:" && uit->second != sonos->m_savedQueuesUpdateID)
      {
        savedQueuesUpdated = true;
        sonos->m_savedQueuesUpdateID = uit->second; // Track current updateID
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
        else if (savedQueuesUpdated && _base.startsWith(QString::fromUtf8("SQ:")))
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

///////////////////////////////////////////////////////////////////////////////
///
/// About promises

void Sonos::PromiseInit::run()
{
  bool r = m_sonos.init(m_debug);
  setResult(QVariant(r));
}

void Sonos::PromiseRenewSubscriptions::run()
{
  m_sonos.renewSubscriptions();
  setResult(QVariant(true));
}

void Sonos::PromiseJoinZones::run()
{
  bool r = m_sonos.joinZones(m_zonePayloads, m_toZonePayload);
  setResult(QVariant(r));
}

void Sonos::PromiseUnjoinZone::run()
{
  bool r = m_sonos.unjoinZone(m_zonePayload);
  setResult(QVariant(r));
}

void Sonos::PromiseUnjoinRooms::run()
{
  bool r = m_sonos.unjoinRooms(m_roomPayloads);
  setResult(QVariant(r));
}

void Sonos::PromiseCreateAlarm::run()
{
  bool r = m_sonos.createAlarm(m_alarmPayload);
  setResult(QVariant(r));
}

void Sonos::PromiseUpdateAlarm::run()
{
  bool r = m_sonos.updateAlarm(m_alarmPayload);
  setResult(QVariant(r));
}

void Sonos::PromiseDestroyAlarm::run()
{
  bool r = m_sonos.destroyAlarm(m_id);
  setResult(QVariant(r));
}

void Sonos::PromiseRefreshShareIndex::run()
{
  bool r = m_sonos.refreshShareIndex();
  setResult(QVariant(r));
}

void Sonos::PromiseDestroySavedQueue::run()
{
  bool r = m_sonos.destroySavedQueue(m_SQid);
  setResult(QVariant(r));
}

void Sonos::PromiseAddItemToFavorites::run()
{
  bool r = m_sonos.addItemToFavorites(m_payload, m_description, m_artURI);
  setResult(QVariant(r));
}

void Sonos::PromiseDestroyFavorite::run()
{
  bool r = m_sonos.destroyFavorite(m_FVid);
  setResult(QVariant(r));
}
