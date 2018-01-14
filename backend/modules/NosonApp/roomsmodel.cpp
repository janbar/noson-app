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

#include "roomsmodel.h"
#include "sonos.h"

RoomItem::RoomItem(const SONOS::ZonePlayerPtr& ptr)
: m_ptr(ptr)
, m_valid(false)
, m_coordinator(false)
{
  m_id = QString::fromUtf8(ptr->GetUUID().c_str());
  m_name = QString::fromUtf8(ptr->c_str());
  m_icon = QString::fromUtf8(ptr->GetIconName().c_str());
  m_coordinator = (ptr->GetAttribut("coordinator") == "true");
  m_valid = true;
}

QVariant RoomItem::payload() const
{
  QVariant var;
  var.setValue<SONOS::ZonePlayerPtr>(m_ptr);
  return var;
}

RoomsModel::RoomsModel(QObject* parent)
: QAbstractListModel(parent)
, m_zoneId("")
{
}

RoomsModel::~RoomsModel()
{
  clearData();
  qDeleteAll(m_items);
  m_items.clear();
}

int RoomsModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return m_items.count();
}

QVariant RoomsModel::data(const QModelIndex& index, int role) const
{
  if (index.row() < 0 || index.row() >= m_items.count())
      return QVariant();

  const RoomItem* item = m_items[index.row()];
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
  case CoordinatorRole:
    return item->coordinator();
  default:
    return QVariant();
  }
}

QHash<int, QByteArray> RoomsModel::roleNames() const
{
  QHash<int, QByteArray> roles;
  roles[PayloadRole] = "payload";
  roles[IdRole] = "id";
  roles[NameRole] = "name";
  roles[IconRole] = "icon";
  roles[CoordinatorRole] = "coordinator";
  return roles;
}

QVariantMap RoomsModel::get(int row)
{
  if (row < 0 || row >= m_items.count())
    return QVariantMap();
  const RoomItem* item = m_items[row];
  QVariantMap model;
  QHash<int, QByteArray> roles = roleNames();
  model[roles[PayloadRole]] = item->payload();
  model[roles[IdRole]] = item->id();
  model[roles[NameRole]] = item->name();
  model[roles[IconRole]] = item->icon();
  model[roles[CoordinatorRole]] = item->coordinator();
  return model;
}

void RoomsModel::clearData()
{
  qDeleteAll(m_data);
  m_data.clear();
}

bool RoomsModel::loadData()
{
  if (!m_provider)
    return false;

  clearData();

  if (m_zoneId.isNull())
  {
    SONOS::ZonePlayerList zonePlayers = m_provider->getSystem().GetZonePlayerList();
    for (SONOS::ZonePlayerList::iterator it = zonePlayers.begin(); it != zonePlayers.end(); ++it)
    {
      RoomItem* item = new RoomItem(it->second);
      if (item->isValid())
        m_data << item;
      else
        delete item;
    }
  }
  else
  {
    SONOS::ZoneList zones = m_provider->getSystem().GetZoneList();
    SONOS::ZoneList::const_iterator itz = zones.find(m_zoneId.toUtf8().constData());
    if (itz != zones.end())
    {
      for (std::vector<SONOS::ZonePlayerPtr>::iterator it = itz->second->begin(); it != itz->second->end(); ++it)
      {
        RoomItem* item = new RoomItem(*it);
        if (item->isValid())
          m_data << item;
        else
          delete item;
      }
    }
  }
  return true;
}

bool RoomsModel::load(QObject* sonos)
{
  m_provider = reinterpret_cast<Sonos*> (sonos);
  m_zoneId = QString();
  if (!loadData())
    return false;
  resetModel();
  return true;
}

bool RoomsModel::load(QObject* sonos, const QString& zoneId)
{
  m_provider = reinterpret_cast<Sonos*> (sonos);
  m_zoneId = zoneId;
  if (!loadData())
    return false;
  resetModel();
  return true;
}

void RoomsModel::resetModel()
{
  beginResetModel();
  beginRemoveRows(QModelIndex(), 0, m_items.count()-1);
  qDeleteAll(m_items);
  m_items.clear();
  endRemoveRows();
  beginInsertRows(QModelIndex(), 0, m_data.count()-1);
  foreach (RoomItem* item, m_data)
      m_items << item;
  m_data.clear();
  endInsertRows();
  endResetModel();
  emit countChanged();
}
