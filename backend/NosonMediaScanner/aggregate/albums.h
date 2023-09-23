/*
 *      Copyright (C) 2019 Jean-Luc Barriere
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#ifndef ALBUMS_H
#define ALBUMS_H

#include "aggregate.h"
#include "listmodel.h"
#include "tools.h"

namespace mediascanner
{

class AlbumModel : public Model
{
public:
  AlbumModel(const MediaFilePtr& file);
  const QByteArray& key() const { return m_key; }
  const QString& artist() { return m_file->mediaInfo->albumArtist; }
  const QString& album() { return m_file->mediaInfo->album; }
  const QString& filePath() { return m_file->filePath; }
  int year() { return m_file->mediaInfo->year; }
  bool hasArt() { return m_file->mediaInfo->hasArt; }
  const QString& normalized() { return m_normalized; }
  QVariant payload() const;
private:
  QByteArray m_key;
  QString m_normalized;
};

class Albums : public ListModel
{
  Q_OBJECT
  Q_PROPERTY(int count READ rowCount NOTIFY countChanged)
  Q_PROPERTY(QString artist READ artistFilter WRITE setArtistFilter NOTIFY artistChanged)
  Q_PROPERTY(QString composer READ composerFilter WRITE setComposerFilter NOTIFY composerChanged)

  typedef Aggregate<AlbumModel> AggregateType;
  typedef AggregateType::TuplePtr ItemPtr;

public:

  enum Roles
  {
    PayloadRole,
    IdRole,
    ArtistRole,
    AlbumRole,
    FilePathRole,
    YearRole,
    HasArtRole,
    NormalizedRole,
    ComposerRole,
  };

  Albums(QObject* parent = nullptr);
  virtual ~Albums() override;

  const QString& artistFilter() { return m_artistFilter; }
  void setArtistFilter(const QString& filter) { m_artistFilter = filter; emit artistChanged(); }

  const QString& composerFilter() { return m_composerFilter; }
  void setComposerFilter(const QString& filter) { m_composerFilter = filter; emit composerChanged(); }

  void addItem(ItemPtr& item);

  void removeItem(const QByteArray& key);

  int rowCount(const QModelIndex& parent = QModelIndex()) const override;

  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

  bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

  Q_INVOKABLE QVariantMap get(int row);

  Q_INVOKABLE bool isNew() { return m_dataState == ListModel::New; }

  Q_INVOKABLE bool init(bool fill = true) override { return ListModel::init(fill); }

  Q_INVOKABLE void clear() override;

  Q_INVOKABLE bool load() override;

  void onFileAdded(const MediaFilePtr& file) override;
  void onFileRemoved(const MediaFilePtr& file) override;

signals:
  void countChanged();
  void loaded(bool succeeded);
  void dataUpdated();

  void artistChanged();
  void composerChanged();

protected:
  QHash<int, QByteArray> roleNames() const override;

private:
  AggregateType m_data;
  QList<ItemPtr> m_items;
  QString m_artistFilter;
  QString m_composerFilter;
};

}

Q_DECLARE_METATYPE(mediascanner::Aggregate<mediascanner::AlbumModel>::TuplePtr);

#endif // ALBUMS_H
