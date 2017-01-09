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

#include "tracksmodel.h"
#include "sonos.h"
#include "../../lib/noson/noson/src/private/cppdef.h"

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
  clear();
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

void TracksModel::clear()
{
  {
    SONOS::LockGuard lock(m_lock);
    beginRemoveRows(QModelIndex(), 0, m_items.count());
    qDeleteAll(m_items);
    m_items.clear();
    m_totalCount = 0;
    endRemoveRows();
  }
  emit countChanged();
}

bool TracksModel::load()
{
  setUpdateSignaled(false);

  if (!m_provider)
    return false;
  clear();
  {
    SONOS::LockGuard lock(m_lock);
    SAFE_DELETE(m_contentList);
    SAFE_DELETE(m_contentDirectory);
  }
  const SONOS::PlayerPtr player = m_provider->getPlayer();
  if (!player)
    return false;
  {
    SONOS::LockGuard lock(m_lock);
    m_contentDirectory = new SONOS::ContentDirectory(player->GetHost(), player->GetPort());
    if (m_contentDirectory)
      m_contentList = new SONOS::ContentList(*m_contentDirectory, m_root.isEmpty() ? SONOS::ContentSearch(SONOS::SearchTrack,"").Root() : m_root.toUtf8().constData());
    if (!m_contentList)
      return false;
    m_totalCount = m_contentList->size();
    m_iterator = m_contentList->begin();
  }
  emit totalCountChanged();

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
      addItem(item);
      ++cnt;
    }
    else
    {
      delete item;
      // Also decrease total count
      if (m_totalCount)
      {
        --m_totalCount;
        emit totalCountChanged();
      }
    }
    ++m_iterator;
  }
  if (m_contentList->failure())
    return m_loaded = false;
  m_updateID = m_contentList->GetUpdateID(); // sync new baseline
  return m_loaded = true;
}

bool TracksModel::loadMore()
{
  SONOS::LockGuard lock(m_lock);
  if (!m_contentDirectory || !m_contentList)
    return false;
  // At end return false
  if (m_iterator == m_contentList->end())
    return false;

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
      addItem(item);
      ++cnt;
    }
    else
    {
      delete item;
      // Also decrease total count
      if (m_totalCount)
      {
        --m_totalCount;
        emit totalCountChanged();
      }
    }
    ++m_iterator;
  }
  if (m_contentList->failure())
    return false;
  return true;
}

bool TracksModel::asyncLoad()
{
  if (m_provider)
    m_provider->runModelLoader(this);
}

void TracksModel::handleDataUpdate()
{
  if (!updateSignaled())
  {
    setUpdateSignaled(true);
    dataUpdated();
  }
}

