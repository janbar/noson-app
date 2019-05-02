/*
 *      Copyright (C) 2018-2019 Jean-Luc Barriere
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

#ifndef NOSONAPPALARMSMODEL_H
#define NOSONAPPALARMSMODEL_H

#include "listmodel.h"
#include <noson/alarm.h>

#include <QAbstractListModel>

namespace nosonapp
{

class AlarmItem
{
public:
  AlarmItem(const SONOS::AlarmPtr& ptr);

  virtual ~AlarmItem() { }

  bool isValid() const { return m_valid; }

  QVariant payload() const;

  QString id() const;

  bool enabled() const;
  void setEnabled(bool value);

  QString programURI() const;
  void setProgramURI(const QString& value);

  QString programTitle() const;

  QVariant programMetadata() const;
  void setProgramMetadata(const QVariant& value);

  QString playMode() const;
  void setPlayMode(const QString& value);

  int volume() const;
  void setVolume(int value);

  bool includeLinkedZones() const;
  void setIncludeLinkedZones(bool value);

  QString roomUUID() const;
  void setRoomUUID(const QString& value);

  QString startLocalTime() const;
  void setStartLocalTime(const QString& value);

  QString duration() const;
  void setDuration(const QString& value);

  QString recurrence() const;
  void setRecurrence(const QString& value);

private:
  SONOS::AlarmPtr m_ptr;
  bool m_valid;
};

class AlarmsModel : public QAbstractListModel, public ListModel
{
  Q_OBJECT
  Q_PROPERTY(int count READ rowCount NOTIFY countChanged)

public:
  enum AlarmsRoles
  {
    PayloadRole,
    IdRole,
    EnabledRole,
    ProgramUriRole,
    ProgramTitleRole,
    ProgramMetadataRole,
    PlayModeRole,
    VolumeRole,
    IncludeLinkedZonesRole,
    RoomIdRole,
    StartLocalTimeRole,
    DurationRole,
    RecurrenceRole,
  };

  AlarmsModel(QObject* parent = 0);
  virtual ~AlarmsModel();

  void addItem(AlarmItem* item);

  int rowCount(const QModelIndex& parent = QModelIndex()) const;

  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;

  bool setData(const QModelIndex& index, const QVariant& value, int role);

  bool insertRow(int row, const QModelIndex& parent = QModelIndex());
  bool removeRow(int row, const QModelIndex& parent = QModelIndex());

  Q_INVOKABLE int append();

  Q_INVOKABLE bool remove(int row);

  Q_INVOKABLE QVariantMap get(int row);

  Q_INVOKABLE bool init(QObject* sonos, bool fill = false) { return ListModel::init(sonos, fill); }

  virtual void clearData();

  virtual bool loadData();

  Q_INVOKABLE bool asyncLoad();

  Q_INVOKABLE void resetModel();

  virtual void handleDataUpdate();

signals:
  void dataUpdated();
  void countChanged();
  void loaded(bool succeeded);

protected:
  QHash<int, QByteArray> roleNames() const;

private:
  QList<AlarmItem*> m_items;
  QList<AlarmItem*> m_data;

  static void alarmEventCB(void* handle);
};

}

#endif /* NOSONAPPALARMSMODEL_H */

