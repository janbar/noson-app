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

#ifndef NOSONAPPQUEUEMODEL_H
#define NOSONAPPQUEUEMODEL_H

#include "tracksmodel.h"
#include "listmodel.h"

namespace nosonapp
{

class Player;

class QueueModel : public QAbstractListModel, public ListModel<Player>
{
  Q_OBJECT
  Q_PROPERTY(int count READ rowCount NOTIFY countChanged)
  Q_PROPERTY(bool failure READ dataFailure NOTIFY loaded)
  Q_PROPERTY(int dataState READ dataState NOTIFY loaded)
  Q_PROPERTY(int totalCount READ totalCount NOTIFY totalCountChanged)
  Q_PROPERTY(int firstIndex READ firstIndex NOTIFY loaded)

public:
  enum QueueRoles
  {
    PayloadRole,
    TrackIndexRole,
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

  Q_INVOKABLE bool isNew() { return m_dataState == DataStatus::DataBlank; }

  Q_INVOKABLE bool init(nosonapp::Player* provider, bool fill = false);

  virtual void clearData();

  virtual bool loadData();

  int totalCount() const { return m_totalCount; }

  int firstIndex() const { return m_firstIndex; }

  Q_INVOKABLE bool fetchAt(int index);

  Q_INVOKABLE bool fetchBack();

  Q_INVOKABLE bool fetchFront();

  Q_INVOKABLE bool asyncLoad();

  Q_INVOKABLE void resetModel();

  virtual void handleDataUpdate();

  Q_INVOKABLE int containerUpdateID() { return m_updateID; }

signals:
  void dataUpdated();
  void countChanged();
  void loaded(bool succeeded);
  void viewUpdated();
  void totalCountChanged();

protected:
  QHash<int, QByteArray> roleNames() const;

private:
  QList<TrackItem*> m_items;
  QList<TrackItem*> m_data;

  SONOS::ContentDirectory * m_content = nullptr;
  SONOS::ContentBrowser * m_browser = nullptr;
  unsigned m_totalCount = 0;
  unsigned m_firstIndex = 0;
  unsigned m_fetchIndex = 0;
};

}

#endif /* NOSONAPPQUEUEMODEL_H */

