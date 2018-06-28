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

#include "favoritesmodel.h"
#include "sonos.h"
#include "tools.h"
#include "../../lib/noson/noson/src/contentdirectory.h"

#include <cstdio> // for strncpy
#include <cctype> // for isdigit

FavoriteItem::FavoriteItem(const SONOS::DigitalItemPtr& ptr, const QString& baseURL)
: m_ptr(ptr)
, m_valid(false)
, m_canQueue(false)
{
  m_id = QString::fromUtf8(ptr->GetObjectID().c_str());
  m_title = QString::fromUtf8(ptr->GetValue("dc:title").c_str());
  m_normalized = normalizedString(m_title);
  m_description = QString::fromUtf8(ptr->GetValue("r:description").c_str());
  QString uri = QString::fromUtf8(ptr->GetValue("upnp:albumArtURI").c_str());
  if (!uri.isEmpty())
  {
    if (uri.at(0) == '/')
      m_art.append(baseURL).append(uri);
    else
      m_art.append(uri);
  }
  if (SONOS::System::ExtractObjectFromFavorite(ptr, m_objectPtr))
  {
    m_valid = true;
    m_canQueue = SONOS::System::CanQueueItem(m_objectPtr);
    m_objectId = QString::fromUtf8(m_objectPtr->GetObjectID().c_str());
    m_objectUri = QString::fromUtf8(m_objectPtr->GetValue("res").c_str());
    m_isService = SONOS::System::IsItemFromService(m_objectPtr);
    switch (m_objectPtr->subType())
    {
    case SONOS::DigitalItem::SubType_album:
      m_type = FavoriteType::album;
      m_artist = QString::fromUtf8(m_objectPtr->GetValue("dc:creator").c_str());
      m_album = QString::fromUtf8(m_objectPtr->GetValue("dc:title").c_str());
      break;
    case SONOS::DigitalItem::SubType_person:
      m_type = FavoriteType::person;
      m_artist = QString::fromUtf8(m_objectPtr->GetValue("dc:title").c_str());
      break;
    case SONOS::DigitalItem::SubType_genre:
      m_type = FavoriteType::genre;
      break;
    case SONOS::DigitalItem::SubType_playlistContainer:
      m_type = FavoriteType::playlist;
      break;
    case SONOS::DigitalItem::SubType_audioItem:
      m_type = FavoriteType::audioItem;
      m_artist = QString::fromUtf8(m_objectPtr->GetValue("dc:creator").c_str());
      m_album = QString::fromUtf8(m_objectPtr->GetValue("upnp:album").c_str());
      break;
    default:
      m_type = FavoriteType::unknown;
    }
  }
}

QVariant FavoriteItem::payload() const
{
  QVariant var;
  var.setValue<SONOS::DigitalItemPtr>(m_ptr);
  return var;
}

QVariant FavoriteItem::object() const
{
  QVariant var;
  var.setValue<SONOS::DigitalItemPtr>(m_objectPtr);
  return var;
}

FavoritesModel::FavoritesModel(QObject* parent)
: QAbstractListModel(parent)
{
}

FavoritesModel::~FavoritesModel()
{
  clearData();
  qDeleteAll(m_items);
  m_items.clear();
}

void FavoritesModel::addItem(FavoriteItem* item)
{
  {
    SONOS::LockGuard lock(m_lock);
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    m_items << item;
    m_objectIDs.insert(item->objectId(), item->id());
    endInsertRows();
  }
  emit countChanged();
}

int FavoritesModel::rowCount(const QModelIndex& parent) const
{
  Q_UNUSED(parent);
  SONOS::LockGuard lock(m_lock);
  return m_items.count();
}

QVariant FavoritesModel::data(const QModelIndex& index, int role) const
{
  SONOS::LockGuard lock(m_lock);
  if (index.row() < 0 || index.row() >= m_items.count())
      return QVariant();

  const FavoriteItem* item = m_items[index.row()];
  switch (role)
  {
  case PayloadRole:
    return item->payload();
  case IdRole:
    return item->id();
  case TitleRole:
    return item->title();
  case DescriptionRole:
    return item->description();
  case ArtRole:
    return item->art();
  case NormalizedRole:
    return item->normalized();
  case ObjectRole:
    return item->object();
  case ObjectIdRole:
    return item->objectId();
  case ObjectUriRole:
    return item->objectUri();
  case TypeRole:
    return item->type();
  case CanQueueRole:
    return item->canQueue();
  case ArtistRole:
    return item->artist();
  case AlbumRole:
    return item->album();
  case IsServiceRole:
    return item->isService();
  default:
    return QVariant();
  }
}

bool FavoritesModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
  SONOS::LockGuard lock(m_lock);
  if (index.row() < 0 || index.row() >= m_items.count())
      return false;

  FavoriteItem* item = m_items[index.row()];
  switch (role)
  {
  case ArtRole:
    item->setArt(value.toString());
    return true;
  default:
    return false;
  }
}

QHash<int, QByteArray> FavoritesModel::roleNames() const
{
  QHash<int, QByteArray> roles;
  roles[PayloadRole] = "payload";
  roles[IdRole] = "id";
  roles[TitleRole] = "title";
  roles[DescriptionRole] = "description";
  roles[ArtRole] = "art";
  roles[NormalizedRole] = "normalized";
  roles[ObjectRole] = "object";
  roles[ObjectIdRole] = "objectId";
  roles[ObjectUriRole] = "objectUri";
  roles[TypeRole] = "type";
  roles[CanQueueRole] = "canQueue";
  roles[ArtistRole] = "artist";
  roles[AlbumRole] = "album";
  roles[IsServiceRole] = "isService";
  return roles;
}

QVariantMap FavoritesModel::get(int row)
{
  SONOS::LockGuard lock(m_lock);
  if (row < 0 || row >= m_items.count())
    return QVariantMap();
  const FavoriteItem* item = m_items[row];
  QVariantMap model;
  QHash<int, QByteArray> roles = roleNames();
  model[roles[PayloadRole]] = item->payload();
  model[roles[IdRole]] = item->id();
  model[roles[TitleRole]] = item->title();
  model[roles[DescriptionRole]] = item->description();
  model[roles[ArtRole]] = item->art();
  model[roles[NormalizedRole]] = item->normalized();
  model[roles[ObjectRole]] = item->object();
  model[roles[ObjectIdRole]] = item->objectId();
  model[roles[ObjectUriRole]] = item->objectUri();
  model[roles[TypeRole]] = item->type();
  model[roles[CanQueueRole]] = item->canQueue();
  model[roles[ArtistRole]] = item->artist();
  model[roles[AlbumRole]] = item->album();
  model[roles[IsServiceRole]] = item->isService();
  return model;
}

bool FavoritesModel::init(QObject* sonos, const QString& root, bool fill)
{
  QString _root;
  if (root.isEmpty())
    _root = QString::fromUtf8("FV:2");
  else
    _root = root;
  return ListModel::init(sonos, _root, fill);
}

void FavoritesModel::clearData()
{
  SONOS::LockGuard lock(m_lock);
  qDeleteAll(m_data);
  m_data.clear();
}

bool FavoritesModel::loadData()
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
  SONOS::ContentList cl(cd, m_root.isEmpty() ? "FV:2" : m_root.toUtf8().constData());
  for (SONOS::ContentList::iterator it = cl.begin(); it != cl.end(); ++it)
  {
    FavoriteItem* item = new FavoriteItem(*it, url);
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

bool FavoritesModel::asyncLoad()
{
  if (m_provider)
  {
    m_provider->runModelLoader(this);
    return true;
  }
  return false;
}

void FavoritesModel::resetModel()
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
      m_objectIDs.clear();
      endRemoveRows();
    }
    if (m_data.count() > 0)
    {
      beginInsertRows(QModelIndex(), 0, m_data.count()-1);
      foreach (FavoriteItem* item, m_data) {
          m_items << item;
          m_objectIDs.insert(item->objectId(), item->id());
      }
      m_data.clear();
      endInsertRows();
    }
    m_dataState = ListModel::Synced;
    endResetModel();
  }
  emit countChanged();
}

void FavoritesModel::handleDataUpdate()
{
  if (!updateSignaled())
  {
    setUpdateSignaled(true);
    dataUpdated();
  }
}

QString FavoritesModel::findFavorite(const QVariant& payload) const
{
  if (!m_provider)
    return "";
  SONOS::DigitalItemPtr ptr = payload.value<SONOS::DigitalItemPtr>();
  SONOS::PlayerPtr player = m_provider->getPlayer();
  if (ptr && player)
  {
    SONOS::LockGuard lock(m_lock);
    //@FIXME handle queued item
    QString objId = QString::fromUtf8(player->GetItemIdFromUriMetadata(ptr).c_str());
    QMap<QString, QString>::ConstIterator it = m_objectIDs.find(objId);
    if (it != m_objectIDs.end())
      return it.value();
  }
  return "";
}
