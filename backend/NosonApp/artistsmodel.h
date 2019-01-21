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

#ifndef ARTISTSMODEL_H
#define ARTISTSMODEL_H

#include "listmodel.h"

#include <QAbstractListModel>

class ArtistItem
{
public:
  ArtistItem(const SONOS::DigitalItemPtr& ptr, const QString& baseURL);

  virtual ~ArtistItem() { }

  bool isValid() const { return m_valid; };

  QVariant payload() const;

  const QString& id() const { return m_id; }

  const QString& artist() const { return m_artist; }

  const QString& art() const { return m_art; }

  const QString& normalized() const { return m_normalized; }

private:
  SONOS::DigitalItemPtr m_ptr;
  bool m_valid;
  QString m_id;
  QString m_artist;
  QString m_art;
  QString m_normalized;
};

class ArtistsModel : public QAbstractListModel, public ListModel
{
  Q_OBJECT
  Q_PROPERTY(int count READ rowCount NOTIFY countChanged)

public:
  enum ArtistRoles
  {
    PayloadRole,
    IdRole,
    ArtistRole,
    ArtRole,
    NormalizedRole,
  };

  ArtistsModel(QObject* parent = 0);
  virtual ~ArtistsModel();

  void addItem(ArtistItem* item);

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
  QList<ArtistItem*> m_items;
  QList<ArtistItem*> m_data;
};

#endif /* ARTISTSMODEL_H */

