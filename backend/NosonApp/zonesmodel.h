/*
 *      Copyright (C) 2015-2019 Jean-Luc Barriere
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

#ifndef NOSONAPPZONESMODEL_H
#define NOSONAPPZONESMODEL_H

#include "listmodel.h"
#include <noson/sonoszone.h>

#include <QAbstractListModel>

namespace nosonapp
{

class Sonos;
class Player;
class ZPRef;

class ZoneItem
{
public:
  ZoneItem(ZPRef* ptr);

  virtual ~ZoneItem() { }

  bool isValid() const { return m_valid; }

  QVariant payload() const;
  
  const QString& id() const { return m_id; }

  const QString& name() const { return m_name; }

  const QString& icon() const { return m_icon; }

  bool isGroup() const { return m_isGroup; }

  const QString& shortName() const { return m_shortName; }

  const QString& coordinatorName() const { return m_coordinatorName; }

  ZPRef* ref() const { return m_ptr; }

private:
  ZPRef* m_ptr;
  bool m_valid;
  QString m_id;
  QString m_name;
  QString m_icon;
  bool m_isGroup;
  QString m_shortName;
  QString m_coordinatorName;
};

class ZonesModel : public QAbstractListModel, public ListModel<Sonos>
{
  Q_OBJECT
  Q_PROPERTY(int count READ rowCount NOTIFY countChanged)

public:
  enum ZonesRoles
  {
    PayloadRole,
    IdRole,
    NameRole,
    IconRole,
    IsGroupRole,
    ShortNameRole,
    CoordinatorNameRole,
  };

  ZonesModel(QObject* parent = 0);
  virtual ~ZonesModel();

  void addItem(ZoneItem* item);

  int rowCount(const QModelIndex& parent = QModelIndex()) const;

  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;

  Q_INVOKABLE QVariantMap get(int row);

  Q_INVOKABLE Player* holdPlayer(int row);

  Q_INVOKABLE void releasePlayer(Player* player);

  Q_INVOKABLE bool isNew() { return m_dataState == DataStatus::DataBlank; }

  Q_INVOKABLE bool init(Sonos* provider, bool fill = false) { return ListModel<Sonos>::configure(provider, fill); }

  virtual void clearData();

  Q_INVOKABLE bool loadData();

  Q_INVOKABLE bool asyncLoad();

  Q_INVOKABLE void resetModel();

  Q_INVOKABLE void appendModel() { }

  virtual void handleDataUpdate();

signals:
  void dataUpdated();
  void countChanged();
  void loaded(bool succeeded);

  void zpConnectedChanged(int pid);
  void zpRenderingChanged(int pid);
  void zpRenderingGroupChanged(int pid);
  void zpSourceChanged(int pid);
  void zpPlaybackStateChanged(int pid);
  void zpPlayModeChanged(int pid);
  void zpSleepTimerChanged(int pid);

protected:
  QHash<int, QByteArray> roleNames() const;

private:
  QList<ZoneItem*> m_items;
  QList<ZoneItem*> m_data;

  int m_nextPid;

  typedef QMap<QString, ZPRef*> PlayerMap; // { key:zoneName , value:ref }
  PlayerMap m_players;
  PlayerMap m_recycleBin;
};

}

#endif /* NOSONAPPZONESMODEL_H */

