/*
 *      Copyright (C) 2015-2016 Jean-Luc Barriere
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

#ifndef MEDIAMODEL_H
#define MEDIAMODEL_H

#include "listmodel.h"
#include "../../lib/noson/noson/src/smapi.h"

#include <QAbstractListModel>
#include <QStack>
#include <QPair>

class MediaType : public QObject
{
  Q_OBJECT
  Q_ENUMS(itemType)

  public:
    enum itemType
    {
      unknown   = 0,
      album     = 1,
      person    = 2,
      genre     = 3,
      playlist  = 4,
      audioItem = 5,
      folder    = 6,
    };
};

class MediaItem
{
public:
  MediaItem(const SONOS::SMAPIItem& data);

  virtual ~MediaItem() { }

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

  const int displayType() const { return m_displayType; }

  const bool isContainer() const { return m_isContainer; }

private:
  SONOS::DigitalItemPtr m_ptr;
  bool m_valid;
  QString m_id;
  QString m_title;
  QString m_description;
  QString m_art;
  QString m_normalized;
  MediaType::itemType m_type;
  bool m_canQueue;
  bool m_canPlay;
  QString m_artist;
  QString m_album;
  QString m_parent;
  QString m_objectId;
  int m_displayType;
  bool m_isContainer;
};

class MediaAuth : public QObject
{
  Q_OBJECT
  Q_PROPERTY(QString type READ type)
  Q_PROPERTY(QString serialNum READ serialNum)
  Q_PROPERTY(QString key READ key)
  Q_PROPERTY(QString token READ token)

public:
  MediaAuth(QObject* parent = 0) : QObject(parent) { }
  virtual ~MediaAuth() { }

  QString type() { return QString::fromUtf8(m_auth.type.c_str()); }
  QString serialNum() { return QString::fromUtf8(m_auth.serialNum.c_str()); }
  QString key() { return QString::fromUtf8(m_auth.key.c_str()); }
  QString token() { return QString::fromUtf8(m_auth.token.c_str()); }

  void resetAuth(const SONOS::SMOAKeyring::OAuth& auth) { m_auth = auth; }

private:
  SONOS::SMOAKeyring::OAuth m_auth;
};

class MediaModel : public QAbstractListModel, public ListModel
{
  Q_OBJECT
  Q_PROPERTY(int count READ rowCount NOTIFY countChanged)
  Q_PROPERTY(int totalCount READ totalCount NOTIFY totalCountChanged)
  Q_PROPERTY(bool isRoot READ isRoot NOTIFY isRootChanged)
  Q_PROPERTY(bool isAuthExpired READ isAuthExpired NOTIFY authStatusChanged())

public:
  enum AnyRoles
  {
    PayloadRole,
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
  };

  MediaModel(QObject* parent = 0);
  virtual ~MediaModel();

  void addItem(MediaItem* item);

  int rowCount(const QModelIndex& parent = QModelIndex()) const;

  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;

  Q_INVOKABLE QVariantMap get(int row);

  Q_INVOKABLE bool init(QObject* sonos, const QVariant& service, bool fill = false);

  Q_INVOKABLE void clear();

  Q_INVOKABLE bool load();

  int totalCount() const { return m_totalCount; }

  bool isRoot() const { return m_path.empty(); }

  Q_INVOKABLE bool loadMore();

  Q_INVOKABLE bool loadChild(const QString& id, const QString& title, int displayType);

  Q_INVOKABLE bool loadParent();

  Q_INVOKABLE QString path() const;

  Q_INVOKABLE QString pathId() const;

  Q_INVOKABLE int previousDisplayType() const;

  bool isAuthExpired() const;

  Q_INVOKABLE QString beginDeviceRegistration();

  Q_INVOKABLE int requestDeviceAuth(); // 0: retry, 1: succeeded, 2: failed

  Q_INVOKABLE MediaAuth* getDeviceAuth();

  Q_INVOKABLE bool asyncLoad();

  virtual void handleDataUpdate();

  Q_INVOKABLE int containerUpdateID() { return m_updateID; }

signals:
  void dataUpdated();
  void countChanged();
  void totalCountChanged();
  void isRootChanged();
  void authStatusChanged();

protected:
  QHash<int, QByteArray> roleNames() const;

private:
  QList<MediaItem*> m_items;

  SONOS::SMAPI* m_smapi;
  SONOS::SMOAKeyring::OAuth m_auth;

  struct Path
  {
    Path(): id(), title(), displayType(0) {}
    Path(const QString& _id, const QString& _title, int dt) : id(_id), title(_title), displayType(dt) {}
    QString id;
    QString title;
    int displayType;
  };

  QStack<Path> m_path;
  unsigned m_nextIndex;
  unsigned m_totalCount;
};

#endif /* MEDIAMODEL_H */

