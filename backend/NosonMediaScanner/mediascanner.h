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
#ifndef MEDIASCANNER_H
#define MEDIASCANNER_H

#include "mediafile.h"

#include <QObject>
#include <QString>
#include <QList>

/*
 * By default all supported media are processed
 * FLAC-Vorbis parser cannot be disabled
 */
#ifndef DISABLE_ID3PARSER /* MP3 & AAC */
#define ENABLE_ID3PARSER
#endif
#ifndef DISABLE_MP4PARSER /* M4A */
#define ENABLE_MP4PARSER
#endif
#ifndef DISABLE_OGGPARSER /* OGG-Vorbis */
#define ENABLE_OGGPARSER
#endif

#define MEDIASCANNER_MAX_THREAD 2

Q_DECLARE_METATYPE(mediascanner::MediaFilePtr)

namespace mediascanner
{

class MediaDatabase;
class MediaScannerEngine;
class ListModel;

class MediaScanner : public QObject
{
  Q_OBJECT
  Q_PROPERTY(bool emptyState READ emptyState NOTIFY emptyStateChanged)
  Q_PROPERTY(bool working READ working NOTIFY workingChanged)

private:
    static MediaScanner * _instance;
    explicit MediaScanner(QObject * parent = nullptr);

public:
  static MediaScanner * instance(QObject * parent = nullptr);
  virtual ~MediaScanner();

  Q_INVOKABLE void start(int maxThread = MEDIASCANNER_MAX_THREAD);
  Q_INVOKABLE void debug(bool enable);
  bool isDebug() const { return m_debug; }
  bool emptyState() const;
  bool working() const;

  void registerModel(ListModel * model);
  void unregisterModel(ListModel * model);
  QList<MediaFilePtr> allParsedFiles() const;

  Q_INVOKABLE bool addRootPath(const QString& dirPath);
  Q_INVOKABLE bool removeRootPath(const QString& dirPath);
  Q_INVOKABLE void clearRoots();

signals:
  void emptyStateChanged();
  void workingChanged();
  void put(mediascanner::MediaFilePtr filePtr);
  void remove(mediascanner::MediaFilePtr filePtr);

private:
  MediaScannerEngine * m_engine;
  bool m_debug;
};

}

#endif /* MEDIASCANNER_H */
