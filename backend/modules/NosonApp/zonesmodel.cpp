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

#include "zonesmodel.h"
#include "sonos.h"

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
}

void ZonesModel::addItem(ZoneItem* item)
{
  {
    SONOS::LockGuard lock(m_lock);
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    m_items << item;
    endInsertRows();
  }
  emit countChanged();
}

int ZonesModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return m_items.count();
}

QVariant ZonesModel::data(const QModelIndex& index, int role) const
{
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
  SONOS::LockGuard lock(m_lock);
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

bool ZonesModel::init(QObject* sonos, bool fill)
{
  return ListModel::init(sonos, "", fill);
}

void ZonesModel::clear()
{
  {
    SONOS::LockGuard lock(m_lock);
    beginRemoveRows(QModelIndex(), 0, m_items.count());
    m_items.clear();
    endRemoveRows();
  }
  emit countChanged();
}

bool ZonesModel::load()
{
  setUpdateSignaled(false);
  
  if (!m_provider)
    return false;
  clear();
  SONOS::ZoneList zones = m_provider->getSystem().GetZoneList();

  for (SONOS::ZoneList::iterator it = zones.begin(); it != zones.end(); ++it)
  {
    ZoneItem* item = new ZoneItem(it->second);
    if (item->isValid())
      addItem(item);
    else
      delete item;
  }
  return m_loaded = true;
}

void ZonesModel::handleDataUpdate()
{
  if (!updateSignaled())
  {
    setUpdateSignaled(true);
    dataUpdated();
  }
}
