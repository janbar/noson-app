/*
 *      Copyright (C) 2018-2019 Jean-Luc Barriere
 *
 *  This file is part of Noson-App
 *
 *  Noson is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Noson is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Noson.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "diskcachemanager.h"

#include <QNetworkDiskCache>
#include <QNetworkCacheMetaData>
#include <QDateTime>
#include <QDir>
#include <QDebug>

using namespace thumbnailer;

DiskCacheManager::DiskCacheManager(const QString& offlineStoragePath,
            qint64 maxCacheSize, QObject* parent)
: QObject(parent)
{
  m_cache = new QNetworkDiskCache();
  m_cache->setCacheDirectory(offlineStoragePath + QDir::separator() + "thumbnailer");
  m_cache->setMaximumCacheSize(maxCacheSize);
}

DiskCacheManager::~DiskCacheManager()
{
  delete m_cache;
}

QIODevice* DiskCacheManager::queryData(const QUrl& url, bool noExpire /*=false*/)
{
  QNetworkCacheMetaData meta = m_cache->metaData(url);
  if(meta.isValid())
  {
    if (noExpire || meta.expirationDate() > QDateTime::currentDateTime())
      return m_cache->data(url);
    m_cache->remove(url);
  }
  return nullptr;
}

QIODevice* DiskCacheManager::createData(const QUrl& url, const QDateTime& expirationDate)
{
  QNetworkCacheMetaData metadata;
  metadata.setUrl(url);
  metadata.setExpirationDate(expirationDate);
  metadata.setSaveToDisk(true);
  QIODevice* dev = m_cache->prepare(metadata);
  return dev;
}

void DiskCacheManager::insertData(QIODevice* cacheDev)
{
  m_cache->insert(cacheDev);
}

void DiskCacheManager::clear()
{
  m_cache->clear();
}
