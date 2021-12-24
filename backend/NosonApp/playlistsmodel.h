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

#ifndef NOSONAPPPLAYLISTSMODEL_H
#define NOSONAPPPLAYLISTSMODEL_H

#include "listmodel.h"

#include <QAbstractListModel>

namespace nosonapp
{

class Sonos;

class PlaylistItem
{
public:
  PlaylistItem(const SONOS::DigitalItemPtr& ptr, const QString& baseURL);

  virtual ~PlaylistItem() { }

  bool isValid() const { return m_valid; }

  QVariant payload() const;

  const QString& id() const { return m_id; }

  const QString& title() const { return m_title; }

  int artsCount() const { return m_arts.size(); }

  QString art(int index) const { return (m_arts.size() > index ? m_arts[index] : ""); }

  QStringList arts() const { return QStringList(m_arts); }

  const QString& normalized() const { return m_normalized; }

private:
  SONOS::DigitalItemPtr m_ptr;
  bool m_valid;
  QString m_id;
  QString m_title;
  QList<QString> m_arts;
  QString m_normalized;
};

class PlaylistsModel : public QAbstractListModel, public ListModel<Sonos>
{
  Q_OBJECT
  Q_PROPERTY(int count READ rowCount NOTIFY countChanged)
  Q_PROPERTY(bool failure READ dataFailure NOTIFY loaded)

public:
  enum PlaylistRoles
  {
    PayloadRole,
    IdRole,
    TitleRole,
    ArtRole,
    NormalizedRole,
    ArtsRole,
  };

  PlaylistsModel(QObject* parent = 0);
  virtual ~PlaylistsModel();

  void addItem(PlaylistItem* item);

  int rowCount(const QModelIndex& parent = QModelIndex()) const;

  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;

  Q_INVOKABLE QVariantMap get(int row);

  Q_INVOKABLE bool isNew() { return DataStatus::DataBlank; }

  Q_INVOKABLE bool init(nosonapp::Sonos* provider, const QString& root, bool fill = false);

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
  QList<PlaylistItem*> m_items;
  QList<PlaylistItem*> m_data;
};

}

#endif /* NOSONAPPPLAYLISTSMODEL_H */

