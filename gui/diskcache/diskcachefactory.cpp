#include "diskcachefactory.h"

#include <QNetworkAccessManager>
#include <QNetworkCacheMetaData>
#include <QNetworkDiskCache>
#include <QStandardPaths>

#include "cachingnetworkaccessmanager.h"
#include "cachereply.h"

DiskCacheFactory::DiskCacheFactory(int cacheSize)
: mCacheSize(cacheSize) { }

QNetworkAccessManager *DiskCacheFactory::create(QObject *parent)
{
  QNetworkAccessManager *nam = new CachingNetworkAccessManager(parent);

  QNetworkDiskCache *diskCache = new QNetworkDiskCache(nam);
  QString cacheFolder = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
  diskCache->setCacheDirectory(cacheFolder);
  diskCache->setMaximumCacheSize(mCacheSize);

  nam->setCache(diskCache);

  qDebug() << "installing network cache of" << (mCacheSize / 1024) << "KB in folder" << cacheFolder;

  return nam;
}
