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

#ifndef NOSONAPPROOMSMODEL_H
#define NOSONAPPROOMSMODEL_H

#include <noson/sonosplayer.h>

#include <QAbstractListModel>

namespace nosonapp
{

class Sonos;

class RoomItem
{
public:
  RoomItem(const SONOS::ZonePlayerPtr& ptr);

  virtual ~RoomItem() { }

  bool isValid() const { return m_valid; }

  QVariant payload() const;

  const QString& id() const { return m_id; }

  const QString& name() const { return m_name; }

  const QString& icon() const { return m_icon; }

  bool coordinator() const { return m_coordinator; }

private:
  SONOS::ZonePlayerPtr m_ptr;
  bool m_valid;
  QString m_id;
  QString m_name;
  QString m_icon;
  bool m_coordinator;

};

class RoomsModel : public QAbstractListModel
{
  Q_OBJECT
  Q_PROPERTY(int count READ rowCount NOTIFY countChanged)

public:
  enum RoomsRoles
  {
    PayloadRole,
    IdRole,
    NameRole,
    IconRole,
    CoordinatorRole,
  };

  RoomsModel(QObject* parent = 0);
  virtual ~RoomsModel();

  void addItem(RoomItem* item);

  int rowCount(const QModelIndex& parent = QModelIndex()) const;

  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;

  Q_INVOKABLE QVariantMap get(int row);

  virtual void clearData();

  virtual bool loadData();

  Q_INVOKABLE bool load(QObject* sonos);

  Q_INVOKABLE bool load(QObject* sonos, const QString& zoneId);

  virtual void resetModel();

signals:
  void countChanged();

protected:
  QHash<int, QByteArray> roleNames() const;

private:
  QList<RoomItem*> m_items;
  QList<RoomItem*> m_data;
  Sonos* m_provider;
  QString m_zoneId;
};

}

#endif /* NOSONAPPROOMSMODEL_H */

