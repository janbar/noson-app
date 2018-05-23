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

#include "servicesmodel.h"
#include "sonos.h"
#include "tools.h"

#include <cstdio> // for strncpy
#include <cctype> // for isdigit

ServiceItem::ServiceItem(const SONOS::SMServicePtr& ptr)
: m_ptr(ptr)
, m_valid(false)
{
  m_id = QString::fromUtf8(ptr->GetServiceDesc().c_str());
  m_title = QString::fromUtf8(ptr->GetName().c_str());
  m_normalized = normalizedString(m_title);
  m_icon = QString::fromUtf8(SONOS::System::GetLogoForService(ptr, "square").c_str());
  m_nickName = QString::fromUtf8(ptr->GetAccount()->GetCredentials().username.c_str());
  m_type = QString::fromUtf8(ptr->GetServiceType().c_str());
  m_serialNum = QString::fromUtf8(ptr->GetAccount()->GetSerialNum().c_str());
  m_valid = true;
}

QVariant ServiceItem::payload() const
{
  QVariant var;
  var.setValue<SONOS::SMServicePtr>(m_ptr);
  return var;
}

ServicesModel::ServicesModel(QObject* parent)
: QAbstractListModel(parent)
{
}

ServicesModel::~ServicesModel()
{
  clearData();
  qDeleteAll(m_items);
  m_items.clear();
}

void ServicesModel::addItem(ServiceItem* item)
{
  {
    SONOS::LockGuard lock(m_lock);
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    m_items << item;
    endInsertRows();
  }
  emit countChanged();
}

int ServicesModel::rowCount(const QModelIndex& parent) const
{
  Q_UNUSED(parent);
  SONOS::LockGuard lock(m_lock);
  return m_items.count();
}

QVariant ServicesModel::data(const QModelIndex& index, int role) const
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
  case SerialNumRole:
    return item->serialNum();
  default:
    return QVariant();
  }
}

QHash<int, QByteArray> ServicesModel::roleNames() const
{
  QHash<int, QByteArray> roles;
  roles[PayloadRole] = "payload";
  roles[IdRole] = "id";
  roles[TitleRole] = "title";
  roles[IconRole] = "icon";
  roles[NickNameRole] = "nickName";
  roles[NormalizedRole] = "normalized";
  roles[TypeRole] = "type";
  roles[SerialNumRole] = "serialNum";
  return roles;
}

QVariantMap ServicesModel::get(int row)
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
  model[roles[SerialNumRole]] = item->serialNum();
  return model;
}

void ServicesModel::clearData()
{
  SONOS::LockGuard lock(m_lock);
  qDeleteAll(m_data);
  m_data.clear();
}

bool ServicesModel::loadData()
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
  SONOS::SMServiceList list = player->GetEnabledServices();
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

bool ServicesModel::asyncLoad()
{
  if (m_provider)
  {
    m_provider->runModelLoader(this);
    return true;
  }
  return false;
}

void ServicesModel::resetModel()
{
  {
    SONOS::LockGuard lock(m_lock);
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
      foreach (ServiceItem* item, m_data)
          m_items << item;
      m_data.clear();
      m_dataState = ListModel::Synced;
      endInsertRows();
    }
    endResetModel();
  }
  emit countChanged();
}

void ServicesModel::handleDataUpdate()
{
  if (!updateSignaled())
  {
    setUpdateSignaled(true);
    dataUpdated();
  }
}
