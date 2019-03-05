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
#include "albums.h"

using namespace mediascanner;

AlbumModel::AlbumModel(const MediaFilePtr& file)
: Model(file)
{
  if (file->mediaInfo)
  {
    m_key = file->mediaInfo->artist.toLower().toUtf8()
        .append("/").append(file->mediaInfo->album.toLower().toUtf8());
    m_normalized = normalizedString(file->mediaInfo->album);
  }
}

QVariant AlbumModel::payload() const
{
  QVariant var;
  var.setValue<MediaFilePtr>(m_file);
  return var;
}

Albums::Albums(QObject * parent)
: ListModel(parent)
, m_artistFilter()
{
}

Albums::~Albums()
{
  clear();
}

void Albums::addItem(ItemPtr& item)
{
  {
    LockGuard lock(m_lock);
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    m_items << item;
    endInsertRows();
  }
  emit countChanged();
}

void Albums::removeItem(const QByteArray& id)
{
  {
    LockGuard lock(m_lock);
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

int Albums::rowCount(const QModelIndex& parent) const
{
  Q_UNUSED(parent);
  LockGuard lock(m_lock);
  return m_items.count();
}

QVariant Albums::data(const QModelIndex& index, int role) const
{
  LockGuard lock(m_lock);
  if (index.row() < 0 || index.row() >= m_items.count())
      return QVariant();

  const ItemPtr item = m_items[index.row()];
  switch (role)
  {
  case PayloadRole:
  {
    QVariant var;
    var.setValue<ItemPtr>(item);
    return var;
  }
  case IdRole:
    return item->model.key();
  case ArtistRole:
    return item->model.artist();
  case AlbumRole:
    return item->model.album();
  case FilePathRole:
    return item->model.filePath();
  case YearRole:
    return item->model.year();
  case HasArtRole:
    return item->model.hasArt();
  case NormalizedRole:
    return item->model.normalized();
  default:
    return QVariant();
  }
}

bool Albums::setData(const QModelIndex &index, const QVariant &value, int role)
{
  LockGuard lock(m_lock);
  if (index.row() < 0 || index.row() >= m_items.count())
      return false;

  ItemPtr item = m_items[index.row()];
  switch (role)
  {
  default:
    return false;
  }
}

QHash<int, QByteArray> Albums::roleNames() const
{
  QHash<int, QByteArray> roles;
  roles[PayloadRole] = "payload";
  roles[IdRole] = "id";
  roles[ArtistRole] = "artist";
  roles[AlbumRole] = "album";
  roles[FilePathRole] = "filePath";
  roles[YearRole] = "year";
  roles[HasArtRole] = "hasArt";
  roles[NormalizedRole] = "normalized";
  return roles;
}

QVariantMap Albums::get(int row)
{
  LockGuard lock(m_lock);
  if (row < 0 || row >= m_items.count())
    return QVariantMap();
  const ItemPtr item = m_items[row];
  QVariantMap model;
  QHash<int, QByteArray> roles = roleNames();
  QVariant var;
  var.setValue<ItemPtr>(item);
  model[roles[PayloadRole]] = var;
  model[roles[IdRole]] = item->model.key();
  model[roles[ArtistRole]] = item->model.artist();
  model[roles[AlbumRole]] = item->model.album();
  model[roles[FilePathRole]] = item->model.filePath();
  model[roles[YearRole]] = item->model.year();
  model[roles[HasArtRole]] = item->model.hasArt();
  model[roles[NormalizedRole]] = item->model.normalized();
  return model;
}

void Albums::clear()
{
  LockGuard lock(m_lock);
  if (m_dataState == ListModel::New)
      return;
  beginRemoveRows(QModelIndex(), 0, m_items.count()-1);
  m_items.clear();
  endRemoveRows();
  m_dataState = ListModel::NoData;
}

bool Albums::load()
{
  {
    LockGuard lock(m_lock);
    beginResetModel();
    clear();

    m_data.clear();
    QList<MediaFilePtr> list = m_provider->allParsedFiles();
    for (const MediaFilePtr& file : list)
      onFileAdded(file);

    m_dataState = ListModel::Loaded;
    endResetModel();
  }
  emit countChanged();
  emit loaded(true);
  return true;
}

void Albums::onFileAdded(const MediaFilePtr& file)
{
  QByteArray key;
  if (
          (m_artistFilter.isEmpty() || m_artistFilter.compare(file->mediaInfo->artist, Qt::CaseSensitivity::CaseInsensitive) == 0) &&
          m_data.insertFile(file, &key))
    addItem(m_data.find(key).value());
}

void Albums::onFileRemoved(const MediaFilePtr& file)
{
  QByteArray key;
  if (m_data.removeFile(file, &key))
    removeItem(key);
}
