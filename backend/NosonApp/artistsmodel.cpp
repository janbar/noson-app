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

#include "artistsmodel.h"
#include "sonos.h"
#include "tools.h"
#include <noson/contentdirectory.h>

using namespace nosonapp;

ArtistItem::ArtistItem(const SONOS::DigitalItemPtr& ptr, const QString& baseURL)
: m_ptr(ptr)
, m_valid(false)
{
  m_id = QString::fromUtf8(ptr->GetObjectID().c_str());
  if (ptr->subType() == SONOS::DigitalItem::SubType_person)
  {
    m_artist = QString::fromUtf8(ptr->GetValue("dc:title").c_str());
    m_normalized = normalizedString(m_artist);
    (void)baseURL;
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
  qDeleteAll(m_data);
  m_data.clear();
  qDeleteAll(m_items);
  m_items.clear();
}

void ArtistsModel::addItem(ArtistItem* item)
{
  {
    LockGuard g(m_lock);
    beginInsertRows(QModelIndex(), m_items.count(), m_items.count());
    m_items << item;
    endInsertRows();
  }
  emit countChanged();
}

int ArtistsModel::rowCount(const QModelIndex& parent) const
{
  Q_UNUSED(parent);
#ifdef USE_RECURSIVE_MUTEX
  LockGuard g(m_lock);
#endif
  return m_items.count();
}

QVariant ArtistsModel::data(const QModelIndex& index, int role) const
{
#ifdef USE_RECURSIVE_MUTEX
  LockGuard g(m_lock);
#endif
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
  LockGuard g(m_lock);
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

bool ArtistsModel::init(Sonos* provider, const QString& root, bool fill)
{
  QString _root;
  if (root.isEmpty())
    _root = QString::fromUtf8(SONOS::ContentSearch(SONOS::SearchArtist,"").Root().c_str());
  else
    _root = root;
  return ListModel<Sonos>::configure(provider, _root, fill);
}

void ArtistsModel::clearData()
{
  LockGuard g(m_lock);
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

  LockGuard g(m_lock);
  qDeleteAll(m_data);
  m_data.clear();
  m_dataState = DataStatus::DataNotFound;
  QString url = m_provider->getBaseUrl();
  SONOS::ContentDirectory cd(m_provider->getHost(), m_provider->getPort());
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
  m_dataState = DataStatus::DataLoaded;
  emit loaded(true);
  return true;
}

bool ArtistsModel::asyncLoad()
{
  if (m_provider)
  {
    m_provider->runContentLoader(this);
    return true;
  }
  return false;
}

void ArtistsModel::resetModel()
{
  {
    LockGuard g(m_lock);
    if (m_dataState != DataStatus::DataLoaded)
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
    m_dataState = DataStatus::DataSynced;
    endResetModel();
  }
  emit countChanged();
}

void ArtistsModel::clearModel()
{
  LockGuard g(m_lock);
  if (m_dataState != DataStatus::DataBlank)
  {
    qDeleteAll(m_data);
    m_data.clear();
    m_dataState = DataStatus::DataLoaded;
    emit loaded(true);
  }
}

void ArtistsModel::handleDataUpdate()
{
  if (!updateSignaled())
  {
    setUpdateSignaled(true);
    dataUpdated();
  }
}
