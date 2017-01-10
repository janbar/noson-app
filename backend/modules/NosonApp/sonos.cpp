/*
 *      Copyright (C) 2015-2016 Jean-Luc Barriere
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

#include "sonos.h"
#include "../../lib/noson/noson/src/private/debug.h"
#include "../../lib/noson/noson/src/contentdirectory.h"
#include "listmodel.h"

#include <QString>

class ContentLoader : public SONOS::OS::CWorker
{
public:
  ContentLoader(Sonos& sonos, ListModel* payload)
  : m_sonos(sonos)
  , m_payload(payload) { }

  ContentLoader(Sonos& sonos) : m_sonos(sonos), m_payload(0) { }

  virtual void Process()
  {
    if (m_payload)
      m_sonos.loadModel(m_payload);
    else
      m_sonos.loadEmptyModels();
  }
private:
  Sonos& m_sonos;
  ListModel* m_payload;
};

Sonos::Sonos(QObject* parent)
: QObject(parent)
, m_library(ManagedContents())
, m_shareUpdateID(0)
, m_system(this, topologyEventCB)
, m_threadpool(5)
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

bool Sonos::init(int debug)
{
  SONOS::DBGLevel(debug);
  return m_system.Discover();
}

void Sonos::setLocale(const QString& locale)
{
  m_locale.Store(locale);
}

QString Sonos::getLocale()
{
  return m_locale.Load();
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
  SONOS::ZoneList zones = m_system.GetZoneList();
  for (SONOS::ZoneList::const_iterator it = zones.begin(); it != zones.end(); ++it)
  {
    if (zoneName.isEmpty() || zoneName == QString::fromUtf8(it->second->GetZoneName().c_str()))
    {
      return m_system.ConnectZone(it->second, this, playerEventCB);
    }
  }
  return false;
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

const SONOS::System& Sonos::getSystem() const
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
      if (!it->model->m_loaded)
        left.push_back(qMakePair(it->model, SONOS::LockGuard(it->model->m_lock)));
  }
  emit loadingStarted();
  if (!left.empty())
  {
    while (!left.isEmpty())
    {
      QPair<ListModel*, SONOS::LockGuard> item = left.front();
      item.first->load();
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
    SONOS::DBG(DBG_INFO, "%s: %p (%s)\n", __FUNCTION__, item.first, item.first->m_root.toUtf8().constData());
    emit loadingStarted();
    item.first->m_pending = false; // accept add next request in queue
    item.first->load();
    emit loadingFinished();
  }
}

void Sonos::registerModel(ListModel* model, const QString& root)
{
  if (model)
  {
    SONOS::DBG(DBG_INFO, "%s: %p (%s)\n", __FUNCTION__, model, root.toUtf8().constData());
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
      model->m_provider = 0;
      model->m_root.clear();
      mc->erase(*itl);
    }
  }
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
    }
  }
}

void Sonos::topologyEventCB(void *handle)
{
  Sonos* sonos = static_cast<Sonos*>(handle);
  emit sonos->topologyChanged();
}
