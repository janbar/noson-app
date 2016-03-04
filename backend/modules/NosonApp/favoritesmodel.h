/*
 *      Copyright (C) 2015-2016 Jean-Luc Barriere
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

#ifndef FAVORITESMODEL_H
#define FAVORITESMODEL_H

#include "listmodel.h"

#include <QAbstractListModel>

class FavoriteType : public QObject
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
    };
};

class FavoriteItem
{
public:
  FavoriteItem(const SONOS::DigitalItemPtr& ptr, const QString& baseURL);

  virtual ~FavoriteItem() { }

  bool isValid() const { return m_valid; }

  QVariant payload() const;

  const QString& id() const { return m_id; }

  const QString& title() const { return m_title; }

  const QString& description() const { return m_description; }

  const QString& art() const { return m_art; }

  const QString& normalized() const { return m_normalized; }

  QVariant object() const;

  const QString& objectId() const { return m_objectId; }

  int type() const { return m_type; }

  bool canQueue() const { return m_canQueue; }

  const QString artist() const { return m_artist; }

  const QString album() const { return m_album; }

private:
  SONOS::DigitalItemPtr m_ptr;
  bool m_valid;
  QString m_id;
  QString m_title;
  QString m_description;
  QString m_art;
  QString m_normalized;
  SONOS::DigitalItemPtr m_objectPtr;
  QString m_objectId;
  FavoriteType::itemType m_type;
  bool m_canQueue;
  QString m_artist;
  QString m_album;
};

class FavoritesModel : public QAbstractListModel, public ListModel
{
  Q_OBJECT
  Q_PROPERTY(int count READ rowCount NOTIFY countChanged)

public:
  enum AnyRoles
  {
    PayloadRole,
    IdRole,
    TitleRole,
    DescriptionRole,
    ArtRole,
    NormalizedRole,
    ObjectRole,
    ObjectIdRole,
    TypeRole,
    CanQueueRole,
    ArtistRole,
    AlbumRole,
  };

  FavoritesModel(QObject* parent = 0);
  virtual ~FavoritesModel();

  void addItem(FavoriteItem* item);

  int rowCount(const QModelIndex& parent = QModelIndex()) const;

  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;

  Q_INVOKABLE QVariantMap get(int row);

  Q_INVOKABLE bool init(QObject* sonos, const QString& root, bool fill = false);

  Q_INVOKABLE void clear();

  Q_INVOKABLE bool load();

  Q_INVOKABLE bool asyncLoad();

  virtual void handleDataUpdate();

  Q_INVOKABLE int containerUpdateID() { return m_updateID; }

  Q_INVOKABLE QString findFavorite(const QString& objectID) const;

signals:
  void dataUpdated();
  void countChanged();

protected:
  QHash<int, QByteArray> roleNames() const;

private:
  QList<FavoriteItem*> m_items;
};

#endif /* FAVORITESMODEL_H */

