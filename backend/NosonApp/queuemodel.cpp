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

#include "queuemodel.h"
#include "player.h"
#include "cppdef.h"

#include <noson/contentdirectory.h>

#define MODELVIEW_SIZE  100

using namespace nosonapp;

QueueModel::QueueModel(QObject* parent)
: QAbstractListModel(parent)
{
}

QueueModel::~QueueModel()
{
  qDeleteAll(m_data);
  m_data.clear();
  qDeleteAll(m_items);
  m_items.clear();
  SAFE_DELETE(m_browser)
  SAFE_DELETE(m_content)
}

void QueueModel::addItem(TrackItem* item)
{
  {
    LockGuard<QRecursiveMutex> g(m_lock);
    beginInsertRows(QModelIndex(), m_items.count(), m_items.count());
    m_items << item;
    endInsertRows();
  }
  emit countChanged();
}

int QueueModel::rowCount(const QModelIndex& parent) const
{
  Q_UNUSED(parent);
  LockGuard<QRecursiveMutex> g(m_lock);
  return m_items.count();
}

QVariant QueueModel::data(const QModelIndex& index, int role) const
{
  LockGuard<QRecursiveMutex> g(m_lock);
  if (index.row() < 0 || index.row() >= m_items.count())
      return QVariant();

  const TrackItem* item = m_items[index.row()];
  switch (role)
  {
  case PayloadRole:
    return item->payload();
  case TrackIndexRole:
    return m_firstIndex + index.row();
  case IdRole:
    return item->id();
  case TitleRole:
    return item->title();
  case AuthorRole:
    return item->author();
  case AlbumRole:
    return item->album();
  case AlbumTrackNoRole:
    return item->albumTrackNo();
  case ArtRole:
    return item->art();
  case IsServiceRole:
    return item->isService();
  default:
    return QVariant();
  }
}

bool QueueModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
  LockGuard<QRecursiveMutex> g(m_lock);
  if (index.row() < 0 || index.row() >= m_items.count())
      return false;

  TrackItem* item = m_items[index.row()];
  switch (role)
  {
  case ArtRole:
    item->setArt(value.toString());
    return true;
  default:
    return false;
  }
}

QHash<int, QByteArray> QueueModel::roleNames() const
{
  QHash<int, QByteArray> roles;
  roles[PayloadRole] = "payload";
  roles[TrackIndexRole] = "trackIndex";
  roles[IdRole] = "id";
  roles[TitleRole] = "title";
  roles[AuthorRole] = "author";
  roles[AlbumRole] = "album";
  roles[AlbumTrackNoRole] = "albumTrackNo";
  roles[ArtRole] = "art";
  roles[IsServiceRole] = "isService";
  return roles;
}

QVariantMap QueueModel::get(int row)
{
  LockGuard<QRecursiveMutex> g(m_lock);
  if (row < 0 || row >= m_items.count())
    return QVariantMap();
  const TrackItem* item = m_items[row];
  QVariantMap model;
  QHash<int, QByteArray> roles = roleNames();
  model[roles[PayloadRole]] = item->payload();
  model[roles[TrackIndexRole]] = m_firstIndex + row;
  model[roles[IdRole]] = item->id();
  model[roles[TitleRole]] = item->title();
  model[roles[AuthorRole]] = item->author();
  model[roles[AlbumRole]] = item->album();
  model[roles[AlbumTrackNoRole]] = item->albumTrackNo();
  model[roles[ArtRole]] = item->art();
  model[roles[IsServiceRole]] = item->isService();
  return model;
}

bool QueueModel::init(Player* provider, bool fill)
{
  if (!provider)
    return false;
  SAFE_DELETE(m_browser)
  SAFE_DELETE(m_content)
  m_content = new SONOS::ContentDirectory(provider->getHost(), provider->getPort());
  QString root = QString::fromUtf8(SONOS::ContentSearch(SONOS::SearchQueue, "").Root().c_str());
  // configure to listen any update on the current content
  return ListModel<Player>::configure(provider, root, fill);
}

void QueueModel::clearData()
{
  LockGuard<QRecursiveMutex> g(m_lock);
  qDeleteAll(m_data);
  m_data.clear();
}

bool QueueModel::loadData()
{
  setUpdateSignaled(false);

  if (!m_provider)
  {
    emit loaded(false);
    return false;
  }

  LockGuard<QRecursiveMutex> g(m_lock);
  qDeleteAll(m_data);
  m_data.clear();
  m_dataState = DataStatus::DataNotFound;

  SAFE_DELETE(m_browser)
  m_browser = new SONOS::ContentBrowser(*m_content, m_root.toUtf8().constData(), 1);
  if (m_browser->total() > 0)
  {
    // adjust query index depending of data count
    if (m_fetchIndex + MODELVIEW_SIZE > m_browser->total())
    {
      int f = m_browser->total() - MODELVIEW_SIZE;
      m_fetchIndex = (f > 0 ? f : 0);
    }
    if (!m_browser->Browse(m_fetchIndex, MODELVIEW_SIZE))
    {
      emit totalCountChanged();
      m_dataState = DataStatus::DataFailure;
      emit loaded(false);
      return false;
    }
  }
  m_updateID = m_browser->GetUpdateID(); // sync new baseline
  m_totalCount = m_browser->total();

  SONOS::ContentBrowser::Table& tab = m_browser->table();
  QString url = m_provider->getBaseUrl();
  for (SONOS::ContentBrowser::Table::const_iterator it = tab.begin(); it != tab.end(); ++it)
  {
    TrackItem* item = new TrackItem(*it, url);
    m_data << item;
  }
  emit totalCountChanged();
  m_dataState = DataStatus::DataLoaded;
  emit loaded(true);
  return true;
}

bool QueueModel::fetchAt(int index)
{
  if (!m_provider)
    return false;
  LockGuard<QRecursiveMutex> g(m_lock);
  m_fetchIndex = (index < 0 ? 0 : index);
  m_provider->runContentLoader(this);
  return true;
}

bool QueueModel::fetchBack()
{
  if (!m_provider)
    return false;
  LockGuard<QRecursiveMutex> g(m_lock);
  if (m_firstIndex + m_items.size() < m_totalCount)
  {
    m_fetchIndex = m_firstIndex + MODELVIEW_SIZE/2;
    m_provider->runContentLoader(this);
    return true;
  }
  return false;
}

bool QueueModel::fetchFront()
{
  if (!m_provider)
    return false;
  LockGuard<QRecursiveMutex> g(m_lock);
  if (m_firstIndex > 0)
  {
    if (m_firstIndex > MODELVIEW_SIZE/2)
      m_fetchIndex = m_firstIndex - MODELVIEW_SIZE/2;
    else
      m_fetchIndex = 0;
    m_provider->runContentLoader(this);
    return true;
  }
  return false;
}

bool QueueModel::asyncLoad()
{
  if (m_provider)
  {
    m_provider->runContentLoader(this);
    return true;
  }
  return false;
}

void QueueModel::resetModel()
{
  {
    LockGuard<QRecursiveMutex> g(m_lock);
    if (m_dataState != DataStatus::DataLoaded)
      return;

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
      foreach (TrackItem* item, m_data)
          m_items << item;
      endInsertRows();
    }
    m_firstIndex = m_fetchIndex;

    emit viewUpdated();

    m_data.clear();
    m_dataState = DataStatus::DataSynced;
  }
  emit countChanged();
}

void QueueModel::handleDataUpdate()
{
  if (!updateSignaled())
  {
    setUpdateSignaled(true);
    emit dataUpdated();
  }
}
