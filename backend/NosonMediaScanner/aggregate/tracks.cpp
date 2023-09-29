/*
 *      Copyright (C) 2019 Jean-Luc Barriere
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#include "tracks.h"

#include <string>

using namespace mediascanner;

TrackModel::TrackModel(const MediaFilePtr& file)
: Model(file)
{
  m_key = QByteArray(std::to_string(file->fileId).c_str());
  if (file->mediaInfo)
  {
    m_normalized = normalizedString(file->mediaInfo->title);
    m_position = ((file->mediaInfo->discNo & 0x7fff) << 16) | (file->mediaInfo->trackNo & 0xffff);
  }
}

QVariant TrackModel::payload() const
{
  QVariant var;
  var.setValue<MediaFilePtr>(MediaFilePtr(m_file));
  return var;
}

Tracks::Tracks(QObject * parent)
: ListModel(parent)
, m_artistFilter()
, m_albumFilter()
, m_genreFilter()
, m_composerFilter()
{
}

Tracks::~Tracks()
{
  clear();
}

void Tracks::addItem(ItemPtr& item)
{
  {
    LockGuard<QRecursiveMutex> lock(m_lock);
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    m_items << item;
    endInsertRows();
  }
  emit countChanged();
}

void Tracks::removeItem(const QByteArray& id)
{
  {
    LockGuard<QRecursiveMutex> lock(m_lock);
    int row = 0;
    for (const ItemPtr& item : m_items)
    {
      if (item->model.key() == id)
      {
        beginRemoveRows(QModelIndex(), row, row);
        m_items.removeOne(item);
        endRemoveRows();
        break;
      }
      ++row;
    }
  }
  emit countChanged();
}

int Tracks::rowCount(const QModelIndex& parent) const
{
  Q_UNUSED(parent);
  LockGuard<QRecursiveMutex> lock(m_lock);
  return m_items.count();
}

QVariant Tracks::data(const QModelIndex& index, int role) const
{
  LockGuard<QRecursiveMutex> lock(m_lock);
  if (index.row() < 0 || index.row() >= m_items.count())
      return QVariant();

  const ItemPtr item = m_items[index.row()];
  switch (role)
  {
  case PayloadRole:
  {
    QVariant var;
    var.setValue<ItemPtr>(ItemPtr(item));
    return var;
  }
  case IdRole:
    return item->model.key();
  case TitleRole:
    return item->model.title();
  case AuthorRole:
    return item->model.author();
  case AlbumRole:
    return item->model.album();
  case AlbumArtistRole:
    return item->model.albumArtist();
  case GenreRole:
    return item->model.genre();
  case ComposerRole:
    return item->model.composer();
  case FilePathRole:
    return item->model.filePath();
  case CodecRole:
    return item->model.codec();
  case AlbumTrackNoRole:
    return item->model.albumTrackNo();
  case AlbumDiscNoRole:
    return item->model.albumDiscNo();
  case YearRole:
    return item->model.year();
  case DurationRole:
    return item->model.duration();
  case SampleRateRole:
    return item->model.sampleRate();
  case ChannelsRole:
    return item->model.channels();
  case BitRateRole:
    return item->model.bitRate();
  case HasArtRole:
    return item->model.hasArt();
  case ArtRole:
    return item->model.art();
  case NormalizedRole:
    return item->model.normalized();
  case PositionRole:
    return item->model.position();
  default:
    return QVariant();
  }
}

bool Tracks::setData(const QModelIndex &index, const QVariant &value, int role)
{
  LockGuard<QRecursiveMutex> lock(m_lock);
  if (index.row() < 0 || index.row() >= m_items.count())
      return false;

  ItemPtr item = m_items[index.row()];
  switch (role)
  {
  case ArtRole:
    item->model.setArt(value.toString());
    return true;
  default:
    return false;
  }
}

QHash<int, QByteArray> Tracks::roleNames() const
{
  QHash<int, QByteArray> roles;
  roles[PayloadRole] = "payload";
  roles[IdRole] = "id";
  roles[TitleRole] = "title";
  roles[AuthorRole] = "author";
  roles[AlbumRole] = "album";
  roles[AlbumArtistRole] = "albumArtist";
  roles[GenreRole] = "genre";
  roles[ComposerRole] = "composer";
  roles[FilePathRole] = "filePath";
  roles[CodecRole] = "codec";
  roles[AlbumTrackNoRole] = "albumTrackNo";
  roles[AlbumDiscNoRole] = "albumDiscNo";
  roles[YearRole] = "year";
  roles[DurationRole] = "duration";
  roles[SampleRateRole] = "sampleRate";
  roles[ChannelsRole] = "channels";
  roles[BitRateRole] = "bitRate";
  roles[HasArtRole] = "hasArt";
  roles[ArtRole] = "art";
  roles[NormalizedRole] = "normalized";
  roles[PositionRole] = "position";
  return roles;
}

QVariantMap Tracks::get(int row)
{
  LockGuard<QRecursiveMutex> lock(m_lock);
  if (row < 0 || row >= m_items.count())
    return QVariantMap();
  const ItemPtr item = m_items[row];
  QVariantMap model;
  QHash<int, QByteArray> roles = roleNames();
  QVariant var;
  var.setValue<ItemPtr>(ItemPtr(item));
  model[roles[PayloadRole]] = var;
  model[roles[IdRole]] = item->model.key();
  model[roles[TitleRole]] = item->model.title();
  model[roles[AuthorRole]] = item->model.author();
  model[roles[AlbumRole]] = item->model.album();
  model[roles[AlbumArtistRole]] = item->model.albumArtist();
  model[roles[GenreRole]] = item->model.genre();
  model[roles[ComposerRole]] = item->model.composer();
  model[roles[FilePathRole]] = item->model.filePath();
  model[roles[CodecRole]] = item->model.codec();
  model[roles[AlbumTrackNoRole]] = item->model.albumTrackNo();
  model[roles[AlbumDiscNoRole]] = item->model.albumDiscNo();
  model[roles[YearRole]] = item->model.year();
  model[roles[DurationRole]] = item->model.duration();
  model[roles[SampleRateRole]] = item->model.sampleRate();
  model[roles[ChannelsRole]] = item->model.channels();
  model[roles[BitRateRole]] = item->model.bitRate();
  model[roles[HasArtRole]] = item->model.hasArt();
  model[roles[ArtRole]] = item->model.art();
  model[roles[NormalizedRole]] = item->model.normalized();
  model[roles[PositionRole]] = item->model.position();
  return model;
}

void Tracks::clear()
{
  LockGuard<QRecursiveMutex> lock(m_lock);
  if (m_dataState == ListModel::New)
      return;
  if (m_items.count() > 0)
  {
    beginRemoveRows(QModelIndex(), 0, m_items.count()-1);
    m_items.clear();
    endRemoveRows();
  }
  m_dataState = ListModel::NoData;
}

bool Tracks::load()
{
  {
    LockGuard<QRecursiveMutex> lock(m_lock);
    beginResetModel();
    clear();

    m_data.clear();
    QList<MediaFilePtr> list = m_provider->allParsedFiles();
    for (const MediaFilePtr& file : list)
      checkAndAdd(file);

    m_dataState = ListModel::Loaded;
    endResetModel();
  }
  emit countChanged();
  emit loaded(true);
  return true;
}

void Tracks::checkAndAdd(const MediaFilePtr& file)
{
    QByteArray key;
    if (
            (m_artistFilter.isEmpty() || m_artistFilter.compare(file->mediaInfo->artist, Qt::CaseSensitivity::CaseInsensitive) == 0) &&
            (m_albumFilter.isEmpty() || m_albumFilter.compare(file->mediaInfo->album, Qt::CaseSensitivity::CaseInsensitive) == 0) &&
            (m_albumArtistFilter.isEmpty() || m_albumArtistFilter.compare(file->mediaInfo->albumArtist, Qt::CaseSensitivity::CaseInsensitive) == 0) &&
            (m_genreFilter.isEmpty() || m_genreFilter.compare(file->mediaInfo->genre, Qt::CaseSensitivity::CaseInsensitive) == 0) &&
            (m_composerFilter.isEmpty() || m_composerFilter.compare(file->mediaInfo->composer, Qt::CaseSensitivity::CaseInsensitive) == 0) &&
            m_data.insertFile(file, &key))
      addItem(m_data.find(key).value());
}

void Tracks::onFileAdded(const MediaFilePtr& file)
{
  LockGuard<QRecursiveMutex> lock(m_lock);
  checkAndAdd(file);
}

void Tracks::onFileRemoved(const MediaFilePtr& file)
{
  LockGuard<QRecursiveMutex> lock(m_lock);
  QByteArray key;
  if (m_data.removeFile(file, &key))
    removeItem(key);
}
