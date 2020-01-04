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

#include "zonesmodel.h"
#include "sonos.h"
#include "player.h"

#include <QSet>
#include <QQmlEngine>

using namespace nosonapp;

namespace nosonapp
{

class ZPRef
{
public:
  ZPRef(Player* _player) : refcount(0), player(_player) { }
  ~ZPRef() { delete player; }
  ZPRef(const ZPRef&) = delete;
  void operator=(const ZPRef&) = delete;
  int refcount;
  Player* player;
};

}

ZoneItem::ZoneItem(ZPRef* ptr)
: m_ptr(ptr)
, m_valid(false)
, m_isGroup(false)
{
  m_id = ptr->player->zoneId();
  m_name = ptr->player->zoneName();
  const SONOS::ZonePtr zone = ptr->player->zone();
  if (zone->size() == 1)
    m_icon = QString::fromUtf8(zone->GetCoordinator()->GetIconName().c_str());
  else
  {
    m_icon = "";
    m_isGroup = true;
  }
  m_shortName = ptr->player->zoneShortName();
  m_coordinatorName = ptr->player->coordinatorName();
  m_valid = true;
}

QVariant ZoneItem::payload() const
{
  QVariant var;
  var.setValue<SONOS::ZonePtr>(m_ptr->player->zone());
  return var;
}

ZonesModel::ZonesModel(QObject* parent)
: QAbstractListModel(parent)
, m_nextPid(1)
{
}

ZonesModel::~ZonesModel()
{
  qDeleteAll(m_data);
  m_data.clear();
  qDeleteAll(m_items);
  m_items.clear();
  qDeleteAll(m_recycleBin);
  m_recycleBin.clear();
  qDeleteAll(m_players);
  m_players.clear();
}

void ZonesModel::addItem(ZoneItem* item)
{
  {
    LockGuard g(m_lock);
    beginInsertRows(QModelIndex(), m_items.count(), m_items.count());
    m_items << item;
    endInsertRows();
  }
  emit countChanged();
}

int ZonesModel::rowCount(const QModelIndex& parent) const
{
  Q_UNUSED(parent);
#ifdef USE_RECURSIVE_MUTEX
  LockGuard g(m_lock);
#endif
  return m_items.count();
}

QVariant ZonesModel::data(const QModelIndex& index, int role) const
{
#ifdef USE_RECURSIVE_MUTEX
  LockGuard g(m_lock);
#endif
  if (index.row() < 0 || index.row() >= m_items.count())
      return QVariant();

  const ZoneItem* item = m_items[index.row()];
  switch (role)
  {
  case PayloadRole:
    return item->payload();
  case IdRole:
    return item->id();
  case NameRole:
    return item->name();
  case IconRole:
    return item->icon();
  case IsGroupRole:
    return item->isGroup();
  case ShortNameRole:
    return item->shortName();
  case CoordinatorNameRole:
    return item->coordinatorName();
  default:
    return QVariant();
  }
}

QHash<int, QByteArray> ZonesModel::roleNames() const
{
  QHash<int, QByteArray> roles;
  roles[PayloadRole] = "payload";
  roles[IdRole] = "id";
  roles[NameRole] = "name";
  roles[IconRole] = "icon";
  roles[IsGroupRole] = "isGroup";
  roles[ShortNameRole] = "shortName";
  roles[CoordinatorNameRole] = "coordinatorName";
  return roles;
}

QVariantMap ZonesModel::get(int row)
{
  LockGuard g(m_lock);
  if (row < 0 || row >= m_items.count())
    return QVariantMap();
  const ZoneItem* item = m_items[row];
  QVariantMap model;
  QHash<int, QByteArray> roles = roleNames();
  model[roles[PayloadRole]] = item->payload();
  model[roles[IdRole]] = item->id();
  model[roles[NameRole]] = item->name();
  model[roles[IconRole]] = item->icon();
  model[roles[IsGroupRole]] = item->isGroup();
  model[roles[ShortNameRole]] = item->shortName();
  model[roles[CoordinatorNameRole]] = item->coordinatorName();
  return model;
}

Player* ZonesModel::holdPlayer(int row)
{
  LockGuard g(m_lock);
  if (row < 0 || row >= m_items.count())
    return nullptr;
  ZPRef* ref = m_items[row]->ref();
  ref->refcount++;
  return ref->player;
}

void ZonesModel::releasePlayer(Player* player)
{
  LockGuard g(m_lock);
  PlayerMap::iterator rp = m_recycleBin.find(player->zoneName());
  if (rp != m_recycleBin.end())
  {
    if (rp.value()->refcount > 1)
      rp.value()->refcount--;
    else
    {
      qDebug("destroy player %d", rp.value()->player->pid());
      delete rp.value();
      m_recycleBin.erase(rp);
    }
  }
  else
  {
    PlayerMap::iterator mp = m_players.find(player->zoneName());
    if (mp != m_players.end())
    {
      mp.value()->refcount--;
    }
  }
}

void ZonesModel::clearData()
{
  LockGuard g(m_lock);
  qDeleteAll(m_data);
  m_data.clear();
}

bool ZonesModel::loadData()
{
  setUpdateSignaled(false);

  if (!m_provider)
  {
    emit loaded(false);
    return false;
  }

  LockGuard g(m_lock);
  qDeleteAll(m_data);
  m_data.clear();
  m_dataState = DataStatus::DataNotFound;
  SONOS::ZoneList zones = m_provider->getSystem().GetZoneList();

  QSet<QString> keep;
  QList<SONOS::ZonePtr> create;
  QList<PlayerMap::iterator> drop;
  for (SONOS::ZoneList::iterator it = zones.begin(); it != zones.end(); ++it)
  {
    QString name = QString::fromUtf8(it->second->GetZoneName().c_str());
    PlayerMap::iterator mp = m_players.find(name);
    if (mp == m_players.end())
      create.append(it->second);
    else
    {
      qDebug("keep player %d [%s]", mp.value()->player->pid(), it->second->GetZoneName().c_str());
      keep.insert(name);
    }
  }
  for (PlayerMap::iterator mp = m_players.begin(); mp != m_players.end(); ++mp)
  {
    if (keep.find(mp.key()) == keep.end())
      drop.append(mp);
  }
  // detach deleted players from the pool
  for (PlayerMap::iterator& mp : drop)
  {
    ZPRef* r = mp.value();
    m_players.erase(mp);
    QObject::disconnect(r->player, SIGNAL(connectedChanged(int)), this, SIGNAL(zpConnectedChanged(int)));
    QObject::disconnect(r->player, SIGNAL(renderingChanged(int)), this, SIGNAL(zpRenderingChanged(int)));
    QObject::disconnect(r->player, SIGNAL(renderingGroupChanged(int)), this, SIGNAL(zpRenderingGroupChanged(int)));
    QObject::disconnect(r->player, SIGNAL(renderingCountChanged(int)), this, SIGNAL(zpRenderingCountChanged(int)));
    QObject::disconnect(r->player, SIGNAL(sourceChanged(int)), this, SIGNAL(zpSourceChanged(int)));
    QObject::disconnect(r->player, SIGNAL(playbackStateChanged(int)), this, SIGNAL(zpPlaybackStateChanged(int)));
    QObject::disconnect(r->player, SIGNAL(playModeChanged(int)), this, SIGNAL(zpPlayModeChanged(int)));
    QObject::disconnect(r->player, SIGNAL(sleepTimerChanged(int)), this, SIGNAL(zpSleepTimerChanged(int)));
    if (r->refcount > 0)
      m_recycleBin.insert(r->player->zoneName(), r);
    else
    {
      qDebug("destroy player %d", r->player->pid());
      delete r;
    }
  }
  // add new players in the pool
  for (SONOS::ZonePtr& zone : create)
  {
    Player* p = new Player();
    QQmlEngine::setObjectOwnership(p, QQmlEngine::CppOwnership);
    p->init(m_provider, zone);
    m_players.insert(p->zoneName(), new ZPRef(p));
    p->setPid(m_nextPid++);
    QObject::connect(p, SIGNAL(connectedChanged(int)), this, SIGNAL(zpConnectedChanged(int)));
    QObject::connect(p, SIGNAL(renderingChanged(int)), this, SIGNAL(zpRenderingChanged(int)));
    QObject::connect(p, SIGNAL(renderingGroupChanged(int)), this, SIGNAL(zpRenderingGroupChanged(int)));
    QObject::connect(p, SIGNAL(renderingCountChanged(int)), this, SIGNAL(zpRenderingCountChanged(int)));
    QObject::connect(p, SIGNAL(sourceChanged(int)), this, SIGNAL(zpSourceChanged(int)));
    QObject::connect(p, SIGNAL(playbackStateChanged(int)), this, SIGNAL(zpPlaybackStateChanged(int)));
    QObject::connect(p, SIGNAL(playModeChanged(int)), this, SIGNAL(zpPlayModeChanged(int)));
    QObject::connect(p, SIGNAL(sleepTimerChanged(int)), this, SIGNAL(zpSleepTimerChanged(int)));
    qDebug("create player %d [%s]", p->pid(), zone->GetZoneName().c_str());
  }

  for (ZPRef*& r : m_players)
  {
    ZoneItem* item = new ZoneItem(r);
    if (item->isValid())
      m_data << item;
    else
      delete item;
  }
  m_dataState = DataStatus::DataLoaded;
  emit loaded(true);
  return true;
}

bool ZonesModel::asyncLoad()
{
  if (m_provider)
  {
    m_provider->runContentLoader(this);
    return true;
  }
  return false;
}

void ZonesModel::resetModel()
{
  {
    LockGuard g(m_lock);
    if (m_dataState != DataStatus::DataLoaded)
      return;
    beginResetModel();
    if (m_items.count() > 0)
    {
      beginRemoveRows(QModelIndex(), 0, m_items.count()-1);
      qDeleteAll(m_items);
      m_items.clear();
      endRemoveRows();
    }
    if (m_data.count() > 0)
    {
      beginInsertRows(QModelIndex(), 0, m_data.count()-1);
      foreach (ZoneItem* item, m_data)
          m_items << item;
      m_data.clear();
      endInsertRows();
    }
    m_dataState = DataStatus::DataSynced;
    endResetModel();
  }
  emit countChanged();
}

void ZonesModel::handleDataUpdate()
{
  if (!updateSignaled())
  {
    setUpdateSignaled(true);
    dataUpdated();
    }
}
