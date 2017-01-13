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

#include "genresmodel.h"
#include "sonos.h"
#include "tools.h"
#include "../../lib/noson/noson/src/contentdirectory.h"

GenreItem::GenreItem(const SONOS::DigitalItemPtr& ptr, const QString& baseURL)
: m_ptr(ptr)
, m_valid(false)
{
  m_id = QString::fromUtf8(ptr->GetObjectID().c_str());
  if (ptr->subType() == SONOS::DigitalItem::SubType_genre)
  {
    m_genre = QString::fromUtf8(ptr->GetValue("dc:title").c_str());
    m_normalized = normalizedString(m_genre);
    //m_art.append(baseURL).append(uri);
    m_valid = true;
  }
}

QVariant GenreItem::payload() const
{
  QVariant var;
  var.setValue<SONOS::DigitalItemPtr>(m_ptr);
  return var;
}

GenresModel::GenresModel(QObject* parent)
: QAbstractListModel(parent)
{
}

GenresModel::~GenresModel()
{
  clear();
}

void GenresModel::addItem(GenreItem* item)
{
  {
    SONOS::LockGuard lock(m_lock);
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    m_items << item;
    endInsertRows();
  }
  emit countChanged();
}

int GenresModel::rowCount(const QModelIndex& parent) const
{
  Q_UNUSED(parent);
  SONOS::LockGuard lock(m_lock);
  return m_items.count();
}

QVariant GenresModel::data(const QModelIndex& index, int role) const
{
  SONOS::LockGuard lock(m_lock);
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
  SONOS::LockGuard lock(m_lock);
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

bool GenresModel::init(QObject* sonos, const QString& root, bool fill)
{
  QString _root;
  if (root.isEmpty())
    _root = QString::fromUtf8(SONOS::ContentSearch(SONOS::SearchGenre,"").Root().c_str());
  else
    _root = root;
  return ListModel::init(sonos, _root, fill);
}

void GenresModel::clear()
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

bool GenresModel::load()
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
  SONOS::ContentList cl(cd, m_root.isEmpty() ? SONOS::ContentSearch(SONOS::SearchGenre,"").Root() : m_root.toUtf8().constData());
  for (SONOS::ContentList::iterator it = cl.begin(); it != cl.end(); ++it)
  {
    GenreItem* item = new GenreItem(*it, url);
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

bool GenresModel::asyncLoad()
{
  if (m_provider)
    m_provider->runModelLoader(this);
}

void GenresModel::handleDataUpdate()
{
  if (!updateSignaled())
  {
    setUpdateSignaled(true);
    dataUpdated();
  }
}
