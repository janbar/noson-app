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

#include "queuemodel.h"
#include "sonos.h"
#include "../../lib/noson/noson/src/contentdirectory.h"

QueueModel::QueueModel(QObject* parent)
: QAbstractListModel(parent)
{
}

QueueModel::~QueueModel()
{
  clear();
}

void QueueModel::addItem(TrackItem* item)
{
  {
    SONOS::LockGuard lock(m_lock);
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    m_items << item;
    endInsertRows();
  }
  emit countChanged();
}

int QueueModel::rowCount(const QModelIndex& parent) const
{
  Q_UNUSED(parent);
  SONOS::LockGuard lock(m_lock);
  return m_items.count();
}

QVariant QueueModel::data(const QModelIndex& index, int role) const
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

QHash<int, QByteArray> QueueModel::roleNames() const
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

QVariantMap QueueModel::get(int row)
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

bool QueueModel::init(QObject* sonos, const QString& root, bool fill)
{
  QString _root;
  if (root.isEmpty())
    _root = QString::fromUtf8(SONOS::ContentSearch(SONOS::SearchQueue,"").Root().c_str());
  else
    _root = root;
  return ListModel::init(sonos, _root, fill);
}

void QueueModel::clear()
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

bool QueueModel::load()
{
  setUpdateSignaled(false);

  if (!m_provider)
    return false;
  clear();
  const SONOS::PlayerPtr player = m_provider->getPlayer();
  if (!player)
    return false;

  QString port;
  port.setNum(player->GetPort());
  QString url = "http://";
  url.append(player->GetHost().c_str()).append(":").append(port);

  SONOS::ContentDirectory cd(player->GetHost(), player->GetPort());
  SONOS::ContentList cl(cd, m_root.isEmpty() ? SONOS::ContentSearch(SONOS::SearchQueue,"").Root() : m_root.toUtf8().constData());
  for (SONOS::ContentList::iterator it = cl.begin(); it != cl.end(); ++it)
  {
    TrackItem* item = new TrackItem(*it, url);
    addItem(item);
  }
  if (cl.failure())
    return m_loaded = false;
  m_updateID = cl.GetUpdateID(); // sync new baseline
  return m_loaded = true;
}

bool QueueModel::asyncLoad()
{
  if (m_provider)
    m_provider->runModelLoader(this);
}

void QueueModel::handleDataUpdate()
{
  if (!updateSignaled())
  {
    setUpdateSignaled(true);
    dataUpdated();
  }
}
