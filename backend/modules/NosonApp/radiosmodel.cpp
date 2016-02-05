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

#include "radiosmodel.h"
#include "sonos.h"
#include "tools.h"
#include "../../lib/noson/noson/src/contentdirectory.h"

#include <cstdio> // for strncpy
#include <cctype> // for isdigit

RadioItem::RadioItem(const SONOS::DigitalItemPtr& ptr, const QString& baseURL)
: m_ptr(ptr)
, m_valid(false)
{
  m_id = QString::fromUtf8(ptr->GetObjectID().c_str());
  if (ptr->subType() == SONOS::DigitalItem::SubType_audioItem)
  {
    m_title = QString::fromUtf8(ptr->GetValue("dc:title").c_str());
    m_normalized = normalizedString(m_title);
    m_genre = "";
    m_icon = "";
    m_streamId = "";
    m_valid = true;
    
    //@FIXME: extract tunein id from 'res'
    // x-sonosapi-stream:s24773?sid=254&amp;flags=32
    char buf[48];
    char* beg;
    strncpy(buf, ptr->GetValue("res").c_str(), sizeof(buf));
    if ((beg = strchr(buf, ':')) && *(++beg) == 's')
    {
      char* end = beg;
      while (isdigit(*(++end)));
      m_streamId = QString::fromUtf8(beg, end - beg);
    }
  }
}

QVariant RadioItem::payload() const
{
  QVariant var;
  var.setValue<SONOS::DigitalItemPtr>(m_ptr);
  return var;
}

RadiosModel::RadiosModel(QObject* parent)
: QAbstractListModel(parent)
{
}

RadiosModel::~RadiosModel()
{
}

void RadiosModel::addItem(RadioItem* item)
{
  {
    SONOS::LockGuard lock(m_lock);
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    m_items << item;
    endInsertRows();
  }
  emit countChanged();
}

int RadiosModel::rowCount(const QModelIndex& parent) const
{
  Q_UNUSED(parent);
  SONOS::LockGuard lock(m_lock);
  return m_items.count();
}

QVariant RadiosModel::data(const QModelIndex& index, int role) const
{
  SONOS::LockGuard lock(m_lock);
  if (index.row() < 0 || index.row() >= m_items.count())
      return QVariant();

  const RadioItem* item = m_items[index.row()];
  switch (role)
  {
  case PayloadRole:
    return item->payload();
  case IdRole:
    return item->id();
  case TitleRole:
    return item->title();
  case GenreRole:
    return item->genre();
  case IconRole:
    return item->icon();
  case StreamIdRole:
    return item->streamId();
  case NormalizedRole:
    return item->normalized();
  default:
    return QVariant();
  }
}

QHash<int, QByteArray> RadiosModel::roleNames() const
{
  QHash<int, QByteArray> roles;
  roles[PayloadRole] = "payload";
  roles[IdRole] = "id";
  roles[TitleRole] = "title";
  roles[GenreRole] = "genre";
  roles[IconRole] = "icon";
  roles[StreamIdRole] = "streamId";
  roles[NormalizedRole] = "normalized";
  return roles;
}

QVariantMap RadiosModel::get(int row)
{
  SONOS::LockGuard lock(m_lock);
  if (row < 0 || row >= m_items.count())
    return QVariantMap();
  const RadioItem* item = m_items[row];
  QVariantMap model;
  QHash<int, QByteArray> roles = roleNames();
  model[roles[PayloadRole]] = item->payload();
  model[roles[IdRole]] = item->id();
  model[roles[TitleRole]] = item->title();
  model[roles[GenreRole]] = item->genre();
  model[roles[IconRole]] = item->icon();
  model[roles[StreamIdRole]] = item->streamId();
  model[roles[NormalizedRole]] = item->normalized();
  return model;
}

bool RadiosModel::init(QObject* sonos, const QString& root, bool fill)
{
  QString _root;
  if (root.isEmpty())
    _root = QString::fromUtf8(SONOS::ContentSearch(SONOS::SearchRadio,"").Root().c_str());
  else
    _root = root;
  return ListModel::init(sonos, _root, fill);
}

void RadiosModel::clear()
{
  {
    SONOS::LockGuard lock(m_lock);
    beginRemoveRows(QModelIndex(), 0, m_items.count());
    m_items.clear();
    endRemoveRows();
  }
  emit countChanged();
}

bool RadiosModel::load()
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
  SONOS::ContentList cl(cd, m_root.isEmpty() ? SONOS::ContentSearch(SONOS::SearchRadio,"").Root() : m_root.toUtf8().constData());
  for (SONOS::ContentList::iterator it = cl.begin(); it != cl.end(); ++it)
  {
    RadioItem* item = new RadioItem(*it, url);
    if (item->isValid())
      addItem(item);
    else
      delete item;
  }
  m_updateID = cl.GetUpdateID(); // sync new baseline
  return m_loaded = true;
}

bool RadiosModel::asyncLoad()
{
  if (m_provider)
    m_provider->runModelLoader(this);
}

void RadiosModel::handleDataUpdate()
{
  if (!updateSignaled())
  {
    setUpdateSignaled(true);
    dataUpdated();
  }
}
