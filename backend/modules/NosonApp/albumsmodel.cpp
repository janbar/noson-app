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

#include "albumsmodel.h"
#include "sonos.h"
#include "tools.h"
#include "../../lib/noson/noson/src/contentdirectory.h"

AlbumItem::AlbumItem(const SONOS::DigitalItemPtr& ptr, const QString& baseURL)
: m_ptr(ptr)
, m_valid(false)
{
  m_id = QString::fromUtf8(ptr->GetObjectID().c_str());
  if (ptr->subType() == SONOS::DigitalItem::SubType_album)
  {
    m_title = QString::fromUtf8(ptr->GetValue("dc:title").c_str());
    m_normalized = normalizedString(m_title);
    m_artist = QString::fromUtf8(ptr->GetValue("dc:creator").c_str());
    QString uri = QString::fromUtf8(ptr->GetValue("upnp:albumArtURI").c_str());
    if (!uri.isEmpty())
      m_art.append(baseURL).append(uri);
    m_valid = true;
  }
}

QVariant AlbumItem::payload() const
{
  QVariant var;
  var.setValue<SONOS::DigitalItemPtr>(m_ptr);
  return var;
}

AlbumsModel::AlbumsModel(QObject* parent)
: QAbstractListModel(parent)
{
}

AlbumsModel::~AlbumsModel()
{
}

void AlbumsModel::addItem(AlbumItem* item)
{
  {
    SONOS::LockGuard lock(m_lock);
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    m_items << item;
    endInsertRows();
  }
  emit countChanged();
}

int AlbumsModel::rowCount(const QModelIndex& parent) const
{
  Q_UNUSED(parent);
  SONOS::LockGuard lock(m_lock);
  return m_items.count();
}

QVariant AlbumsModel::data(const QModelIndex& index, int role) const
{
  SONOS::LockGuard lock(m_lock);
  if (index.row() < 0 || index.row() >= m_items.count())
      return QVariant();

  const AlbumItem* item = m_items[index.row()];
  switch (role)
  {
  case PayloadRole:
    return item->payload();
  case IdRole:
    return item->id();
  case TitleRole:
    return item->title();
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

QHash<int, QByteArray> AlbumsModel::roleNames() const
{
  QHash<int, QByteArray> roles;
  roles[PayloadRole] = "payload";
  roles[IdRole] = "id";
  roles[TitleRole] = "title";
  roles[ArtistRole] = "artist";
  roles[ArtRole] = "art";
  roles[NormalizedRole] = "normalized";
  return roles;
}

QVariantMap AlbumsModel::get(int row)
{
  SONOS::LockGuard lock(m_lock);
  if (row < 0 || row >= m_items.count())
    return QVariantMap();
  const AlbumItem* item = m_items[row];
  QVariantMap model;
  QHash<int, QByteArray> roles = roleNames();
  model[roles[PayloadRole]] = item->payload();
  model[roles[IdRole]] = item->id();
  model[roles[TitleRole]] = item->title();
  model[roles[ArtistRole]] = item->artist();
  model[roles[ArtRole]] = item->art();
  model[roles[NormalizedRole]] = item->normalized();
  return model;
}

bool AlbumsModel::init(QObject* sonos, const QString& root, bool fill)
{
  QString _root;
  if (root.isEmpty())
    _root = QString::fromUtf8(SONOS::ContentSearch(SONOS::SearchAlbum,"").Root().c_str());
  else
    _root = root;
  return ListModel::init(sonos, _root, fill);
}

void AlbumsModel::clear()
{
  {
    SONOS::LockGuard lock(m_lock);
    beginRemoveRows(QModelIndex(), 0, m_items.count());
    m_items.clear();
    endRemoveRows();
  }
  emit countChanged();
}

bool AlbumsModel::load()
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
  SONOS::ContentList cl(cd, m_root.isEmpty() ? SONOS::ContentSearch(SONOS::SearchAlbum,"").Root() : m_root.toUtf8().constData());
  for (SONOS::ContentList::iterator it = cl.begin(); it != cl.end(); ++it)
  {
    AlbumItem* item = new AlbumItem(*it, url);
    if (item->isValid())
      addItem(item);
    else
      delete item;
  }
  if (cl.failure())
    return m_loaded = false;
  m_updateID = cl.GetUpdateID(); // sync new baseline
  return m_loaded = true;
}

bool AlbumsModel::asyncLoad()
{
  if (m_provider)
    m_provider->runModelLoader(this);
}

void AlbumsModel::handleDataUpdate()
{
  if (!updateSignaled())
  {
    setUpdateSignaled(true);
    dataUpdated();
  }
}
