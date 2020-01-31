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

#ifndef NOSONAPPALLSERVICESMODEL_H
#define NOSONAPPALLSERVICESMODEL_H

#include "listmodel.h"
#include "servicesmodel.h"

#include <QAbstractListModel>

namespace nosonapp
{

class AllServicesModel : public QAbstractListModel, public ListModel<Sonos>
{
  Q_OBJECT
  Q_PROPERTY(int count READ rowCount NOTIFY countChanged)
  Q_PROPERTY(bool failure READ dataFailure NOTIFY loaded)

public:
  enum AllServiceRoles
  {
    PayloadRole,
    IdRole,
    TitleRole,
    IconRole,
    NickNameRole,
    NormalizedRole,
    TypeRole,
    AuthRole,
  };

  AllServicesModel(QObject* parent = 0);
  virtual ~AllServicesModel();

  void addItem(ServiceItem* item);

  int rowCount(const QModelIndex& parent = QModelIndex()) const;

  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;

  Q_INVOKABLE QVariantMap get(int row);

  Q_INVOKABLE bool isNew() { return m_dataState == DataStatus::DataBlank; }

  Q_INVOKABLE bool init(Sonos* provider, bool fill = false) { return ListModel::configure(provider, fill); }

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

#endif /* NOSONAPPALLSERVICESMODEL_H */

