/*
 *      Copyright (C) 2016 Jean-Luc Barriere
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

#include "roomsmodel.h"
#include "sonos.h"

RoomItem::RoomItem(const SONOS::ZonePlayerPtr& ptr)
: m_ptr(ptr)
, m_valid(false)
{
  m_id = QString::fromUtf8(ptr->GetUUID().c_str());
  m_name = QString::fromUtf8(ptr->c_str());
  m_icon = QString::fromUtf8(ptr->GetIconName().c_str());
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
{
}

RoomsModel::~RoomsModel()
{
}

void RoomsModel::addItem(RoomItem* item)
{
  {
    SONOS::LockGuard lock(m_lock);
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    m_items << item;
    endInsertRows();
  }
  emit countChanged();
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
  return roles;
}

QVariantMap RoomsModel::get(int row)
{
  SONOS::LockGuard lock(m_lock);
  if (row < 0 || row >= m_items.count())
    return QVariantMap();
  const RoomItem* item = m_items[row];
  QVariantMap model;
  QHash<int, QByteArray> roles = roleNames();
  model[roles[PayloadRole]] = item->payload();
  model[roles[IdRole]] = item->id();
  model[roles[NameRole]] = item->name();
  model[roles[IconRole]] = item->icon();
  return model;
}

bool RoomsModel::init(QObject* sonos, bool fill)
{
  return ListModel::init(sonos, "", fill);
}

void RoomsModel::clear()
{
  {
    SONOS::LockGuard lock(m_lock);
    beginRemoveRows(QModelIndex(), 0, m_items.count());
    m_items.clear();
    endRemoveRows();
  }
  emit countChanged();
}

bool RoomsModel::load()
{
  setUpdateSignaled(false);

  if (!m_provider)
    return false;
  clear();
  SONOS::ZonePlayerList zonePlayers = m_provider->getSystem().GetZonePlayerList();

  for (SONOS::ZonePlayerList::iterator it = zonePlayers.begin(); it != zonePlayers.end(); ++it)
  {
    RoomItem* item = new RoomItem(it->second);
    if (item->isValid())
      addItem(item);
    else
      delete item;
  }
  return m_loaded = true;
}

bool RoomsModel::load(const QString& zoneId)
{
  setUpdateSignaled(false);

  if (!m_provider)
    return false;
  clear();
  SONOS::ZoneList zones = m_provider->getSystem().GetZoneList();
  SONOS::ZoneList::const_iterator itz = zones.find(zoneId.toUtf8().constData());
  if (itz != zones.end())
  {
    for (std::vector<SONOS::ZonePlayerPtr>::iterator it = itz->second->begin(); it != itz->second->end(); ++it)
    {
      RoomItem* item = new RoomItem(*it);
      if (item->isValid())
        addItem(item);
      else
        delete item;
    }
  }
  return m_loaded = true;
}

void RoomsModel::handleDataUpdate()
{
  if (!updateSignaled())
  {
    setUpdateSignaled(true);
    dataUpdated();
  }
}
