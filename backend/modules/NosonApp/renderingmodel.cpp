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

#include "renderingmodel.h"

RenderingItem::RenderingItem(const Player::RCProperty& rcp)
: m_volume(0)
, m_mute(0)
{
  m_uuid = QString::fromUtf8(rcp.uuid.c_str());
  m_name = QString::fromUtf8(rcp.name.c_str());
  m_volume = rcp.volumeFake;
  m_mute = rcp.mute;
}

RenderingModel::RenderingModel(QObject* parent)
: QAbstractListModel(parent)
{
}

RenderingModel::~RenderingModel()
{
  clear();
}

void RenderingModel::addItem(RenderingItem* item)
{
  beginInsertRows(QModelIndex(), rowCount(), rowCount());
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
  return roles;
}

void RenderingModel::clear()
{
  beginRemoveRows(QModelIndex(), 0, m_items.count());
  qDeleteAll(m_items);
  m_items.clear();
  endRemoveRows();
  emit countChanged();
}

bool RenderingModel::load(QObject* player)
{
  Player* _player = reinterpret_cast<Player*> (player);
  if (!_player)
    return false;

  clear();
  const Player::RCTable& tab = _player->renderingTable();
  for (Player::RCTable::const_iterator it = tab.begin(); it != tab.end(); ++it)
    addItem(new RenderingItem(*it));
  return true;
}

void RenderingModel::setVolume(int index, const QVariant& volume)
{
  setData(QAbstractListModel::index(index), volume, VolumeRole);
}

void RenderingModel::setMute(int index, const QVariant& mute)
{
  setData(QAbstractListModel::index(index), mute, MuteRole);
}
