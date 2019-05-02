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

class ZoneItem
{
public:
  ZoneItem(const SONOS::ZonePtr& ptr);

  virtual ~ZoneItem() { }

  bool isValid() const { return m_valid; }

  QVariant payload() const;
  
  const QString& id() const { return m_id; }

  const QString& name() const { return m_name; }

  const QString& icon() const { return m_icon; }

  bool isGroup() const { return m_isGroup; }

  const QString& shortName() const { return m_shortName; }

private:
  SONOS::ZonePtr m_ptr;
  bool m_valid;
  QString m_id;
  QString m_name;
  QString m_icon;
  bool m_isGroup;
  QString m_shortName;
};

class ZonesModel : public QAbstractListModel, public ListModel
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
  };

  ZonesModel(QObject* parent = 0);
  virtual ~ZonesModel();

  void addItem(ZoneItem* item);

  int rowCount(const QModelIndex& parent = QModelIndex()) const;

  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;

  Q_INVOKABLE QVariantMap get(int row);

  Q_INVOKABLE bool isNew() { return m_dataState == ListModel::New; }

  Q_INVOKABLE bool init(QObject* sonos, bool fill = false) { return ListModel::init(sonos, fill); }

  virtual void clearData();

  virtual bool loadData();

  Q_INVOKABLE bool asyncLoad();

  Q_INVOKABLE void resetModel();

  Q_INVOKABLE void appendModel() { }

  virtual void handleDataUpdate();

signals:
  void dataUpdated();
  void countChanged();
  void loaded(bool succeeded);

protected:
  QHash<int, QByteArray> roleNames() const;

private:
  QList<ZoneItem*> m_items;
  QList<ZoneItem*> m_data;
};

}

#endif /* NOSONAPPZONESMODEL_H */

