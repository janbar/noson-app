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
#include "genres.h"

using namespace mediascanner;

GenreModel::GenreModel(const MediaFilePtr& file)
: Model(file)
{
  if (file->mediaInfo)
  {
    m_key = file->mediaInfo->genre.toLower().toUtf8();
    m_normalized = normalizedString(file->mediaInfo->genre);
  }
}

QVariant GenreModel::payload() const
{
  QVariant var;
  var.setValue<MediaFilePtr>(m_file);
  return var;
}

Genres::Genres(QObject * parent)
: ListModel(parent)
{
}

Genres::~Genres()
{
  clear();
}

void Genres::addItem(ItemPtr& item)
{
  {
    QMutexLocker lock(m_lock);
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    m_items << item;
    endInsertRows();
  }
  emit countChanged();
}

void Genres::removeItem(const QByteArray& id)
{
  {
    QMutexLocker lock(m_lock);
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

int Genres::rowCount(const QModelIndex& parent) const
{
  Q_UNUSED(parent);
  QMutexLocker lock(m_lock);
  return m_items.count();
}

QVariant Genres::data(const QModelIndex& index, int role) const
{
  QMutexLocker lock(m_lock);
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
  case GenreRole:
    return item->model.genre();
  case NormalizedRole:
    return item->model.normalized();
  default:
    return QVariant();
  }
}

bool Genres::setData(const QModelIndex &index, const QVariant &value, int role)
{
  QMutexLocker lock(m_lock);
  if (index.row() < 0 || index.row() >= m_items.count())
      return false;

  ItemPtr item = m_items[index.row()];
  switch (role)
  {
  default:
    return false;
  }
}

QHash<int, QByteArray> Genres::roleNames() const
{
  QHash<int, QByteArray> roles;
  roles[PayloadRole] = "payload";
  roles[IdRole] = "id";
  roles[GenreRole] = "genre";
  roles[NormalizedRole] = "normalized";
  return roles;
}

QVariantMap Genres::get(int row)
{
  QMutexLocker lock(m_lock);
  if (row < 0 || row >= m_items.count())
    return QVariantMap();
  const ItemPtr item = m_items[row];
  QVariantMap model;
  QHash<int, QByteArray> roles = roleNames();
  QVariant var;
  var.setValue<ItemPtr>(item);
  model[roles[PayloadRole]] = var;
  model[roles[IdRole]] = item->model.key();
  model[roles[GenreRole]] = item->model.genre();
  model[roles[NormalizedRole]] = item->model.normalized();
  return model;
}

void Genres::clear()
{
  QMutexLocker lock(m_lock);
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

bool Genres::load()
{
  {
    QMutexLocker lock(m_lock);
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

void Genres::onFileAdded(const MediaFilePtr& file)
{
  QByteArray key;
  if (m_data.insertFile(file, &key))
    addItem(m_data.find(key).value());
}

void Genres::onFileRemoved(const MediaFilePtr& file)
{
  QByteArray key;
  if (m_data.removeFile(file, &key))
    removeItem(key);
}
