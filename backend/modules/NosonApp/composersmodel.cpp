/*
 *      Copyright (C) 2018 Jean-Luc Barriere
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

#include "composersmodel.h"
#include "sonos.h"
#include "tools.h"
#include <noson/contentdirectory.h>

ComposerItem::ComposerItem(const SONOS::DigitalItemPtr& ptr, const QString& baseURL)
: m_ptr(ptr)
, m_valid(false)
{
  m_id = QString::fromUtf8(ptr->GetObjectID().c_str());
  if (ptr->subType() == SONOS::DigitalItem::SubType_person)
  {
    m_composer = QString::fromUtf8(ptr->GetValue("dc:title").c_str());
    m_normalized = normalizedString(m_composer);
    (void)baseURL; //m_art.append(baseURL).append(uri);
    m_valid = true;
  }
}

QVariant ComposerItem::payload() const
{
  QVariant var;
  var.setValue<SONOS::DigitalItemPtr>(m_ptr);
  return var;
}

ComposersModel::ComposersModel(QObject* parent)
: QAbstractListModel(parent)
{
}

ComposersModel::~ComposersModel()
{
  clearData();
  qDeleteAll(m_items);
  m_items.clear();
}

void ComposersModel::addItem(ComposerItem* item)
{
  {
    SONOS::LockGuard lock(m_lock);
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    m_items << item;
    endInsertRows();
  }
  emit countChanged();
}

int ComposersModel::rowCount(const QModelIndex& parent) const
{
  Q_UNUSED(parent);
  SONOS::LockGuard lock(m_lock);
  return m_items.count();
}

QVariant ComposersModel::data(const QModelIndex& index, int role) const
{
  SONOS::LockGuard lock(m_lock);
  if (index.row() < 0 || index.row() >= m_items.count())
      return QVariant();

  const ComposerItem* item = m_items[index.row()];
  switch (role)
  {
  case PayloadRole:
    return item->payload();
  case IdRole:
    return item->id();
  case ComposerRole:
    return item->composer();
  case ArtRole:
    return item->art();
  case NormalizedRole:
    return item->normalized();
  default:
    return QVariant();
  }
}

QHash<int, QByteArray> ComposersModel::roleNames() const
{
  QHash<int, QByteArray> roles;
  roles[PayloadRole] = "payload";
  roles[IdRole] = "id";
  roles[ComposerRole] = "composer";
  roles[ArtRole] = "art";
  roles[NormalizedRole] = "normalized";
  return roles;
}

QVariantMap ComposersModel::get(int row)
{
  SONOS::LockGuard lock(m_lock);
  if (row < 0 || row >= m_items.count())
    return QVariantMap();
  const ComposerItem* item = m_items[row];
  QVariantMap model;
  QHash<int, QByteArray> roles = roleNames();
  model[roles[PayloadRole]] = item->payload();
  model[roles[IdRole]] = item->id();
  model[roles[ComposerRole]] = item->composer();
  model[roles[ArtRole]] = item->art();
  model[roles[NormalizedRole]] = item->normalized();
  return model;
}

bool ComposersModel::init(QObject* sonos, const QString& root, bool fill)
{
  QString _root;
  if (root.isEmpty())
    _root = QString::fromUtf8(SONOS::ContentSearch(SONOS::SearchComposer,"").Root().c_str());
  else
    _root = root;
  return ListModel::init(sonos, _root, fill);
}

void ComposersModel::clearData()
{
  SONOS::LockGuard lock(m_lock);
  qDeleteAll(m_data);
  m_data.clear();
}

bool ComposersModel::loadData()
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
  SONOS::ContentList cl(cd, m_root.isEmpty() ? SONOS::ContentSearch(SONOS::SearchComposer,"").Root() : m_root.toUtf8().constData());
  for (SONOS::ContentList::iterator it = cl.begin(); it != cl.end(); ++it)
  {
    ComposerItem* item = new ComposerItem(*it, url);
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

bool ComposersModel::asyncLoad()
{
  if (m_provider)
  {
    m_provider->runModelLoader(this);
    return true;
  }
  return false;
}

void ComposersModel::resetModel()
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
      endRemoveRows();
    }
    if (m_data.count() > 0)
    {
      beginInsertRows(QModelIndex(), 0, m_data.count()-1);
      foreach (ComposerItem* item, m_data)
          m_items << item;
      m_data.clear();
      endInsertRows();
    }
    m_dataState = ListModel::Synced;
    endResetModel();
  }
  emit countChanged();
}

void ComposersModel::handleDataUpdate()
{
  if (!updateSignaled())
  {
    setUpdateSignaled(true);
    dataUpdated();
  }
}
