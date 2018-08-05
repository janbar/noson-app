/*
 *      Copyright (C) 2018 Jean-Luc Barriere
 *
 *  This file is part of Noson-App
 *
 *  Noson-App is free software: you can redistribute it and/or modify
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
 *  along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef COMPOSERSMODEL_H
#define COMPOSERSMODEL_H

#include "listmodel.h"

#include <QAbstractListModel>

class ComposerItem
{
public:
  ComposerItem(const SONOS::DigitalItemPtr& ptr, const QString& baseURL);

  virtual ~ComposerItem() { }

  bool isValid() const { return m_valid; };

  QVariant payload() const;

  const QString& id() const { return m_id; }

  const QString& composer() const { return m_composer; }

  const QString& art() const { return m_art; }

  const QString& normalized() const { return m_normalized; }

private:
  SONOS::DigitalItemPtr m_ptr;
  bool m_valid;
  QString m_id;
  QString m_composer;
  QString m_art;
  QString m_normalized;
};

class ComposersModel : public QAbstractListModel, public ListModel
{
  Q_OBJECT
  Q_PROPERTY(int count READ rowCount NOTIFY countChanged)

public:
  enum ComposerRoles
  {
    PayloadRole,
    IdRole,
    ComposerRole,
    ArtRole,
    NormalizedRole,
  };

  ComposersModel(QObject* parent = 0);
  virtual ~ComposersModel();

  void addItem(ComposerItem* item);

  int rowCount(const QModelIndex& parent = QModelIndex()) const;

  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;

  Q_INVOKABLE QVariantMap get(int row);

  Q_INVOKABLE bool isNew() { return m_dataState == ListModel::New; }

  Q_INVOKABLE bool init(QObject* sonos, const QString& root, bool fill = false);

  virtual void clearData();

  virtual bool loadData();

  Q_INVOKABLE bool asyncLoad();

  Q_INVOKABLE void resetModel();

  Q_INVOKABLE void appendModel() { }

  Q_INVOKABLE void clearModel();

  virtual void handleDataUpdate();

  Q_INVOKABLE int containerUpdateID() { return m_updateID; }

signals:
  void dataUpdated();
  void countChanged();
  void loaded(bool succeeded);

protected:
  QHash<int, QByteArray> roleNames() const;

private:
  QList<ComposerItem*> m_items;
  QList<ComposerItem*> m_data;
};

#endif /* COMPOSERSMODEL_H */

