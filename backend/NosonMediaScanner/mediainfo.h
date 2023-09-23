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
#ifndef MEDIAINFO_H
#define MEDIAINFO_H

#include <QString>
#include <QStringList>

namespace mediascanner
{

struct MediaInfo
{
  QString title;
  QString artist;
  QString album;
  QString albumArtist;
  QString genre;
  QString composer;
  int trackNo;
  int year;
  bool hasArt;
  bool isDeleted;
  QString container;
  QString codec;
  int channels;
  int sampleRate;
  int bitRate;
  int duration;

  MediaInfo()
  : trackNo(0), year(0), hasArt(false), isDeleted(false), channels(0), sampleRate(0), bitRate(0), duration(0)
  {}
};

typedef QSharedPointer<MediaInfo> MediaInfoPtr;

}

#endif /* MEDIAINFO_H */

