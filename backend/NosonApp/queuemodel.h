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

#ifndef QUEUEMODEL_H
#define QUEUEMODEL_H

#include "tracksmodel.h"
#include "listmodel.h"

class QueueModel : public QAbstractListModel, public ListModel
{
  Q_OBJECT
  Q_PROPERTY(int count READ rowCount NOTIFY countChanged)

public:
  enum QueueRoles
  {
    PayloadRole,
    IdRole,
    TitleRole,
    AuthorRole,
    AlbumRole,
    AlbumTrackNoRole,
    ArtRole,
    IsServiceRole,
  };

  QueueModel(QObject* parent = 0);
  virtual ~QueueModel();

  void addItem(TrackItem* item);

  int rowCount(const QModelIndex& parent = QModelIndex()) const;

  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;

  bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

  Q_INVOKABLE QVariantMap get(int row);

  Q_INVOKABLE bool isNew() { return m_dataState == ListModel::New; }

  Q_INVOKABLE bool init(QObject* sonos, const QString& root, bool fill = false);

  virtual void clearData();

  virtual bool loadData();

  Q_INVOKABLE bool asyncLoad();

  Q_INVOKABLE void resetModel();

  Q_INVOKABLE void appendModel() { }

  virtual void handleDataUpdate();

  Q_INVOKABLE int containerUpdateID() { return m_updateID; }

signals:
  void dataUpdated();
  void countChanged();
  void loaded(bool succeeded);

protected:
  QHash<int, QByteArray> roleNames() const;

private:
  QList<TrackItem*> m_items;
  QList<TrackItem*> m_data;
};

#endif /* QUEUEMODEL_H */

