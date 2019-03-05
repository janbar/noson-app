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
#include "mediafile.h"
#include "mediainfo.h"
#include "mediaparser.h"
#include "mediarunnable.h"
#include "mediascanner.h"

#include <QThread>
#include <QThreadPool>
#include <QMutex>
#include <QList>
#include <QSet>
#include <QMultiMap>
#include <QQueue>
#include <QFileInfo>
#include <QFileSystemWatcher>

namespace mediascanner
{

class MediaDatabase;

class MediaScannerEngine : public QThread
{
public:
  explicit MediaScannerEngine(MediaScanner * scanner, QObject* parent = nullptr);
  virtual ~MediaScannerEngine() override;

  void addParser(MediaParser * parser);
  void delParser(const QString& name);
  QList<MediaParserPtr> parsers();

  void setMaxThread(int maxThread);
  void stop();

  QList<MediaFilePtr> allParsedFiles() const;

private slots:
  void onFileChanged(const QString& filePath);
  void onDirectoryChanged(const QString& dirPath);

private:

  void run() override;
  void clearDelayedJobs();
  void scanDir(const QString& dirPath, const QList<MediaParserPtr>& parsers, bool deepScan = false);

  typedef QMap<QString, MediaFilePtr> NodeMap;
  typedef QMultiMap<QString, MediaFilePtr> FileMap;

  void resetNode(const QString& nodeName);
  void cleanNode(const QString& nodeName, bool evenPinned, QList<FileMap::iterator>& cleaned);

  void scheduleExtractor(MediaFilePtr filePtr, bool wait = true);
  static void mediaExtractorCallback(void * handle, MediaFilePtr& filePtr);
  static MediaParserPtr matchParser(const QList<MediaParserPtr>& parsers, const QFileInfo& fileInfo);

  MediaScanner * m_scanner;
  unsigned m_sequence;
  NodeMap m_nodes;
  NodeMap m_items;
  FileMap m_files;
  QStringList m_paths;
  QFileSystemWatcher m_watcher;
  QList<MediaParserPtr> m_parsers;
  QThreadPool m_workerPool;
  QMutex * m_mutex;

  QQueue<MediaRunnable*> m_delayedJobs;
};

}
