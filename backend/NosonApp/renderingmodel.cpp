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

#include "renderingmodel.h"

using namespace nosonapp;

RenderingItem::RenderingItem(const Player::RCProperty& rcp)
: m_volume(0)
, m_mute(false)
, m_outputFixed(false)
{
  m_uuid = QString::fromUtf8(rcp.uuid.c_str());
  m_name = QString::fromUtf8(rcp.name.c_str());
  m_volume = rcp.volumeFake;
  m_mute = rcp.mute;
  m_outputFixed = rcp.outputFixed;
}

RenderingModel::RenderingModel(QObject* parent)
: QAbstractListModel(parent)
{
}

RenderingModel::~RenderingModel()
{
  qDeleteAll(m_data);
  m_data.clear();
  qDeleteAll(m_items);
  m_items.clear();
}

void RenderingModel::addItem(RenderingItem* item)
{
  beginInsertRows(QModelIndex(), m_items.count(), m_items.count());
  m_items << item;
  endInsertRows();
  emit countChanged();
}

int RenderingModel::rowCount(const QModelIndex& parent) const
{
  Q_UNUSED(parent);
  return m_items.count();
}

QVariant RenderingModel::data(const QModelIndex& index, int role) const
{
  if (index.row() < 0 || index.row() >= m_items.count())
      return QVariant();

  const RenderingItem* item = m_items[index.row()];
  switch (role)
  {
  case UUIDRole:
    return item->uuid();
  case NameRole:
    return item->name();
  case VolumeRole:
    return item->volume();
  case MuteRole:
    return item->mute();
  case OutputFixedRole:
    return item->outputFixed();
  default:
    return QVariant();
  }
}

bool RenderingModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
  if (index.row() < 0 || index.row() >= m_items.count())
      return false;

  RenderingItem* item = m_items[index.row()];
  switch (role)
  {
  case VolumeRole:
    item->setVolume(value.toDouble());
    break;
  case MuteRole:
    item->setMute(value.toBool());
    break;
  case OutputFixedRole:
    item->setOutputFixed(value.toBool());
    break;
  default:
    return false;
  }
  emit dataChanged(index, index);
  return true;
}

QHash<int, QByteArray> RenderingModel::roleNames() const
{
  QHash<int, QByteArray> roles;
  roles[UUIDRole] = "uuid";
  roles[NameRole] = "name";
  roles[VolumeRole] = "volume";
  roles[MuteRole] = "mute";
  roles[OutputFixedRole] = "outputFixed";
  return roles;
}

void RenderingModel::clearData()
{
  qDeleteAll(m_items);
  m_items.clear();
}

bool RenderingModel::loadData()
{
  if (!m_player)
    return false;

  qDeleteAll(m_data);
  m_data.clear();
  const Player::RCTable& tab = m_player->renderingTable();
  for (Player::RCTable::const_iterator it = tab.begin(); it != tab.end(); ++it)
    m_data << new RenderingItem(*it);
  return true;
}

bool RenderingModel::load(QObject* player)
{
  m_player = reinterpret_cast<Player*> (player);
  if (!loadData())
    return false;
  resetModel();
  return true;
}

void RenderingModel::resetModel()
{
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
    foreach (RenderingItem* item, m_data)
        m_items << item;
    m_data.clear();
    endInsertRows();
  }
  endResetModel();
  emit countChanged();
}

void RenderingModel::setVolume(int index, const QVariant& volume)
{
  setData(QAbstractListModel::index(index), volume, VolumeRole);
}

void RenderingModel::setMute(int index, const QVariant& mute)
{
  setData(QAbstractListModel::index(index), mute, MuteRole);
}

void RenderingModel::setOutputFixed(int index, const QVariant& fixed)
{
  setData(QAbstractListModel::index(index), fixed, OutputFixedRole);
}
