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

#ifndef RADIOSMODEL_H
#define RADIOSMODEL_H

#include "listmodel.h"

#include <QAbstractListModel>

class RadioItem
{
public:
  RadioItem(const SONOS::DigitalItemPtr& ptr, const QString& baseURL);

  virtual ~RadioItem() { }

  bool isValid() const { return m_valid; }

  QVariant payload() const;

  QString id() const { return m_id; }

  QString title() const { return m_title; }

  QString genre() const { return m_genre; }

  QString icon() const { return m_icon; }

  QString streamId() const { return m_streamId; }

  QString normalized() const { return m_normalized; }

private:
  SONOS::DigitalItemPtr m_ptr;
  bool m_valid;
  QString m_id;
  QString m_title;
  QString m_genre;
  QString m_icon;
  QString m_streamId;
  QString m_normalized;
};

class RadiosModel : public QAbstractListModel, public ListModel
{
  Q_OBJECT
  Q_PROPERTY(int count READ rowCount NOTIFY countChanged)

public:
  enum RadioRoles
  {
    PayloadRole,
    IdRole,
    TitleRole,
    GenreRole,
    IconRole,
    StreamIdRole,
    NormalizedRole,
  };

  RadiosModel(QObject* parent = 0);
  virtual ~RadiosModel();

  void addItem(RadioItem* item);

  int rowCount(const QModelIndex& parent = QModelIndex()) const;

  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;

  Q_INVOKABLE QVariantMap get(int row);

  Q_INVOKABLE bool init(QObject* sonos, const QString& root, bool fill = false);

  Q_INVOKABLE void clear();

  Q_INVOKABLE bool load();

  Q_INVOKABLE bool asyncLoad();

  virtual void handleDataUpdate();

  Q_INVOKABLE int containerUpdateID() { return m_updateID; }

signals:
  void dataUpdated();
  void countChanged();

protected:
  QHash<int, QByteArray> roleNames() const;

private:
  QList<RadioItem*> m_items;
};

#endif /* RADIOSMODEL_H */

