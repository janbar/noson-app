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
#include <cassert>

#define THREAD_EXPIRY_TIMEOUT 10000
#define THREAD_WAIT_TIMEOUT   500
#define DEFAULT_MAX_THREAD    2
#define SCHEDULE_TIMEOUT_MS   10
#define FILE_MIN_SIZE         1024
#define RETRY_TIMEOUT_MS      5000
#define RETRY_MAX             3

using namespace mediascanner;

MediaScannerEngine::MediaScannerEngine(MediaScanner * scanner, QObject* parent)
: QThread(parent)
, m_scanner(scanner)
, m_roots()
, m_working(false)
, m_sequence(0)
, m_nodes()
, m_items()
, m_files()
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
, m_fileItemsLock(new QRecursiveMutex)
#else
, m_fileItemsLock(new QMutex(QMutex::Recursive))
#endif
, m_watcher()
, m_parsers()
, m_workerPool()
, m_todo()
, m_condLock(new QMutex())
, m_cond()
, m_countValid(0)
, m_delayed()
{
  m_roots.append(QStandardPaths::standardLocations(QStandardPaths::MusicLocation));
  m_workerPool.setExpiryTimeout(THREAD_EXPIRY_TIMEOUT);
  m_workerPool.setMaxThreadCount(DEFAULT_MAX_THREAD);
  m_delayed.startProcessing(&m_workerPool);
  connect(this, &QThread::started, this, &MediaScannerEngine::onStarted);
}

MediaScannerEngine::~MediaScannerEngine()
{
  stop();
  m_delayed.stopProcessing();
  m_workerPool.clear();
  delete m_condLock;
  delete m_fileItemsLock;
}

void MediaScannerEngine::addParser(MediaParser* parser)
{
  QMutexLocker gc(m_condLock);
  for (MediaParserPtr p : m_parsers)
  {
    if (p->commonName() == parser->commonName())
      return;
  }
  m_parsers.push_back(MediaParserPtr(parser));
}

void MediaScannerEngine::removeParser(const QString& name)
{
  QMutexLocker gc(m_condLock);
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
  // MUST be unlocked as the mutex isn't declared recursive
  QList<MediaParserPtr> list;
  for (MediaParserPtr p : m_parsers)
    list.push_back(p);
  return list;
}

void MediaScannerEngine::setMaxThread(int maxThread)
{
  m_workerPool.setMaxThreadCount(maxThread);
}

QList<MediaFilePtr> MediaScannerEngine::allParsedFiles() const
{
  QMutexLocker g(m_fileItemsLock);

  QList<MediaFilePtr> list;
  for (const MediaFilePtr& file : m_files)
    if (file->isValid)
      list.push_back(file);
  return list;
}

bool MediaScannerEngine::addRootPath(const QString& dirPath)
{
  for (const QString& path : m_roots)
  {
    if (path == dirPath)
      return false;
  }
  m_roots.append(dirPath);
  if (QThread::isRunning())
    launchScan(dirPath);
  return true;
}

bool MediaScannerEngine::removeRootPath(const QString& dirPath)
{
  for(QStringList::iterator it = m_roots.begin(); it != m_roots.end(); ++it)
  {
    if (dirPath != *it)
      continue;
    m_roots.erase(it);
    QList<FileMap::iterator> cleaned;
    m_fileItemsLock->lock(); //is recursive
    cleanNode(dirPath, true, cleaned);
    for (FileMap::iterator it : cleaned)
      m_files.erase(it);
    m_fileItemsLock->unlock();
    return true;
  }
  return false;
}

void MediaScannerEngine::clearRoots()
{
  m_fileItemsLock->lock(); //is recursive
  for (const QString& path : m_roots)
  {
    QList<FileMap::iterator> cleaned;
    cleanNode(path, true, cleaned);
  }
  m_files.clear();
  m_roots.clear();
  m_fileItemsLock->unlock();
}


void MediaScannerEngine::stop()
{
  if (QThread::isRunning())
  {
    QThread::requestInterruption();
    // wake the thread
    m_condLock->lock();
    m_cond.wakeOne();
    m_condLock->unlock();
    while (!QThread::isFinished())
      QThread::msleep(THREAD_WAIT_TIMEOUT);
  }
}

void MediaScannerEngine::onStarted()
{
  for (QString root : m_roots)
    launchScan(root);
}

void MediaScannerEngine::onFileChanged(const QString& filePath)
{
  QFile f(filePath);
  if (f.exists() && f.size() > FILE_MIN_SIZE)
  {
    QMutexLocker g(m_fileItemsLock);

    NodeMap::iterator it = m_items.find(filePath);
    if (it != m_items.end())
      scheduleExtractor(it.value(), false);
    m_watcher.removePath(filePath);
  }
}

void MediaScannerEngine::onDirectoryChanged(const QString& dirPath)
{
  launchScan(dirPath);
}

void MediaScannerEngine::run()
{
  qInfo("scanner engine started");

  connect(&m_watcher, &QFileSystemWatcher::directoryChanged, this, &MediaScannerEngine::onDirectoryChanged);
  connect(&m_watcher, &QFileSystemWatcher::fileChanged, this, &MediaScannerEngine::onFileChanged);

  m_condLock->lock();
  while (!isInterruptionRequested())
  {
    if (m_todo.isEmpty()) // it could be filled before first loop
      m_cond.wait(m_condLock);

    if (!isInterruptionRequested() && !m_todo.isEmpty())
    {
      QList<MediaParserPtr> parserList = parsers();
      // signal start working
      m_working = true;
      m_scanner->workingChanged();
      do
      {
        QString path = m_todo.dequeue();
        m_condLock->unlock();
        scanDir(path, parserList);
        m_condLock->lock();
      }
      while (!isInterruptionRequested() && !m_todo.isEmpty());
      // signal stop working
      m_working = false;
      m_scanner->workingChanged();
    }
  }
  m_condLock->unlock();

  disconnect(&m_watcher, &QFileSystemWatcher::fileChanged, this, &MediaScannerEngine::onFileChanged);
  disconnect(&m_watcher, &QFileSystemWatcher::directoryChanged, this, &MediaScannerEngine::onDirectoryChanged);

  // purge
  m_fileItemsLock->lock();
  m_items.clear();
  m_fileItemsLock->unlock();

  qInfo("scanner engine stopped");
}

void MediaScannerEngine::launchScan(const QString& dirPath)
{
  QMutexLocker gc(m_condLock);
  m_todo.enqueue(dirPath);
  m_cond.wakeOne();
}

void MediaScannerEngine::scanDir(const QString &dirPath, const QList<MediaParserPtr>& parsers, bool deepScan /*= false*/)
{
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
    if (!info.isHidden() && info.isReadable()
            // current and parent path aren't hidden on windows platform
            // so exclude any path not strictly included in the base path
            && info.absoluteFilePath().length() > dirPath.length()
            )
    {
      if (info.isFile())
      {
        MediaParserPtr p = matchParser(parsers, info);
        if (p)
        {
          QMutexLocker g(m_fileItemsLock);

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

          m_fileItemsLock->lock();
          m_files.insert(info.absolutePath(), md);
          if (deepScan)
          {
            m_watcher.addPath(info.absoluteFilePath());
            m_fileItemsLock->unlock();
          }
          else
          {
            m_fileItemsLock->unlock();
            scanDir(info.absoluteFilePath(), parsers, true);
          }

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
  m_fileItemsLock->lock();
  for (FileMap::iterator it : cleaned)
    m_files.erase(it);
  m_fileItemsLock->unlock();
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

  QMutexLocker g(m_fileItemsLock);

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
        // check empty state
        if (it.value()->signaled)
        {
          if (1 == m_countValid--)
            emit m_scanner->emptyStateChanged();
          it.value()->signaled = false;
        }
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
  {
    engine->m_scanner->put(filePtr);
    // check empty state
    if (!filePtr->signaled)
    {
      if (0 == engine->m_countValid++)
        emit engine->m_scanner->emptyStateChanged();
      filePtr->signaled = true;
    }
  }
  else if (filePtr->retry < RETRY_MAX)
  {
    filePtr->retry++;
    MediaExtractor * job = new MediaExtractor(engine, &MediaScannerEngine::mediaExtractorCallback, filePtr, engine->m_scanner->isDebug());
    engine->m_delayed.enqueue(job);
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

MediaScannerEngine::DelayedQueue::DelayedQueue()
: QThread()
, m_workerPool(nullptr)
, m_delayedJobsLock(new QMutex())
, m_delayedJobs()
{
}

MediaScannerEngine::DelayedQueue::~DelayedQueue()
{
  stopProcessing();
  clear();
  delete m_delayedJobsLock;
}

void MediaScannerEngine::DelayedQueue::enqueue(MediaRunnable* runnable)
{
  LockGuard g(m_delayedJobsLock);
  runnable->setTimeout(RETRY_TIMEOUT_MS);
  m_delayedJobs.enqueue(runnable);
}

void MediaScannerEngine::DelayedQueue::clear()
{
  LockGuard g(m_delayedJobsLock);
  while (!m_delayedJobs.isEmpty())
    delete m_delayedJobs.dequeue();
}

void MediaScannerEngine::DelayedQueue::startProcessing(QThreadPool* pool)
{
  assert(pool);
  stopProcessing();
  m_workerPool = pool;
  QThread::start();
}

void MediaScannerEngine::DelayedQueue::stopProcessing()
{
  if (QThread::isRunning())
  {
    QThread::requestInterruption();
    while (!QThread::isFinished())
      QThread::msleep(THREAD_WAIT_TIMEOUT);
    m_workerPool = nullptr;
  }
}

void MediaScannerEngine::DelayedQueue::run()
{
  while (!isInterruptionRequested())
  {
    QThread::msleep(THREAD_WAIT_TIMEOUT);
    {
      LockGuard g(m_delayedJobsLock);
      while (!m_delayedJobs.isEmpty() && !isInterruptionRequested())
      {
        if (m_delayedJobs.front()->timeLeft() > 0)
          break;
        m_workerPool->start(m_delayedJobs.dequeue());
      }
    }
  }
}
