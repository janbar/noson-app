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

#include "listmodel.h"
#include "sonos.h"

ListModel::ListModel()
: m_lock(0)
, m_provider(0)
, m_updateID(0)
, m_pending(false)
, m_loaded(false)
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
  if (!_sonos)
    return false;
  if (m_provider)
    m_provider->unregisterModel(this);
  _sonos->registerModel(this, root);
  // Reset container status to allow async reload
  m_loaded = false;
  if (fill)
    this->load();
}
