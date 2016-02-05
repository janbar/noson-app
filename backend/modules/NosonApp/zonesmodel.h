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

#ifndef ZONESMODEL_H
#define ZONESMODEL_H

#include "listmodel.h"
#include "../../lib/noson/noson/src/sonoszone.h"

#include <QAbstractListModel>

class ZoneItem : public SONOS::ZonePtr
{
public:
  ZoneItem(const SONOS::ZonePtr& ptr);

  virtual ~ZoneItem() { }

  bool isValid() const { return m_valid; }

  QString id() const { return m_id; }

  QString name() const { return m_name; }

  QString icon() const { return m_icon; }

private:
  bool m_valid;
  QString m_id;
  QString m_name;
  QString m_icon;
};

class ZonesModel : public QAbstractListModel, public ListModel
{
  Q_OBJECT
  Q_PROPERTY(int count READ rowCount NOTIFY countChanged)

public:
  enum ZonesRoles
  {
    IdRole,
    NameRole,
    IconRole,
  };

  ZonesModel(QObject* parent = 0);
  virtual ~ZonesModel();

  void addItem(ZoneItem* item);

  int rowCount(const QModelIndex& parent = QModelIndex()) const;

  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;

  Q_INVOKABLE bool init(QObject* sonos, bool fill = false);

  Q_INVOKABLE void clear();

  Q_INVOKABLE bool load();

  virtual void handleDataUpdate();

signals:
  void dataUpdated();
  void countChanged();

protected:
  QHash<int, QByteArray> roleNames() const;

private:
  QList<ZoneItem*> m_items;
};

#endif /* ZONESMODEL_H */

