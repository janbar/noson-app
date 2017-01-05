/*
 *      Copyright (C) 2017 Jean-Luc Barriere
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

#ifndef SERVICESMODEL_H
#define SERVICESMODEL_H

#include "listmodel.h"
#include "../../lib/noson/noson/src/musicservices.h"

#include <QAbstractListModel>

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

private:
  SONOS::SMServicePtr m_ptr;
  bool m_valid;
  QString m_id;
  QString m_title;
  QString m_icon;
  QString m_nickName;
  QString m_normalized;
};

class ServicesModel : public QAbstractListModel, public ListModel
{
  Q_OBJECT
  Q_PROPERTY(int count READ rowCount NOTIFY countChanged)

public:
  enum ServiceRoles
  {
    PayloadRole,
    IdRole,
    TitleRole,
    IconRole,
    NickNameRole,
    NormalizedRole,
  };

  ServicesModel(QObject* parent = 0);
  virtual ~ServicesModel();

  void addItem(ServiceItem* item);

  int rowCount(const QModelIndex& parent = QModelIndex()) const;

  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;

  Q_INVOKABLE QVariantMap get(int row);

  Q_INVOKABLE bool init(QObject* sonos, bool fill = false);

  Q_INVOKABLE void clear();

  Q_INVOKABLE bool load();

  Q_INVOKABLE bool asyncLoad();

  virtual void handleDataUpdate();

signals:
  void dataUpdated();
  void countChanged();

protected:
  QHash<int, QByteArray> roleNames() const;

private:
  QList<ServiceItem*> m_items;
};

#endif /* SERVICESMODEL_H */

