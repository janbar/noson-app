#include "cachereply.h"

CacheReply::CacheReply(QIODevice *cacheDev, const QNetworkRequest &req,
                       QNetworkAccessManager::Operation op, QNetworkCacheMetaData meta,
                       QObject *parent)
  : QNetworkReply(parent)
  , mCacheDev(cacheDev)
{
  setRequest(req);
  setUrl(req.url());
  setOperation(op);
  setAttribute(QNetworkRequest::HttpStatusCodeAttribute, 200);
  setFinished(true);
  QNetworkReply::open(QIODevice::ReadOnly);

  qint64 size = cacheDev->size();
  foreach(QNetworkCacheMetaData::RawHeader header, meta.rawHeaders())
  {
    setRawHeader(header.first, header.second);
  }
  QMetaObject::invokeMethod(this, "metaDataChanged", Qt::QueuedConnection);

  QMetaObject::invokeMethod(this, "downloadProgress", Qt::QueuedConnection,
                            Q_ARG(qint64,size), Q_ARG(qint64, size));
  QMetaObject::invokeMethod(this, "readyRead", Qt::QueuedConnection);
  QMetaObject::invokeMethod(this, "finished", Qt::QueuedConnection);
}

CacheReply::~CacheReply()
{
  close();
  if (mCacheDev)
  {
    mCacheDev->close();
    delete mCacheDev;
  }
}

void CacheReply::abort()
{
  close();
}

qint64 CacheReply::bytesAvailable() const
{
  return mCacheDev->bytesAvailable();
}

bool CacheReply::isSequential() const
{
  return mCacheDev->isSequential();
}

qint64 CacheReply::size() const
{
  return mCacheDev->size();
}

qint64 CacheReply::readData(char *data, qint64 maxlen)
{
  return mCacheDev->read(data, maxlen);
}
