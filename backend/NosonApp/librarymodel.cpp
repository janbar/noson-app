/*
 *      Copyright (C) 2022 Jean-Luc Barriere
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

#include "librarymodel.h"
#include "sonos.h"
#include "tools.h"
#include "cppdef.h"

#include <cstdio> // for strncpy
#include <cctype> // for isdigit

#define MODELVIEW_SIZE      100
#define ROOT_DISPLAY_TYPE   SONOS::SMAPIItem::Editorial
#define ROOT_TAG            "root"
#define SEARCH_TAG          "SEARCH"

#define SEARCH_ARTISTS      "artists"
#define SEARCH_ALBUMS       "albums"
#define SEARCH_TRACKS       "tracks"
#define SEARCH_GENRES       "genres"
#define SEARCH_COMPOSERS    "composers"

using namespace nosonapp;

LibraryItem::LibraryItem(const SONOS::DigitalItemPtr& data, const QString& baseURL)
: m_ptr(data)
, m_valid(false)
, m_canQueue(false)
, m_canPlay(false)
, m_isContainer(false)
{
  m_id = QString::fromUtf8(data->GetObjectID().c_str());
  m_parent = QString::fromUtf8(data->GetParentID().c_str());
  m_title = QString::fromUtf8(data->GetValue("dc:title").c_str());
  m_normalized = normalizedString(m_title);
  m_description = QString::fromUtf8(data->GetValue("r:description").c_str());
  QString uri = QString::fromUtf8(data->GetValue("upnp:albumArtURI").c_str());
  if (!uri.isEmpty())
  {
    if (uri.at(0) == '/')
      m_art.append(baseURL).append(uri);
    else
      m_art.append(uri);
  }

  m_displayType = LibraryModel::DisplayUnknown;
  bool _canqueue = false;
  bool _canplay = false;

  std::string tmp;
  switch (data->subType())
  {
  case SONOS::DigitalItem::SubType_album:
    m_type = LibraryModel::NodeAlbum;
    m_displayType = LibraryModel::DisplayTrackList;
    _canqueue = true;
    _canplay = true;
    tmp = data->GetValue("dc:creator");
    if (!tmp.empty())
      m_artist = QString::fromUtf8(tmp.c_str());
    else
      m_artist = QString::fromUtf8(data->GetValue("dc:contributor").c_str());
    m_album = QString::fromUtf8(data->GetValue("dc:title").c_str());
    break;
  case SONOS::DigitalItem::SubType_person:
    m_type = LibraryModel::NodePerson;
    _canqueue = false;
    _canplay = true;
    m_artist = QString::fromUtf8(data->GetValue("dc:title").c_str());
    break;
  case SONOS::DigitalItem::SubType_genre:
    m_type = LibraryModel::NodeGenre;
    _canqueue = false;
    _canplay = true;
    break;
  case SONOS::DigitalItem::SubType_playlistContainer:
    m_type = LibraryModel::NodePlaylist;
    m_displayType = LibraryModel::DisplayTrackList;
    _canqueue = false;
    _canplay = false;
    break;
  case SONOS::DigitalItem::SubType_audioItem:
    m_type = LibraryModel::NodeAudioItem;
    _canqueue = true;
    _canplay = true;
    tmp = data->GetValue("dc:creator");
    if (!tmp.empty())
      m_artist = QString::fromUtf8(tmp.c_str());
    else
      m_artist = QString::fromUtf8(data->GetValue("dc:contributor").c_str());
    m_album = QString::fromUtf8(data->GetValue("upnp:album").c_str());
    break;
  case SONOS::DigitalItem::SubType_storageFolder:
    m_type = LibraryModel::NodeFolder;
    m_displayType = LibraryModel::DisplayItemList;
    _canqueue = false;
    _canplay = false;
    break;
  default:
    m_type = LibraryModel::NodeUnknown;
  }

  if (!data->GetValue("res").empty())
  {
    m_canQueue = (_canqueue && SONOS::System::CanQueueItem(data));
    m_canPlay = _canplay;
    m_objectId = QString::fromUtf8(data->GetObjectID().c_str());
  }

  m_isContainer = data->IsContainer();
  m_valid = true;
}

QVariant LibraryItem::payload() const
{
  QVariant var;
  var.setValue<SONOS::DigitalItemPtr>(SONOS::DigitalItemPtr(m_ptr));
  return var;
}

LibraryModel::LibraryModel(QObject* parent)
: QAbstractListModel(parent)
{
}

LibraryModel::~LibraryModel()
{
  qDeleteAll(m_data);
  m_data.clear();
  qDeleteAll(m_items);
  m_items.clear();
  SAFE_DELETE(m_browser);
  SAFE_DELETE(m_content);
}

void LibraryModel::addItem(LibraryItem* item)
{
  {
    LockGuard<QRecursiveMutex> g(m_lock);
    beginInsertRows(QModelIndex(), m_items.count(), m_items.count());
    m_items << item;
    endInsertRows();
  }
  emit countChanged();
}

int LibraryModel::rowCount(const QModelIndex& parent) const
{
  Q_UNUSED(parent);
  LockGuard<QRecursiveMutex> g(m_lock);
  return m_items.count();
}

QVariant LibraryModel::data(const QModelIndex& index, int role) const
{
  LockGuard<QRecursiveMutex> g(m_lock);
  if (index.row() < 0 || index.row() >= m_items.count())
      return QVariant();

  const LibraryItem* item = m_items[index.row()];
  switch (role)
  {
  case PayloadRole:
    return item->payload();
  case ItemIndexRole:
    return m_firstIndex + index.row();
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
  case TypeRole:
    return item->type();
  case CanQueueRole:
    return item->canQueue();
  case CanPlayRole:
    return item->canPlay();
  case ArtistRole:
    return item->artist();
  case AlbumRole:
    return item->album();
  case ParentRole:
    return item->parent();
  case ObjectIdRole:
    return item->objectId();
  case DisplayTypeRole:
    return item->displayType();
  case IsContainerRole:
    return item->isContainer();
  default:
    return QVariant();
  }
}

QHash<int, QByteArray> LibraryModel::roleNames() const
{
  QHash<int, QByteArray> roles;
  roles[PayloadRole] = "payload";
  roles[ItemIndexRole] = "itemIndex";
  roles[IdRole] = "id";
  roles[TitleRole] = "title";
  roles[DescriptionRole] = "description";
  roles[ArtRole] = "art";
  roles[NormalizedRole] = "normalized";
  roles[TypeRole] = "type";
  roles[CanQueueRole] = "canQueue";
  roles[CanPlayRole] = "canPlay";
  roles[ArtistRole] = "artist";
  roles[AlbumRole] = "album";
  roles[ParentRole] = "parent";
  roles[ObjectIdRole] = "objectId";
  roles[DisplayTypeRole] = "displayType";
  roles[IsContainerRole] = "isContainer";
  return roles;
}

QVariantMap LibraryModel::get(int row)
{
  LockGuard<QRecursiveMutex> g(m_lock);
  if (row < 0 || row >= m_items.count())
    return QVariantMap();
  const LibraryItem* item = m_items[row];
  QVariantMap model;
  QHash<int, QByteArray> roles = roleNames();
  model[roles[PayloadRole]] = item->payload();
  model[roles[ItemIndexRole]] = m_firstIndex + row;
  model[roles[IdRole]] = item->id();
  model[roles[TitleRole]] = item->title();
  model[roles[DescriptionRole]] = item->description();
  model[roles[ArtRole]] = item->art();
  model[roles[NormalizedRole]] = item->normalized();
  model[roles[TypeRole]] = item->type();
  model[roles[CanQueueRole]] = item->canQueue();
  model[roles[CanPlayRole]] = item->canPlay();
  model[roles[ArtistRole]] = item->artist();
  model[roles[AlbumRole]] = item->album();
  model[roles[ParentRole]] = item->parent();
  model[roles[ObjectIdRole]] = item->objectId();
  model[roles[DisplayTypeRole]] = item->displayType();
  model[roles[IsContainerRole]] = item->isContainer();
  return model;
}

bool LibraryModel::init(Sonos* provider, const QString& root, bool fill)
{
  if (!provider)
    return false;
  SAFE_DELETE(m_browser)
  SAFE_DELETE(m_content)
  m_content = new SONOS::ContentDirectory(provider->getHost(), provider->getPort());
  // initialize path from root
  m_path.clear();
  m_path.push(Path(root, QString(ROOT_TAG), DisplayUnknown, NodeUnknown));
  emit pathChanged();
  // configure to listen any update on the current content
  return ListModel<Sonos>::configure(provider, pathId().toUtf8().constData(), fill);
}

void LibraryModel::clearData()
{
  LockGuard<QRecursiveMutex> g(m_lock);
  qDeleteAll(m_data);
  m_data.clear();
}

bool LibraryModel::loadData()
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

  SAFE_DELETE(m_browser);
  m_browser = new SONOS::ContentBrowser(*m_content, pathId().toUtf8().constData(), 1);
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
    LibraryItem* item = new LibraryItem(*it, url);
    m_data << item;
  }
  emit totalCountChanged();
  m_dataState = DataStatus::DataLoaded;
  emit loaded(true);
  return true;
}

bool LibraryModel::fetchAt(int index)
{
  if (!m_provider)
    return false;
  LockGuard<QRecursiveMutex> g(m_lock);
  m_fetchIndex = (index < 0 ? 0 : index);
  m_provider->runContentLoader(this);
  return true;
}

bool LibraryModel::fetchBack()
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

bool LibraryModel::fetchFront()
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

bool LibraryModel::asyncLoad()
{
  if (m_provider)
  {
    m_provider->runContentLoader(this);
    return true;
  }
  return false;
}

void LibraryModel::resetModel()
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
      foreach (LibraryItem* item, m_data)
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

QString LibraryModel::pathName() const
{
  LockGuard<QRecursiveMutex> g(m_lock);
  if (m_path.empty())
    return m_root;
  else
    return m_path.top().title;
}

QString LibraryModel::pathId() const
{
  LockGuard<QRecursiveMutex> g(m_lock);
  if (m_path.empty())
    return QString();
  else
    return m_path.top().id;
}

int LibraryModel::displayType() const
{
  LockGuard<QRecursiveMutex> g(m_lock);
  if (m_path.empty())
    return DisplayUnknown;
  else
    return m_path.top().displayType;
}

int LibraryModel::nodeType() const
{
  LockGuard<QRecursiveMutex> g(m_lock);
  if (m_path.empty())
    return NodeUnknown;
  else
    return m_path.top().nodeType;
}

int LibraryModel::viewIndex() const
{
  LockGuard<QRecursiveMutex> g(m_lock);
  if (m_path.empty())
    return 0;
  else
    return m_path.top().viewIndex;
}

QList<QString> LibraryModel::listSearchCategories() const
{
  QList<QString> list;
  list.append("artists");
  list.append("albums");
  list.append("tracks");
  list.append("genres");
  list.append("composers");
  return list;
}

bool LibraryModel::loadChild(const QString& id, const QString& title, int displayType, int nodeType, int viewIndex /*= 0*/)
{
  if (id.isEmpty())
    return false;
  {
    LockGuard<QRecursiveMutex> g(m_lock);
    // save current view index for this path item
    if (!m_path.empty())
    {
      m_path.top().fetchIndex = m_firstIndex;
      m_path.top().viewIndex = viewIndex;
    }
    m_path.push(Path(id, title, displayType, nodeType));
    m_fetchIndex = 0;
  }
  // reconfigure to listen any update on the current content
  ListModel<Sonos>::configure(m_provider, pathId().toUtf8().constData(), false);
  emit pathChanged();
  return loadData();
}

bool LibraryModel::asyncLoadChild(const QString &id, const QString &title, int displayType, int nodeType, int viewIndex /*= 0*/)
{
  if (id.isEmpty())
    return false;
  {
    LockGuard<QRecursiveMutex> g(m_lock);
    // save current view index for this path item
    if (!m_path.empty())
    {
      m_path.top().fetchIndex = m_firstIndex;
      m_path.top().viewIndex = viewIndex;
    }
    m_path.push(Path(id, title, displayType, nodeType));
    m_fetchIndex = 0;
  }
  // reconfigure to listen any update on the current content
  ListModel<Sonos>::configure(m_provider, pathId().toUtf8().constData(), false);
  emit pathChanged();
  return asyncLoad();
}

bool LibraryModel::loadParent()
{
  {
    LockGuard<QRecursiveMutex> g(m_lock);
    if (!m_path.empty())
      m_path.pop();
    // reset state before signal the change
    m_fetchIndex = m_path.top().fetchIndex;
  }
  emit pathChanged();
  // reconfigure to listen any update on the current content
  ListModel<Sonos>::configure(m_provider, pathId().toUtf8().constData(), false);
  return loadData();
}

bool LibraryModel::asyncLoadParent()
{
  if (!m_provider)
    return false;
  m_provider->runContentLoaderForContext(this, 1);
  return true;
}

bool LibraryModel::loadSearch(const QString &category, const QString &term)
{
  {
    LockGuard<QRecursiveMutex> g(m_lock);
    m_path.clear();
    QString arg;
    if (category == SEARCH_ALBUMS)
      arg.append("A:ALBUM");
    else if (category == SEARCH_ARTISTS)
      arg.append("A:ARTIST");
    else if (category == SEARCH_COMPOSERS)
      arg.append("A:COMPOSER");
    else if (category == SEARCH_GENRES)
      arg.append("A:GENRE");
    else if (category == SEARCH_TRACKS)
      arg.append("A:TRACKS");
    arg.append(":").append(term);
    m_path.push(Path(arg, SEARCH_TAG, LibraryModel::DisplayItemList, LibraryModel::NodeFolder));
    m_fetchIndex = 0;
  }
  emit pathChanged();
  return loadData();
}

bool LibraryModel::asyncLoadSearch(const QString &category, const QString &term)
{
  {
    LockGuard<QRecursiveMutex> g(m_lock);
    m_path.clear();
    QString arg;
    if (category == SEARCH_ALBUMS)
      arg.append("A:ALBUM");
    else if (category == SEARCH_ARTISTS)
      arg.append("A:ARTIST");
    else if (category == SEARCH_COMPOSERS)
      arg.append("A:COMPOSER");
    else if (category == SEARCH_GENRES)
      arg.append("A:GENRE");
    else if (category == SEARCH_TRACKS)
      arg.append("A:TRACKS");
    arg.append(":").append(term);
    m_path.push(Path(arg, SEARCH_TAG, LibraryModel::DisplayItemList, LibraryModel::NodeFolder));
    m_fetchIndex = 0;
  }
  emit pathChanged();
  if (!m_provider)
    return false;
  m_provider->runContentLoaderForContext(this, 2);
  return true;
}

bool LibraryModel::loadDataForContext(int id)
{
  switch (id)
  {
    case 0:
      return loadData();
    case 1:
      return loadParent();
    case 2:
      return loadData();
    default:
      return false;
  }
}

void LibraryModel::handleDataUpdate()
{
  if (!updateSignaled())
  {
    setUpdateSignaled(true);
    emit dataUpdated();
  }
}

QString LibraryModel::faultString()
{
  return QString::fromUtf8("ERROR");
}
