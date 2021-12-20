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
#include "listmodel.h"
#include "locked.h"

using namespace mediascanner;

ListModel::ListModel(QObject * parent)
: QAbstractListModel(parent)
, m_lock(0)
, m_provider(MediaScanner::instance())
, m_dataState(ListModel::New)
, m_updateSignaled(false)
{
  m_lock = new QRecursiveMutex();
}

ListModel::~ListModel()
{
  {
    LockGuard<QRecursiveMutex> g(m_lock);
    m_provider->unregisterModel(this);
  }
  delete m_lock;
}

bool ListModel::init(bool fill /*= true*/)
{
  LockGuard<QRecursiveMutex> g(m_lock); // is recursive
  m_provider->unregisterModel(this);
  m_provider->registerModel(this);
  m_dataState = ListModel::NoData;
  if (fill)
    return this->load();
  return false; // not filled
}
