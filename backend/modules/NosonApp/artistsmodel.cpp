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

#include "artistsmodel.h"
#include "sonos.h"
#include "tools.h"
#include <noson/contentdirectory.h>

ArtistItem::ArtistItem(const SONOS::DigitalItemPtr& ptr, const QString& baseURL)
: m_ptr(ptr)
, m_valid(false)
{
  m_id = QString::fromUtf8(ptr->GetObjectID().c_str());
  if (ptr->subType() == SONOS::DigitalItem::SubType_person)
  {
    m_artist = QString::fromUtf8(ptr->GetValue("dc:title").c_str());
    m_normalized = normalizedString(m_artist);
    (void)baseURL; //m_art.append(baseURL).append(uri);
    m_valid = true;
  }
}

QVariant ArtistItem::payload() const
{
  QVariant var;
  var.setValue<SONOS::DigitalItemPtr>(m_ptr);
  return var;
}

ArtistsModel::ArtistsModel(QObject* parent)
: QAbstractListModel(parent)
{
}

ArtistsModel::~ArtistsModel()
{
  clearData();
  qDeleteAll(m_items);
  m_items.clear();
}

void ArtistsModel::addItem(ArtistItem* item)
{
  {
    SONOS::LockGuard lock(m_lock);
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    m_items << item;
    endInsertRows();
  }
  emit countChanged();
}

int ArtistsModel::rowCount(const QModelIndex& parent) const
{
  Q_UNUSED(parent);
  SONOS::LockGuard lock(m_lock);
  return m_items.count();
}

QVariant ArtistsModel::data(const QModelIndex& index, int role) const
{
  SONOS::LockGuard lock(m_lock);
  if (index.row() < 0 || index.row() >= m_items.count())
      return QVariant();

  const ArtistItem* item = m_items[index.row()];
  switch (role)
  {
  case PayloadRole:
    return item->payload();
  case IdRole:
    return item->id();
  case ArtistRole:
    return item->artist();
  case ArtRole:
    return item->art();
  case NormalizedRole:
    return item->normalized();
  default:
    return QVariant();
  }
}

QHash<int, QByteArray> ArtistsModel::roleNames() const
{
  QHash<int, QByteArray> roles;
  roles[PayloadRole] = "payload";
  roles[IdRole] = "id";
  roles[ArtistRole] = "artist";
  roles[ArtRole] = "art";
  roles[NormalizedRole] = "normalized";
  return roles;
}

QVariantMap ArtistsModel::get(int row)
{
  SONOS::LockGuard lock(m_lock);
  if (row < 0 || row >= m_items.count())
    return QVariantMap();
  const ArtistItem* item = m_items[row];
  QVariantMap model;
  QHash<int, QByteArray> roles = roleNames();
  model[roles[PayloadRole]] = item->payload();
  model[roles[IdRole]] = item->id();
  model[roles[ArtistRole]] = item->artist();
  model[roles[ArtRole]] = item->art();
  model[roles[NormalizedRole]] = item->normalized();
  return model;
}

bool ArtistsModel::init(QObject* sonos, const QString& root, bool fill)
{
  QString _root;
  if (root.isEmpty())
    _root = QString::fromUtf8(SONOS::ContentSearch(SONOS::SearchArtist,"").Root().c_str());
  else
    _root = root;
  return ListModel::init(sonos, _root, fill);
}

void ArtistsModel::clearData()
{
  SONOS::LockGuard lock(m_lock);
  qDeleteAll(m_data);
  m_data.clear();
}

bool ArtistsModel::loadData()
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
  QString port;
  port.setNum(player->GetPort());
  QString url = "http://";
  url.append(player->GetHost().c_str()).append(":").append(port);

  SONOS::ContentDirectory cd(player->GetHost(), player->GetPort());
  SONOS::ContentList cl(cd, m_root.isEmpty() ? SONOS::ContentSearch(SONOS::SearchArtist,"").Root() : m_root.toUtf8().constData());
  for (SONOS::ContentList::iterator it = cl.begin(); it != cl.end(); ++it)
  {
    ArtistItem* item = new ArtistItem(*it, url);
    if (item->isValid())
      m_data << item;
    else
      delete item;
  }
  if (cl.failure())
  {
    emit loaded(false);
    return false;
  }
  m_updateID = cl.GetUpdateID(); // sync new baseline
  m_dataState = ListModel::Loaded;
  emit loaded(true);
  return true;
}

bool ArtistsModel::asyncLoad()
{
  if (m_provider)
  {
    m_provider->runModelLoader(this);
    return true;
  }
  return false;
}

void ArtistsModel::resetModel()
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
      foreach (ArtistItem* item, m_data)
          m_items << item;
      m_data.clear();
      endInsertRows();
    }
    m_dataState = ListModel::Synced;
    endResetModel();
  }
  emit countChanged();
}

void ArtistsModel::handleDataUpdate()
{
  if (!updateSignaled())
  {
    setUpdateSignaled(true);
    dataUpdated();
  }
}
