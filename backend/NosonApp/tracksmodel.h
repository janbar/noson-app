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

#ifndef NOSONAPPTRACKSMODEL_H
#define NOSONAPPTRACKSMODEL_H

#include "listmodel.h"
#include <noson/contentdirectory.h>

#include <QAbstractListModel>

namespace nosonapp
{

class Sonos;

class TrackItem
{
public:
  TrackItem(const SONOS::DigitalItemPtr& ptr, const QString& baseURL);

  virtual ~TrackItem() { }

  bool isValid() const { return m_valid; }

  QVariant payload() const;

  const QString& id() const { return m_id; }

  const QString& title() const { return m_title; }

  const QString& author() const { return m_author; }

  const QString& album() const { return m_album; }

  const QString& albumTrackNo() const { return m_albumTrackNo; }

  const QString& art() const { return m_art; }

  bool isService() const { return m_isService; }

  void setArt(const QString& art) { m_art = art; }

private:
  SONOS::DigitalItemPtr m_ptr;
  bool m_valid;
  QString m_id;
  QString m_title;
  QString m_author;
  QString m_album;
  QString m_albumTrackNo;
  QString m_art;
  bool m_isService;
};

class TracksModel : public QAbstractListModel, public ListModel<Sonos>
{
  Q_OBJECT
  Q_PROPERTY(int count READ rowCount NOTIFY countChanged)
  Q_PROPERTY(int totalCount READ totalCount NOTIFY totalCountChanged)
  Q_PROPERTY(int firstIndex READ firstIndex CONSTANT)

public:
  enum TrackRoles
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

  TracksModel(QObject* parent = 0);
  ~TracksModel() override;

  void addItem(TrackItem* item);

  int rowCount(const QModelIndex& parent = QModelIndex()) const override;

  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

  bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

  Q_INVOKABLE QVariantMap get(int row);

  Q_INVOKABLE bool isNew() { return m_dataState == DataStatus::DataBlank; }

  Q_INVOKABLE bool init(nosonapp::Sonos* provider, const QString& root, bool fill = false);

  void clearData() override;

  bool loadData() override;

  int totalCount() const { return m_totalCount; }

  int firstIndex() const { return 0; }

  Q_INVOKABLE bool fetchAt(int index);

  Q_INVOKABLE bool asyncLoad();

  bool loadMoreData(unsigned size = 0);

  Q_INVOKABLE bool asyncLoadMore();

  Q_INVOKABLE void resetModel();

  Q_INVOKABLE void appendModel();

  bool loadDataForContext(int id) override;

  void handleDataUpdate() override;

  Q_INVOKABLE int containerUpdateID() { return m_updateID; }

signals:
  void dataUpdated();
  void countChanged();
  void totalCountChanged();
  void loaded(bool succeeded);
  void viewUpdated();
  void loadedMore(bool succeeded);

protected:
  QHash<int, QByteArray> roleNames() const override;

private:
  QList<TrackItem*> m_items;
  QList<TrackItem*> m_data;

  SONOS::ContentDirectory* m_contentDirectory;
  SONOS::ContentList* m_contentList;
  SONOS::ContentList::iterator m_iterator;
  unsigned m_totalCount;
  unsigned m_fetchSize;
};

}

#endif /* NOSONAPPTRACKSMODEL_H */

