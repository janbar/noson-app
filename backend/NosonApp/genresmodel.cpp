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

#include "genresmodel.h"
#include "sonos.h"
#include "tools.h"
#include <noson/contentdirectory.h>

using namespace nosonapp;

GenreItem::GenreItem(const SONOS::DigitalItemPtr& ptr, const QString& baseURL)
: m_ptr(ptr)
, m_valid(false)
{
  m_id = QString::fromUtf8(ptr->GetObjectID().c_str());
  if (ptr->subType() == SONOS::DigitalItem::SubType_genre)
  {
    m_genre = QString::fromUtf8(ptr->GetValue("dc:title").c_str());
    m_normalized = normalizedString(m_genre);
    (void)baseURL;
    m_valid = true;
  }
}

QVariant GenreItem::payload() const
{
  QVariant var;
  var.setValue<SONOS::DigitalItemPtr>(SONOS::DigitalItemPtr(m_ptr));
  return var;
}

GenresModel::GenresModel(QObject* parent)
: QAbstractListModel(parent)
{
}

GenresModel::~GenresModel()
{
  qDeleteAll(m_data);
  m_data.clear();
  qDeleteAll(m_items);
  m_items.clear();
}

void GenresModel::addItem(GenreItem* item)
{
  {
    LockGuard<QRecursiveMutex> g(m_lock);
    beginInsertRows(QModelIndex(), m_items.count(), m_items.count());
    m_items << item;
    endInsertRows();
  }
  emit countChanged();
}

int GenresModel::rowCount(const QModelIndex& parent) const
{
  Q_UNUSED(parent);
  LockGuard<QRecursiveMutex> g(m_lock);
  return m_items.count();
}

QVariant GenresModel::data(const QModelIndex& index, int role) const
{
  LockGuard<QRecursiveMutex> g(m_lock);
  if (index.row() < 0 || index.row() >= m_items.count())
      return QVariant();

  const GenreItem* item = m_items[index.row()];
  switch (role)
  {
  case PayloadRole:
    return item->payload();
  case IdRole:
    return item->id();
  case GenreRole:
    return item->genre();
  case NormalizedRole:
    return item->normalized();
  default:
    return QVariant();
  }
}

QHash<int, QByteArray> GenresModel::roleNames() const
{
  QHash<int, QByteArray> roles;
  roles[PayloadRole] = "payload";
  roles[IdRole] = "id";
  roles[GenreRole] = "genre";
  roles[NormalizedRole] = "normalized";
  return roles;
}

QVariantMap GenresModel::get(int row)
{
  LockGuard<QRecursiveMutex> g(m_lock);
  if (row < 0 || row >= m_items.count())
    return QVariantMap();
  const GenreItem* item = m_items[row];
  QVariantMap model;
  QHash<int, QByteArray> roles = roleNames();
  model[roles[PayloadRole]] = item->payload();
  model[roles[IdRole]] = item->id();
  model[roles[GenreRole]] = item->genre();
  model[roles[NormalizedRole]] = item->normalized();
  return model;
}

bool GenresModel::init(Sonos* provider, const QString& root, bool fill)
{
  QString _root;
  if (root.isEmpty())
    _root = QString::fromUtf8(SONOS::ContentSearch(SONOS::SearchGenre,"").Root().c_str());
  else
    _root = root;
  return ListModel<Sonos>::configure(provider, _root, fill);
}

void GenresModel::clearData()
{
  LockGuard<QRecursiveMutex> g(m_lock);
  qDeleteAll(m_data);
  m_data.clear();
}

bool GenresModel::loadData()
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
  QString url = m_provider->getBaseUrl();
  SONOS::ContentDirectory cd(m_provider->getHost(), m_provider->getPort());
  SONOS::ContentList cl(cd, m_root.isEmpty() ? SONOS::ContentSearch(SONOS::SearchGenre,"").Root() : m_root.toUtf8().constData());
  for (SONOS::ContentList::iterator it = cl.begin(); it != cl.end(); ++it)
  {
    GenreItem* item = new GenreItem(*it, url);
    if (item->isValid())
      m_data << item;
    else
      delete item;
  }
  if (cl.failure())
  {
    m_dataState = DataStatus::DataFailure;
    emit loaded(false);
    return false;
  }
  m_updateID = cl.GetUpdateID(); // sync new baseline
  m_dataState = DataStatus::DataLoaded;
  emit loaded(true);
  return true;
}

bool GenresModel::asyncLoad()
{
  if (m_provider)
  {
    m_provider->runContentLoader(this);
    return true;
  }
  return false;
}

void GenresModel::resetModel()
{
  {
    LockGuard<QRecursiveMutex> g(m_lock);
    if (m_dataState != DataStatus::DataLoaded)
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
      foreach (GenreItem* item, m_data)
          m_items << item;
      m_data.clear();
      endInsertRows();
    }
    m_dataState = DataStatus::DataSynced;
    endResetModel();
  }
  emit countChanged();
}

void GenresModel::clearModel()
{
  LockGuard<QRecursiveMutex> g(m_lock);
  if (m_dataState != DataStatus::DataBlank)
  {
    qDeleteAll(m_data);
    m_data.clear();
    m_dataState = DataStatus::DataLoaded;
    emit loaded(true);
  }
}

void GenresModel::handleDataUpdate()
{
  if (!updateSignaled())
  {
    setUpdateSignaled(true);
    dataUpdated();
  }
}
