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
#ifndef MEDIAFILE_H
#define MEDIAFILE_H

#include "mediaparser.h"
#include "mediainfo.h"

#include <QString>
#include <QDateTime>

namespace mediascanner
{

struct MediaFile
{
  unsigned fileId;
  bool isPinned;
  bool isDirectory;
  QString filePath;
  QString baseName;
  QString suffix;
  QString path;
  qint64 size;
  QDateTime lastModified;
  MediaParserPtr parser;
  int retry;
  bool isValid;
  bool signaled;
  MediaInfoPtr mediaInfo;

  MediaFile(unsigned id)
  : fileId(id)
  , isPinned(false)
  , isDirectory(false)
  , retry(0)
  , isValid(false)
  , signaled(false)
  , mediaInfo(nullptr)
  { }
};

typedef QSharedPointer<MediaFile> MediaFilePtr;

}

#endif /* MEDIAFILE_H */

