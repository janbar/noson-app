/*
 *      Copyright (C) 2022 Jean-Luc Barriere
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

#ifndef NOSONAPPLIBRARYMODEL_H
#define NOSONAPPLIBRARYMODEL_H

#include "listmodel.h"
#include <noson/contentdirectory.h>

#include <QAbstractListModel>
#include <QStack>
#include <QPair>

namespace nosonapp
{

class Sonos;

class LibraryItem
{
public:
  LibraryItem(const SONOS::DigitalItemPtr& data, const QString& baseURL);

  virtual ~LibraryItem() { }

  bool isValid() const { return m_valid; }

  QVariant payload() const;

  const QString& id() const { return m_id; }

  const QString& title() const { return m_title; }

  const QString& description() const { return m_description; }

  const QString& art() const { return m_art; }

  const QString& normalized() const { return m_normalized; }

  int type() const { return m_type; }

  bool canQueue() const { return m_canQueue; }

  bool canPlay() const { return m_canPlay; }

  const QString& artist() const { return m_artist; }

  const QString& album() const { return m_album; }

  const QString& parent() const { return m_parent; }

  const QString& objectId() const { return m_objectId; }

  int displayType() const { return m_displayType; }

  bool isContainer() const { return m_isContainer; }

  int albumTrackNo() const { return m_albumTrackNo; }

private:
  SONOS::DigitalItemPtr m_ptr;
  bool m_valid;
  QString m_id;
  QString m_title;
  QString m_description;
  QString m_art;
  QString m_normalized;
  int m_type;
  bool m_canQueue;
  bool m_canPlay;
  QString m_artist;
  QString m_album;
  QString m_parent;
  QString m_objectId;
  int m_displayType;
  bool m_isContainer;
  int m_albumTrackNo;
};

class LibraryModel : public QAbstractListModel, public ListModel<Sonos>
{
  Q_OBJECT
  Q_PROPERTY(int count READ rowCount NOTIFY countChanged)
  Q_PROPERTY(bool failure READ dataFailure NOTIFY loaded)
  Q_PROPERTY(int totalCount READ totalCount NOTIFY totalCountChanged)
  Q_PROPERTY(int firstIndex READ firstIndex NOTIFY loaded)
  Q_PROPERTY(bool isRoot READ isRoot NOTIFY pathChanged)

public:
  enum AnyRoles
  {
    PayloadRole,
    ItemIndexRole,
    IdRole,
    TitleRole,
    DescriptionRole,
    ArtRole,
    NormalizedRole,
    TypeRole,
    CanQueueRole,
    CanPlayRole,
    ArtistRole,
    AlbumRole,
    ParentRole,
    ObjectIdRole,
    DisplayTypeRole,
    IsContainerRole,
    AlbumTrackNoRole,
  };

  enum NodeType
  {
    NodeUnknown   = 0,
    NodeAlbum     = 1,
    NodePerson    = 2,
    NodeGenre     = 3,
    NodePlaylist  = 4,
    NodeAudioItem = 5,
    NodeFolder    = 6,
    NodePlayable  = 7, // dummy playable
  };

  Q_ENUM(NodeType)

  enum DisplayType
  {
    DisplayUnknown   = 0,
    DisplayGrid      = 1,
    DisplayTrackList = 2,
    DisplayItemList  = 3,
  };

  Q_ENUM(DisplayType)

  LibraryModel(QObject* parent = 0);
  virtual ~LibraryModel();

  void addItem(LibraryItem* item);

  int rowCount(const QModelIndex& parent = QModelIndex()) const;

  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;

  Q_INVOKABLE QVariantMap get(int row);

  Q_INVOKABLE bool isNew() { return m_dataState == DataStatus::DataBlank; }

  Q_INVOKABLE bool init(nosonapp::Sonos* provider, const QString& root, bool fill = false, int display = DisplayUnknown, int node = NodeUnknown);

  Q_INVOKABLE void clearData();

  Q_INVOKABLE bool loadData();

  int totalCount() const { return m_totalCount; }

  int firstIndex() const { return m_firstIndex; }

  Q_INVOKABLE bool fetchAt(int index);

  Q_INVOKABLE bool fetchBack();

  Q_INVOKABLE bool fetchFront();

  Q_INVOKABLE bool asyncLoad();

  Q_INVOKABLE void resetModel();

  bool isRoot() const { return (m_path.size() == 1); }

  Q_INVOKABLE QString pathName() const;

  Q_INVOKABLE QString pathId() const;

  Q_INVOKABLE int displayType() const;

  Q_INVOKABLE int nodeType() const;

  Q_INVOKABLE int viewIndex() const;

  Q_INVOKABLE QList<QString> listSearchCategories() const;

  virtual bool loadChild(const QString& id, const QString& title, int displayType, int nodeType, int viewIndex = 0);

  Q_INVOKABLE bool asyncLoadChild(const QString& id, const QString& title, int displayType, int nodeType, int viewIndex = 0);

  virtual bool loadParent();

  Q_INVOKABLE bool asyncLoadParent();

  virtual bool loadSearch(const QString& category, const QString& term);

  Q_INVOKABLE bool asyncLoadSearch(const QString& category, const QString& term);

  virtual bool loadDataForContext(int id);

  virtual void handleDataUpdate();

  Q_INVOKABLE int containerUpdateID() { return m_updateID; }

  Q_INVOKABLE QString faultString();

signals:
  void dataUpdated();
  void countChanged();
  void loaded(bool succeeded);
  void viewUpdated();
  void totalCountChanged();
  void pathChanged();

protected:
  QHash<int, QByteArray> roleNames() const;

private:
  QList<LibraryItem*> m_items;
  QList<LibraryItem*> m_data;

  SONOS::ContentDirectory * m_content = nullptr;
  SONOS::ContentBrowser * m_browser = nullptr;
  unsigned m_totalCount = 0;
  unsigned m_firstIndex = 0;
  unsigned m_fetchIndex = 0;

  struct Path
  {
    Path() {}
    explicit Path(const QString& _id, const QString& _title, int _displayType, int _nodeType)
    : id(_id), title(_title), displayType(_displayType), nodeType(_nodeType) {}
    QString id;
    QString title;
    int displayType = DisplayUnknown;
    int nodeType    = NodeUnknown;
    int fetchIndex  = 0;
    int viewIndex   = 0;
  };

  QStack<Path> m_path;
};

}

#endif /* NOSONAPPLIBRARYMODEL_H */

