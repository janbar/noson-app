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
#include "mediascannerengine.h"
#include "mediaextractor.h"
#include "locked.h"

#include <QDebug>
#include <QStandardPaths>
#include <QDirIterator>
#include <QQueue>

#define THREAD_EXPIRY_TIMEOUT 10000
#define DEFAULT_MAX_THREAD    2
#define SCHEDULE_TIMEOUT_MS   10
#define FILE_MIN_SIZE         1024
#define RETRY_TIMEOUT_MS      5000
#define RETRY_MAX             3

using namespace mediascanner;

MediaScannerEngine::MediaScannerEngine(MediaScanner * scanner, QObject* parent)
: QThread(parent)
, m_scanner(scanner)
, m_sequence(0)
, m_mutex(new QMutex(QMutex::Recursive))
{
  m_paths = QStandardPaths::standardLocations(QStandardPaths::MusicLocation);
  m_workerPool.setExpiryTimeout(THREAD_EXPIRY_TIMEOUT);
  m_workerPool.setMaxThreadCount(DEFAULT_MAX_THREAD);
}

MediaScannerEngine::~MediaScannerEngine()
{
  m_workerPool.clear();
  delete m_mutex;
}

void MediaScannerEngine::addParser(MediaParser* parser)
{
  LockGuard g(m_mutex);
  for (MediaParserPtr p : m_parsers)
  {
    if (p->commonName() == parser->commonName())
      return;
  }
  m_parsers.push_back(MediaParserPtr(parser));
}

void MediaScannerEngine::delParser(const QString& name)
{
  LockGuard g(m_mutex);
  QList<MediaParserPtr>::iterator it = m_parsers.begin();
  while (it != m_parsers.end())
  {
    if ((*it)->commonName() == name)
    {
      m_parsers.erase(it);
      break;
    }
    ++it;
  }
}

QList<MediaParserPtr> MediaScannerEngine::parsers()
{
  LockGuard g(m_mutex);
  QList<MediaParserPtr> list;
  for (MediaParserPtr p : m_parsers)
    list.push_back(p);
  return list;
}

void MediaScannerEngine::setMaxThread(int maxThread)
{
  m_workerPool.setMaxThreadCount(maxThread);
}

void MediaScannerEngine::stop()
{
  QThread::requestInterruption();
  while (!QThread::isFinished())
    QThread::wait(1000);
}

QList<MediaFilePtr> MediaScannerEngine::allParsedFiles() const
{
  LockGuard g(m_mutex);
  QList<MediaFilePtr> list;
  for (const MediaFilePtr& file : m_files)
    if (file->isValid)
      list.push_back(file);
  return list;
}

void MediaScannerEngine::onFileChanged(const QString& filePath)
{
  QFile f(filePath);
  if (f.exists() && f.size() > FILE_MIN_SIZE)
  {
    NodeMap::iterator it = m_items.find(filePath);
    if (it != m_items.end())
      scheduleExtractor(it.value(), false);
    m_watcher.removePath(filePath);
  }
}

void MediaScannerEngine::onDirectoryChanged(const QString& dirPath)
{
  scanDir(dirPath, parsers(), false);
}

void MediaScannerEngine::run()
{
  connect(&m_watcher, &QFileSystemWatcher::directoryChanged, this, &MediaScannerEngine::onDirectoryChanged, Qt::ConnectionType::QueuedConnection);
  connect(&m_watcher, &QFileSystemWatcher::fileChanged, this, &MediaScannerEngine::onFileChanged);

  for (QString s : m_paths)
  {
    scanDir(s, parsers());
  }

  while (!isInterruptionRequested())
  {
    QThread::msleep(500);
    {
      LockGuard g(m_mutex);
      while (!m_delayedJobs.isEmpty() && !isInterruptionRequested())
      {
        if (m_delayedJobs.front()->timeLeft() > 0)
          break;
        m_workerPool.start(m_delayedJobs.dequeue());
      }
    }
  }

  disconnect(&m_watcher, &QFileSystemWatcher::fileChanged, this, &MediaScannerEngine::onFileChanged);
  disconnect(&m_watcher, &QFileSystemWatcher::directoryChanged, this, &MediaScannerEngine::onDirectoryChanged);
  m_workerPool.clear();
  clearDelayedJobs();
  m_items.clear();
}

void MediaScannerEngine::clearDelayedJobs()
{
  LockGuard g(m_mutex);
  while (!m_delayedJobs.isEmpty())
    delete m_delayedJobs.dequeue();
}

void MediaScannerEngine::scanDir(const QString &dirPath, const QList<MediaParserPtr>& parsers, bool deepScan /*= false*/)
{
  // begin critical section
  LockGuard g(m_mutex);

  if (m_scanner->isDebug())
    qDebug("Watch node %s, deepScan=%s", dirPath.toUtf8().constData(), (deepScan ? "true" : "false"));
  m_watcher.addPath(dirPath);
  resetNode(dirPath);
  QDirIterator::IteratorFlags flags = QDirIterator::NoIteratorFlags;
  if (deepScan)
    flags |= QDirIterator::Subdirectories;
  QDirIterator di(QDir(dirPath), flags);
  while (di.hasNext() && !isInterruptionRequested())
  {
    QString o = di.next();
    QFileInfo info = di.fileInfo();
    if (!info.isHidden() && info.isReadable())
    {
      if (info.isFile())
      {
        MediaParserPtr p = matchParser(parsers, info);
        if (p)
        {
          NodeMap::iterator itf = m_items.find(info.absoluteFilePath());
          if (itf == m_items.end())
          {
            // create the file item
            MediaFilePtr mf(new MediaFile(++m_sequence));
            mf->isPinned = true;
            mf->isDirectory = false;
            mf->filePath = info.absoluteFilePath();
            mf->baseName = info.baseName();
            mf->suffix = info.suffix();
            mf->path = info.absolutePath();
            mf->size = info.size();
            mf->lastModified = info.lastModified();
            mf->parser = p;
            if (m_scanner->isDebug())
              qDebug("Add item %s (%s)", info.absoluteFilePath().toUtf8().constData(), p->commonName());
            m_items.insert(info.absoluteFilePath(), mf);
            m_files.insert(info.absolutePath(), mf);
            if (mf->size > FILE_MIN_SIZE)
              scheduleExtractor(mf);
            else
              m_watcher.addPath(mf->filePath);
          }
          else
          {
            itf.value()->isPinned = true;
          }
        }
      }
      else if (info.isDir())
      {
        NodeMap::iterator itn = m_nodes.find(info.absoluteFilePath());
        if (itn == m_nodes.end())
        {
          // create the file node
          MediaFilePtr md(new MediaFile(++m_sequence));
          md->isPinned = true;
          md->isDirectory = true;
          md->filePath = info.absoluteFilePath();
          md->path = info.absolutePath();
          md->lastModified = info.lastModified();
          m_nodes.insert(info.absoluteFilePath(), md);
          m_files.insert(info.absolutePath(), md);
          if (deepScan)
            m_watcher.addPath(info.absoluteFilePath());
          else
            scanDir(info.absoluteFilePath(), parsers, true);
        }
        else
        {
          itn.value()->isPinned = true;
          resetNode(info.absoluteFilePath());
        }
      }
    }
  }
  // clean unpinned files
  QList<FileMap::iterator> cleaned;
  cleanNode(dirPath, false, cleaned);
  for (FileMap::iterator it : cleaned)
  {
    m_files.erase(it);
  }

  // end critical section
}

/**
 * Unpin the subitems of the given node. That clean the state before a new scan,
 * or that set all subitems for delete.
 * @param nodeName
 */
void MediaScannerEngine::resetNode(const QString& nodeName)
{
  QPair<FileMap::iterator, FileMap::iterator> pair = m_files.equal_range(nodeName);
  FileMap::iterator it = pair.first;
  while (it != pair.second)
  {
    it.value()->isPinned = false;
    ++it;
  }
}

/**
 * Erase subitems from a node
 * @param nodeName
 * @param evenPinned true to erase all, else false to erase unpinned only
 * @param cleaned the list to callback all erased
 */
void MediaScannerEngine::cleanNode(const QString& nodeName, bool evenPinned, QList<FileMap::iterator>& cleaned)
{
  if (m_scanner->isDebug())
    qDebug("Clean node %s", nodeName.toUtf8().constData());
  QPair<FileMap::iterator, FileMap::iterator> pair = m_files.equal_range(nodeName);
  FileMap::iterator it = pair.first;
  while (it != pair.second)
  {
    if (evenPinned || !it.value()->isPinned)
    {
      cleaned.push_back(it);
      if (it.value()->isDirectory)
      {
        m_watcher.removePath(it.value()->filePath);
        if (m_scanner->isDebug())
          qDebug("Remove node %s", it.value()->filePath.toUtf8().constData());
        cleanNode(it.value()->filePath, true, cleaned);
        m_nodes.remove(it.value()->filePath);
      }
      else
      {
        if (m_scanner->isDebug())
          qDebug("Remove item %s", it.value()->filePath.toUtf8().constData());
        m_items.remove(it.value()->filePath);
        m_scanner->remove(it.value());
      }
    }
    ++it;
  }
}

void MediaScannerEngine::scheduleExtractor(MediaFilePtr filePtr, bool wait /*= true*/)
{
  MediaExtractor * job = new MediaExtractor(this, &MediaScannerEngine::mediaExtractorCallback, filePtr, m_scanner->isDebug());
  if (wait)
  {
    while (!isInterruptionRequested())
    {
      if (m_workerPool.tryStart(job))
        break;
      msleep(SCHEDULE_TIMEOUT_MS);
    }
  }
  else if (!isInterruptionRequested())
    m_workerPool.start(job);
}

void MediaScannerEngine::mediaExtractorCallback(void * handle, MediaFilePtr& filePtr)
{
  MediaScannerEngine * engine = static_cast<MediaScannerEngine*>(handle);
  if (!engine)
    return;
  if (filePtr->isValid)
    engine->m_scanner->put(filePtr);
  else if (filePtr->retry < RETRY_MAX)
  {
    filePtr->retry++;
    MediaExtractor * job = new MediaExtractor(engine, &MediaScannerEngine::mediaExtractorCallback, filePtr, engine->m_scanner->isDebug());
    job->setTimeout(RETRY_TIMEOUT_MS);
    engine->m_delayedJobs.enqueue(job);
  }
}

MediaParserPtr MediaScannerEngine::matchParser(const QList<MediaParserPtr>& parsers, const QFileInfo& fileInfo)
{
  for (MediaParserPtr p : parsers)
  {
    if (p->match(fileInfo))
      return p;
  }
  return MediaParserPtr();
}
