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
#ifndef TRACKS_H
#define TRACKS_H

#include "aggregate.h"
#include "listmodel.h"
#include "tools.h"

namespace mediascanner
{

class TrackModel : public Model
{
public:
  TrackModel(const MediaFilePtr& file);
  const QByteArray& key() const { return m_key; }
  const QString& title() { return m_file->mediaInfo->title; }
  const QString& author() { return m_file->mediaInfo->artist; }
  const QString& album() { return m_file->mediaInfo->album; }
  const QString& albumArtist() { return m_file->mediaInfo->albumArtist; }
  const QString& genre() { return m_file->mediaInfo->genre; }
  const QString& composer() { return m_file->mediaInfo->composer; }
  const QString& codec() { return m_file->mediaInfo->codec; }
  const QString& filePath() { return m_file->filePath; }
  int albumTrackNo() { return m_file->mediaInfo->trackNo; }
  int albumDiscNo() { return m_file->mediaInfo->discNo; }
  int year() { return m_file->mediaInfo->year; }
  int duration() { return m_file->mediaInfo->duration; }
  int sampleRate() { return m_file->mediaInfo->sampleRate; }
  int channels() { return m_file->mediaInfo->channels; }
  int bitRate() { return m_file->mediaInfo->bitRate; }
  bool hasArt() { return m_file->mediaInfo->hasArt; }
  const QString& normalized() { return m_normalized; }
  int position() { return m_position; }
  const QString& art() const { return m_art; }
  void setArt(const QString& art) { m_art = art; }
  QVariant payload() const;
private:
  QByteArray m_key;
  QString m_normalized;
  QString m_art;
  int m_position;
};

class Tracks : public ListModel
{
  Q_OBJECT
  Q_PROPERTY(int count READ rowCount NOTIFY countChanged)
  Q_PROPERTY(QString artist READ artistFilter WRITE setArtistFilter NOTIFY artistChanged)
  Q_PROPERTY(QString album READ albumFilter WRITE setAlbumFilter NOTIFY albumChanged)
  Q_PROPERTY(QString albumArtist READ albumArtistFilter WRITE setAlbumArtistFilter NOTIFY albumChanged)
  Q_PROPERTY(QString genre READ genreFilter WRITE setGenreFilter NOTIFY genreChanged)
  Q_PROPERTY(QString composer READ composerFilter WRITE setComposerFilter NOTIFY composerChanged)

  typedef Aggregate<TrackModel> AggregateType;
  typedef AggregateType::TuplePtr ItemPtr;

public:

  enum Roles
  {
    PayloadRole,
    IdRole,
    TitleRole,
    AlbumRole,
    AlbumArtistRole,
    AuthorRole,
    GenreRole,
    ComposerRole,
    CodecRole,
    FilePathRole,
    AlbumTrackNoRole,
    AlbumDiscNoRole,
    YearRole,
    DurationRole,
    SampleRateRole,
    ChannelsRole,
    BitRateRole,
    HasArtRole,
    ArtRole,
    NormalizedRole,
    PositionRole,
  };

  Tracks(QObject* parent = nullptr);
  virtual ~Tracks() override;

  const QString& artistFilter() { return m_artistFilter; }
  void setArtistFilter(const QString& filter) { m_artistFilter = filter; emit artistChanged(); }
  const QString& albumFilter() { return m_albumFilter; }
  void setAlbumFilter(const QString& filter) { m_albumFilter = filter; emit albumChanged(); }
  const QString& albumArtistFilter() { return m_albumArtistFilter; }
  void setAlbumArtistFilter(const QString& filter) { m_albumArtistFilter = filter; emit albumChanged(); }
  const QString& genreFilter() { return m_genreFilter; }
  void setGenreFilter(const QString& filter) { m_genreFilter = filter; emit genreChanged(); }
  const QString& composerFilter() { return m_composerFilter; }
  void setComposerFilter(const QString& filter) { m_composerFilter = filter; emit composerChanged(); }

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
  void albumChanged();
  void genreChanged();
  void composerChanged();

protected:
  QHash<int, QByteArray> roleNames() const override;

private:
  AggregateType m_data;
  QList<ItemPtr> m_items;
  QString m_artistFilter;
  QString m_albumFilter;
  QString m_albumArtistFilter;
  QString m_genreFilter;
  QString m_composerFilter;

  // unlocked
  void checkAndAdd(const MediaFilePtr& file);
  void addItem(ItemPtr& item);
  void removeItem(const QByteArray& key);
};

}

Q_DECLARE_METATYPE(mediascanner::Aggregate<mediascanner::TrackModel>::TuplePtr);

#endif /* TRACKS_H */

