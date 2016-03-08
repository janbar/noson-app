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

#ifndef RENDERINGMODEL_H
#define RENDERINGMODEL_H

#include "player.h"

#include <QAbstractListModel>

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

private:
  QString m_uuid;
  QString m_name;
  double m_volume;
  bool m_mute;
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
  };

  RenderingModel(QObject* parent = 0);
  virtual ~RenderingModel();

  void addItem(RenderingItem* item);

  int rowCount(const QModelIndex& parent = QModelIndex()) const;

  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;

  bool setData(const QModelIndex& index, const QVariant& value, int role);

  Q_INVOKABLE void clear();

  Q_INVOKABLE bool load(QObject* player);

  Q_INVOKABLE void setVolume(int index, const QVariant& volume);

  Q_INVOKABLE void setMute(int index, const QVariant& mute);

signals:
  void countChanged();

protected:
  QHash<int, QByteArray> roleNames() const;

private:
  QList<RenderingItem*> m_items;
};

#endif /* RENDERINGMODEL_H */

