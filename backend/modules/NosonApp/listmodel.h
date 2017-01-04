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

#ifndef LISTMODEL
#define LISTMODEL

#include "../../lib/noson/noson/src/digitalitem.h"
#include "../../lib/noson/noson/src/sonoszone.h"
#include "../../lib/noson/noson/src/musicservices.h"
#include "../../lib/noson/noson/src/locked.h"

#include <QObject>

Q_DECLARE_METATYPE(SONOS::DigitalItemPtr)
Q_DECLARE_METATYPE(SONOS::ZonePtr)
Q_DECLARE_METATYPE(SONOS::ZonePlayerPtr)
Q_DECLARE_METATYPE(SONOS::SMServicePtr)

class Sonos;

class ListModel
{
  friend class Sonos;
public:
  ListModel();
  virtual ~ListModel();

  virtual void clear() = 0;

  virtual bool load() = 0;

  virtual void handleDataUpdate() = 0;

protected:
  SONOS::LockGuard::Lockable* m_lock;
  Sonos* m_provider;
  unsigned m_updateID;
  QString m_root;
  bool m_pending;
  bool m_loaded;

  virtual bool init(QObject* sonos, const QString& root, bool fill = false);

  bool updateSignaled() { return m_updateSignaled.Load(); }
  void setUpdateSignaled(bool val) { m_updateSignaled.Store(val); }

private:
  SONOS::Locked<bool> m_updateSignaled;
};

#endif // LISTMODEL

