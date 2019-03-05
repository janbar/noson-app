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
#ifndef LISTMODEL
#define LISTMODEL

#include "mediascanner.h"
#include "locked.h"

#include <QObject>
#include <QAbstractListModel>

class QMutex;

namespace mediascanner
{

class MediaScanner;

class ListModel : public QAbstractListModel
{
  Q_OBJECT

public:
  ListModel(QObject * parent);
  virtual ~ListModel();

  virtual void clear() = 0;
  virtual bool load() = 0;

  enum dataState {
    New     = 0,
    NoData  = 1,
    Loaded  = 2,
    Synced  = 3
  };


public slots:
  virtual void onFileAdded(const MediaFilePtr& file) = 0;
  virtual void onFileRemoved(const MediaFilePtr& file) = 0;
  
protected:
  QMutex * m_lock;
  MediaScanner * m_provider;
  dataState m_dataState;

  virtual bool init(bool fill = true);

  bool updateSignaled() { return m_updateSignaled.Load(); }
  void setUpdateSignaled(bool val) { m_updateSignaled.Store(val); }

private:
  Locked<bool> m_updateSignaled;
};

}

#endif // LISTMODEL

