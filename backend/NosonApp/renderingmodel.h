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

#ifndef NOSONAPPRENDERINGMODEL_H
#define NOSONAPPRENDERINGMODEL_H

#include "player.h"

#include <QAbstractListModel>

namespace nosonapp
{

class RenderingItem
{
public:
  RenderingItem(const Player::RCProperty& rcp);

  virtual ~RenderingItem() { }

  const QString& uuid() const { return m_uuid; }

  const QString& name() const { return m_name; }

  double volume() const { return m_volume; }

  void setVolume(double volume) { m_volume = volume; }

  bool mute() const { return m_mute; }

  void setMute(bool mute) { m_mute = mute; }

  bool outputFixed() const { return m_outputFixed; }

  void setOutputFixed(bool fixed) { m_outputFixed = fixed; }

private:
  QString m_uuid;
  QString m_name;
  double m_volume;
  bool m_mute;
  bool m_outputFixed;
};

class RenderingModel : public QAbstractListModel
{
  Q_OBJECT
  Q_PROPERTY(int count READ rowCount NOTIFY countChanged)

public:
  enum RenderingRoles
  {
    UUIDRole,
    NameRole,
    VolumeRole,
    MuteRole,
    OutputFixedRole,
  };

  RenderingModel(QObject* parent = 0);
  virtual ~RenderingModel();

  void addItem(RenderingItem* item);

  int rowCount(const QModelIndex& parent = QModelIndex()) const;

  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;

  bool setData(const QModelIndex& index, const QVariant& value, int role);

  virtual void clearData();

  virtual bool loadData();

  Q_INVOKABLE bool load(nosonapp::Player* player);

  virtual void resetModel();

  Q_INVOKABLE void setVolume(int index, const QVariant& volume);

  Q_INVOKABLE void setMute(int index, const QVariant& mute);

  Q_INVOKABLE void setOutputFixed(int index, const QVariant& fixed);

signals:
  void countChanged();

protected:
  QHash<int, QByteArray> roleNames() const;

private:
  QList<RenderingItem*> m_items;
  QList<RenderingItem*> m_data;
  Player* m_player;
};

}

#endif /* NOSONAPPRENDERINGMODEL_H */

