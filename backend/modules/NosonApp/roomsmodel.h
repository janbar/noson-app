/*
 *      Copyright (C) 2016 Jean-Luc Barriere
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

#ifndef ROOMSMODEL_H
#define ROOMSMODEL_H

#include "listmodel.h"
#include "../../lib/noson/noson/src/sonosplayer.h"

#include <QAbstractListModel>

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

class RoomsModel : public QAbstractListModel, public ListModel
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

  Q_INVOKABLE bool init(QObject* sonos, bool fill = false);

  Q_INVOKABLE void clear();

  Q_INVOKABLE bool load();

  Q_INVOKABLE bool load(const QString& zoneId);

  virtual void handleDataUpdate();

signals:
  void dataUpdated();
  void countChanged();

protected:
  QHash<int, QByteArray> roleNames() const;

private:
  QList<RoomItem*> m_items;
};

#endif /* ROOMSMODEL_H */

