/*
 *      Copyright (C) 2018 Jean-Luc Barriere
 *
 *  This file is part of Noson-App
 *
 *  Noson-App is free software: you can redistribute it and/or modify
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
 *  along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef DISKCACHEMANAGER_H
#define DISKCACHEMANAGER_H

#include <QNetworkDiskCache>

namespace thumbnailer
{

  class DiskCacheManager : public QObject
  {
    Q_OBJECT
  public:
    DiskCacheManager(const QString& offlineStoragePath,
            qint64 maxCacheSize, QObject* parent = 0);
    ~DiskCacheManager();

    QIODevice* queryData(const QUrl& url, bool noExpire = false);
    QIODevice* createData(const QUrl& url, const QDateTime& expirationDate);
    void insertData(QIODevice* cacheDev);

    void clear();

  private:
    QNetworkDiskCache* m_cache;
  };

}
#endif /* DISKCACHEMANAGER_H */

