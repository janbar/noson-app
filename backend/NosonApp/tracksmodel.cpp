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

#include "tracksmodel.h"
#include "sonos.h"
#include "private/cppdef.h"

#define LOAD_BULKSIZE 100

TrackItem::TrackItem(const SONOS::DigitalItemPtr& ptr, const QString& baseURL)
: m_ptr(ptr)
, m_valid(false)
, m_isService(false)
{
  m_id = QString::fromUtf8(ptr->GetObjectID().c_str());
  if (ptr->subType() == SONOS::DigitalItem::SubType_audioItem)
  {
    m_title = QString::fromUtf8(ptr->GetValue("dc:title").c_str());
    m_author = QString::fromUtf8(ptr->GetValue("dc:creator").c_str());
    m_album = QString::fromUtf8(ptr->GetValue("upnp:album").c_str());
    m_albumTrackNo = QString::fromUtf8(ptr->GetValue("upnp:originalTrackNumber").c_str());
    QString uri = QString::fromUtf8(ptr->GetValue("upnp:albumArtURI").c_str());
    if (!uri.isEmpty())
      m_art.append(baseURL).append(uri);
    m_isService = SONOS::System::IsItemFromService(ptr);
    m_valid = true;
  }
  else
  {
    m_title = QString::fromUtf8(ptr->GetValue("dc:title").c_str());
  }
}

QVariant TrackItem::payload() const
{
  QVariant var;
  var.setValue<SONOS::DigitalItemPtr>(m_ptr);
  return var;
}

TracksModel::TracksModel(QObject* parent)
: QAbstractListModel(parent)
, m_contentDirectory(0)
, m_contentList(0)
, m_totalCount(0)
{
}

TracksModel::~TracksModel()
{
  clearData();
  qDeleteAll(m_items);
  m_items.clear();
  SAFE_DELETE(m_contentList)
  SAFE_DELETE(m_contentDirectory);
}

void TracksModel::addItem(TrackItem* item)
{
  {
    SONOS::LockGuard lock(m_lock);
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    m_items << item;
    endInsertRows();
  }
  emit countChanged();
}

int TracksModel::rowCount(const QModelIndex& parent) const
{
  Q_UNUSED(parent);
  SONOS::LockGuard lock(m_lock);
  return m_items.count();
}

QVariant TracksModel::data(const QModelIndex& index, int role) const
{
  SONOS::LockGuard lock(m_lock);
  if (index.row() < 0 || index.row() >= m_items.count())
      return QVariant();

  const TrackItem* item = m_items[index.row()];
  switch (role)
  {
  case PayloadRole:
    return item->payload();
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

bool TracksModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
  SONOS::LockGuard lock(m_lock);
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

QHash<int, QByteArray> TracksModel::roleNames() const
{
  QHash<int, QByteArray> roles;
  roles[PayloadRole] = "payload";
  roles[IdRole] = "id";
  roles[TitleRole] = "title";
  roles[AuthorRole] = "author";
  roles[AlbumRole] = "album";
  roles[AlbumTrackNoRole] = "albumTrackNo";
  roles[ArtRole] = "art";
  roles[IsServiceRole] = "isService";
  return roles;
}

QVariantMap TracksModel::get(int row)
{
  SONOS::LockGuard lock(m_lock);
  if (row < 0 || row >= m_items.count())
    return QVariantMap();
  const TrackItem* item = m_items[row];
  QVariantMap model;
  QHash<int, QByteArray> roles = roleNames();
  model[roles[PayloadRole]] = item->payload();
  model[roles[IdRole]] = item->id();
  model[roles[TitleRole]] = item->title();
  model[roles[AuthorRole]] = item->author();
  model[roles[AlbumRole]] = item->album();
  model[roles[AlbumTrackNoRole]] = item->albumTrackNo();
  model[roles[ArtRole]] = item->art();
  model[roles[IsServiceRole]] = item->isService();
  return model;
}

bool TracksModel::init(QObject* sonos, const QString& root, bool fill)
{
  QString _root;
  if (root.isEmpty())
    _root = QString::fromUtf8(SONOS::ContentSearch(SONOS::SearchTrack,"").Root().c_str());
  else
    _root = root;
  return ListModel::init(sonos, _root, fill);
}

void TracksModel::clearData()
{
  SONOS::LockGuard lock(m_lock);
  qDeleteAll(m_data);
  m_data.clear();
}

bool TracksModel::loadData()
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
  SAFE_DELETE(m_contentList);
  SAFE_DELETE(m_contentDirectory);
  m_contentDirectory = new SONOS::ContentDirectory(player->GetHost(), player->GetPort());
  if (m_contentDirectory)
    m_contentList = new SONOS::ContentList(*m_contentDirectory, m_root.isEmpty() ? SONOS::ContentSearch(SONOS::SearchTrack,"").Root() : m_root.toUtf8().constData());
  if (!m_contentList)
  {
    emit loaded(false);
    return false;
  }
  m_totalCount = m_contentList->size();
  m_iterator = m_contentList->begin();

  QString port;
  port.setNum(m_contentDirectory->GetPort());
  QString url = "http://";
  url.append(m_contentDirectory->GetHost().c_str()).append(":").append(port);

  clearData();
  m_dataState = ListModel::NoData;
  unsigned cnt = 0;
  while (cnt < LOAD_BULKSIZE && m_iterator != m_contentList->end())
  {
    TrackItem* item = new TrackItem(*m_iterator, url);
    if (item->isValid())
    {
      m_data << item;
      ++cnt;
    }
    else
    {
      delete item;
      // Also decrease total count
      if (m_totalCount > 0)
        --m_totalCount;
    }
    ++m_iterator;
  }
  if (m_contentList->failure())
  {
    emit loaded(false);
    return false;
  }
  m_updateID = m_contentList->GetUpdateID(); // sync new baseline
  emit totalCountChanged();
  m_dataState = ListModel::Loaded;
  emit loaded(true);
  return true;
}

bool TracksModel::asyncLoad()
{
  if (m_provider)
  {
    m_provider->runModelLoader(this);
    return true;
  }
  return false;
}

bool TracksModel::loadMoreData()
{
  SONOS::LockGuard lock(m_lock);
  if (!m_contentDirectory || !m_contentList)
  {
    emit loadedMore(false);
    return false;
  }
  // At end return false
  if (m_iterator == m_contentList->end())
  {
    emit loadedMore(false);
    return false;
  }

  QString port;
  port.setNum(m_contentDirectory->GetPort());
  QString url = "http://";
  url.append(m_contentDirectory->GetHost().c_str()).append(":").append(port);

  unsigned cnt = 0;
  while (cnt < LOAD_BULKSIZE && m_iterator != m_contentList->end())
  {
    TrackItem* item = new TrackItem(*m_iterator, url);
    if (item->isValid())
    {
      m_data << item;
      ++cnt;
    }
    else
    {
      delete item;
      // Also decrease total count
      if (m_totalCount) {
        --m_totalCount;
        emit totalCountChanged();
      }
    }
    ++m_iterator;
  }
  if (m_contentList->failure())
  {
    emit loadedMore(false);
    return false;
  }
  m_dataState = ListModel::Loaded;
  emit loadedMore(true);
  return true;
}

bool TracksModel::asyncLoadMore()
{
  if (!m_provider)
    return false;
  m_provider->runCustomizedModelLoader(this, 1);
  return true;
}

void TracksModel::resetModel()
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
      foreach (TrackItem* item, m_data)
          m_items << item;
      m_data.clear();
      endInsertRows();
    }
    m_dataState = ListModel::Synced;
    endResetModel();
  }
  emit countChanged();
}

void TracksModel::appendModel()
{
  {
    SONOS::LockGuard lock(m_lock);
    if (m_dataState != ListModel::Loaded)
      return;
    int cnt = m_items.count();
    beginInsertRows(QModelIndex(), cnt, cnt + m_data.count()-1);
    foreach (TrackItem* item, m_data)
        m_items << item;
    m_data.clear();
    m_dataState = ListModel::Synced;
    endInsertRows();
  }
  emit countChanged();
}

bool TracksModel::customizedLoad(int id)
{
  switch (id)
  {
    case 0:
      return loadData();
    case 1:
      return loadMoreData();
    default:
      return false;
  }
}

void TracksModel::handleDataUpdate()
{
  if (!updateSignaled())
  {
    setUpdateSignaled(true);
    dataUpdated();
  }
}

