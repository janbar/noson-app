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

using namespace nosonapp;

ZoneItem::ZoneItem(const SONOS::ZonePtr& ptr)
: m_ptr(ptr)
, m_valid(false)
, m_isGroup(false)
{
  m_id = QString::fromUtf8(ptr->GetGroup().c_str());
  m_name = QString::fromUtf8(ptr->GetZoneName().c_str());
  if (ptr->size() == 1)
    m_icon = QString::fromUtf8(ptr->GetCoordinator()->GetIconName().c_str());
  else
  {
    m_icon = "";
    m_isGroup = true;
  }
  m_shortName = QString::fromUtf8(ptr->GetZoneShortName().c_str());
  m_valid = true;
}

QVariant ZoneItem::payload() const
{
  QVariant var;
  var.setValue<SONOS::ZonePtr>(m_ptr);
  return var;
}

ZonesModel::ZonesModel(QObject* parent)
: QAbstractListModel(parent)
{
}

ZonesModel::~ZonesModel()
{
  qDeleteAll(m_data);
  m_data.clear();
  qDeleteAll(m_items);
  m_items.clear();
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
  return model;
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
  m_dataState = ListModel::NoData;
  SONOS::ZoneList zones = m_provider->getSystem().GetZoneList();
  for (SONOS::ZoneList::iterator it = zones.begin(); it != zones.end(); ++it)
  {
    ZoneItem* item = new ZoneItem(it->second);
    if (item->isValid())
      m_data << item;
    else
      delete item;
  }
  m_dataState = ListModel::Loaded;
  emit loaded(true);
  return true;
}

bool ZonesModel::asyncLoad()
{
  if (m_provider)
  {
    m_provider->runModelLoader(this);
    return true;
  }
  return false;
}

void ZonesModel::resetModel()
{
  {
    LockGuard g(m_lock);
    if (m_dataState != ListModel::Loaded)
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
    m_dataState = ListModel::Synced;
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
