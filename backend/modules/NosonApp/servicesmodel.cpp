/*
 *      Copyright (C) 2017 Jean-Luc Barriere
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
  m_nickName = QString::fromUtf8(ptr->GetAccount()->GetNickname().c_str());
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
  clear();
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
  return model;
}

bool ServicesModel::init(QObject* sonos, bool fill)
{
  return ListModel::init(sonos, "", fill);
}

void ServicesModel::clear()
{
  {
    SONOS::LockGuard lock(m_lock);
    beginRemoveRows(QModelIndex(), 0, m_items.count());
    qDeleteAll(m_items);
    m_items.clear();
    endRemoveRows();
  }
  emit countChanged();
}

bool ServicesModel::load()
{
  setUpdateSignaled(false);
  
  if (!m_provider)
    return false;
  clear();
  const SONOS::PlayerPtr player = m_provider->getPlayer();
  if (!player)
    return false;
  SONOS::SMServiceList list = player->GetAvailableServices();
  for (SONOS::SMServiceList::const_iterator it = list.begin(); it != list.end(); ++it)
  {
    ServiceItem* item = new ServiceItem(*it);
    if (item->isValid())
      addItem(item);
    else
      delete item;
  }
  return m_loaded = true;
}

bool ServicesModel::asyncLoad()
{
  if (m_provider)
    m_provider->runModelLoader(this);
}

void ServicesModel::handleDataUpdate()
{
  if (!updateSignaled())
  {
    setUpdateSignaled(true);
    dataUpdated();
  }
}
