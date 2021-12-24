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

#ifndef NOSONAPPSERVICESMODEL_H
#define NOSONAPPSERVICESMODEL_H

#include "listmodel.h"
#include <noson/musicservices.h>

#include <QAbstractListModel>

namespace nosonapp
{

class Sonos;

class ServiceItem
{
public:
  ServiceItem(const SONOS::SMServicePtr& ptr);

  virtual ~ServiceItem() { }

  bool isValid() const { return m_valid; }

  QVariant payload() const;

  const QString& id() const { return m_id; }

  const QString& title() const { return m_title; }

  const QString& icon() const { return m_icon; }

  const QString& nickName() const { return m_nickName; }

  const QString& normalized() const { return m_normalized; }

  const QString& type() const { return m_type; }

  const QString& serialNum() const { return m_serialNum; }

  const QString& auth() const { return m_auth; }

private:
  SONOS::SMServicePtr m_ptr;
  bool m_valid;
  QString m_id;
  QString m_title;
  QString m_icon;
  QString m_nickName;
  QString m_normalized;
  QString m_type;
  QString m_serialNum;
  QString m_auth;
};

class ServicesModel : public QAbstractListModel, public ListModel<Sonos>
{
  Q_OBJECT
  Q_PROPERTY(int count READ rowCount NOTIFY countChanged)
  Q_PROPERTY(bool failure READ dataFailure NOTIFY loaded)

public:
  enum ServiceRoles
  {
    PayloadRole,
    IdRole,
    TitleRole,
    IconRole,
    NickNameRole,
    NormalizedRole,
    TypeRole,
    SerialNumRole,
    AuthRole,
  };

  ServicesModel(QObject* parent = 0);
  virtual ~ServicesModel();

  void addItem(ServiceItem* item);

  int rowCount(const QModelIndex& parent = QModelIndex()) const;

  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;

  Q_INVOKABLE QVariantMap get(int row);

  Q_INVOKABLE bool isNew() { return m_dataState == DataStatus::DataBlank; }

  Q_INVOKABLE bool init(nosonapp::Sonos* provider, bool fill = false) { return ListModel<Sonos>::configure(provider, fill); }

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
  QList<ServiceItem*> m_items;
  QList<ServiceItem*> m_data;
};

}

#endif /* NOSONAPPSERVICESMODEL_H */

