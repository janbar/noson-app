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

  virtual void clearData() = 0;

  virtual bool loadData() = 0;

  virtual void handleDataUpdate() = 0;

  enum dataState {
    NoData  = 0,
    Loaded  = 1,
    Synced  = 2
  };

protected:
  SONOS::LockGuard::Lockable* m_lock;
  Sonos* m_provider;
  unsigned m_updateID;
  QString m_root;
  bool m_pending;
  dataState m_dataState;

  virtual bool init(QObject* sonos, const QString& root, bool fill = false);

  bool updateSignaled() { return m_updateSignaled.Load(); }
  void setUpdateSignaled(bool val) { m_updateSignaled.Store(val); }

  virtual bool customizedLoad(int id) { return false; }

private:
  SONOS::Locked<bool> m_updateSignaled;
};

#endif // LISTMODEL

