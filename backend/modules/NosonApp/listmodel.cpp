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

#include "listmodel.h"
#include "sonos.h"

ListModel::ListModel()
: m_lock(0)
, m_provider(0)
, m_updateID(0)
, m_pending(false)
, m_dataState(ListModel::New)
, m_updateSignaled(false)
{
  m_lock = SONOS::LockGuard::CreateLock();
}

ListModel::~ListModel()
{
  {
    SONOS::LockGuard lock(m_lock);
    if (m_provider)
      m_provider->unregisterModel(this);
  }
  SONOS::LockGuard::DestroyLock(m_lock);
}

bool ListModel::init(QObject* sonos, const QString& root, bool fill /*= false*/)
{
  SONOS::LockGuard lock(m_lock);
  Sonos* _sonos = reinterpret_cast<Sonos*> (sonos);
  if (m_provider)
    m_provider->unregisterModel(this);
  if (_sonos)
    _sonos->registerModel(this, root);
  m_provider = _sonos;
  m_root = root;
  // Reset container status to allow async reload
  m_dataState = ListModel::NoData;
  if (fill)
    return this->loadData();
  return false; // not filled
}
