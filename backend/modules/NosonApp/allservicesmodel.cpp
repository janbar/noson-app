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

#include "allservicesmodel.h"
#include "sonos.h"
#include "tools.h"

#include <cstdio> // for strncpy
#include <cctype> // for isdigit

AllServicesModel::AllServicesModel(QObject* parent)
: QAbstractListModel(parent)
{
}

AllServicesModel::~AllServicesModel()
{
  clearData();
  qDeleteAll(m_items);
  m_items.clear();
}

void AllServicesModel::addItem(ServiceItem* item)
{
  {
    SONOS::LockGuard lock(m_lock);
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    m_items << item;
    endInsertRows();
  }
  emit countChanged();
}

int AllServicesModel::rowCount(const QModelIndex& parent) const
{
  Q_UNUSED(parent);
  SONOS::LockGuard lock(m_lock);
  return m_items.count();
}

QVariant AllServicesModel::data(const QModelIndex& index, int role) const
{
  SONOS::LockGuard lock(m_lock);
  if (index.row() < 0 || index.row() >= m_items.count())
      return QVariant();

  const ServiceItem* item = m_items[index.row()];
  switch (role)
  {
  case PayloadRole:
    return item->payload();
  case IdRole:
    return item->id();
  case TitleRole:
    return item->title();
  case IconRole:
    return item->icon();
  case NickNameRole:
    return item->nickName();
  case NormalizedRole:
    return item->normalized();
  case TypeRole:
    return item->type();
  default:
    return QVariant();
  }
}

QHash<int, QByteArray> AllServicesModel::roleNames() const
{
  QHash<int, QByteArray> roles;
  roles[PayloadRole] = "payload";
  roles[IdRole] = "id";
  roles[TitleRole] = "title";
  roles[IconRole] = "icon";
  roles[NickNameRole] = "nickName";
  roles[NormalizedRole] = "normalized";
  roles[TypeRole] = "type";
  return roles;
}

QVariantMap AllServicesModel::get(int row)
{
  SONOS::LockGuard lock(m_lock);
  if (row < 0 || row >= m_items.count())
    return QVariantMap();
  const ServiceItem* item = m_items[row];
  QVariantMap model;
  QHash<int, QByteArray> roles = roleNames();
  model[roles[PayloadRole]] = item->payload();
  model[roles[IdRole]] = item->id();
  model[roles[TitleRole]] = item->title();
  model[roles[IconRole]] = item->icon();
  model[roles[NickNameRole]] = item->nickName();
  model[roles[NormalizedRole]] = item->normalized();
  model[roles[TypeRole]] = item->type();
  return model;
}

void AllServicesModel::clearData()
{
  SONOS::LockGuard lock(m_lock);
  qDeleteAll(m_data);
  m_data.clear();
}

bool AllServicesModel::loadData()
{
  setUpdateSignaled(false);
  
  if (!m_provider)
  {
    emit loaded(false);
    return false;
  }
  const SONOS::PlayerPtr player = m_provider->getPlayer();
  if (!player)
  {
    emit loaded(false);
    return false;
  }
  
  SONOS::LockGuard lock(m_lock);
  clearData();
  m_dataState = ListModel::NoData;
  SONOS::SMServiceList list = player->GetAvailableServices();
  for (SONOS::SMServiceList::const_iterator it = list.begin(); it != list.end(); ++it)
  {
    ServiceItem* item = new ServiceItem(*it);
    if (item->isValid())
      m_data << item;
    else
      delete item;
  }
  m_dataState = ListModel::Loaded;
  emit loaded(true);
  return true;
}

bool AllServicesModel::asyncLoad()
{
  if (m_provider)
  {
    m_provider->runModelLoader(this);
    return true;
  }
  return false;
}

void AllServicesModel::resetModel()
{
  {
    SONOS::LockGuard lock(m_lock);
    if (m_dataState != ListModel::Loaded)
        return;
    beginResetModel();
    beginRemoveRows(QModelIndex(), 0, m_items.count()-1);
    qDeleteAll(m_items);
    m_items.clear();
    endRemoveRows();
    beginInsertRows(QModelIndex(), 0, m_data.count()-1);
    foreach (ServiceItem* item, m_data)
        m_items << item;
    m_data.clear();
    m_dataState = ListModel::Synced;
    endInsertRows();
    endResetModel();
  }
  emit countChanged();
}

void AllServicesModel::handleDataUpdate()
{
  if (!updateSignaled())
  {
    setUpdateSignaled(true);
    dataUpdated();
  }
}
