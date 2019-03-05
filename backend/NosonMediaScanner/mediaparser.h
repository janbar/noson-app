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
#ifndef MEDIAPARSER_H
#define MEDIAPARSER_H

#include <QFileInfo>
#include <QSharedPointer>

namespace mediascanner
{

struct MediaFile;
struct MediaInfo;

class MediaParser
{
public:
  virtual ~MediaParser() {}
  virtual const char * commonName() = 0;
  virtual bool match(const QFileInfo& fileInfo) = 0;
  virtual bool parse(MediaFile * file, MediaInfo * info, bool debug) = 0;
};

typedef QSharedPointer<MediaParser> MediaParserPtr;

}

#endif /* MEDIAPARSER_H */

