/*
 *      Copyright (C) 2019 Jean-Luc Barriere
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

#ifndef ARTISTINFO_H
#define ARTISTINFO_H

#include "abstractapi.h"

#include <QIODevice>
#include <QSize>
#include <QUrl>

#include <memory>

namespace thumbnailer
{

  class DiskCacheManager;
  class NetManager;
  class NetRequest;

  class ArtistInfo final : public AbstractWorker
  {
    Q_OBJECT

  public:
    ArtistInfo(DiskCacheManager* cache, NetManager* nam, AbstractAPI* api, const QString& artist, const QSize& requestedSize, bool cached, QObject* parent = 0);
    ~ArtistInfo();

    void run();

    ReplyStatus error() const;

    int errorCode() const;

    QString errorString() const;

    const QByteArray& image() const;

    bool isCached() const;

  private slots:
    void queryInfo();
    void readInfo();
    void processInfo();
    void readImage();
    void processImage();

  private:
    AbstractAPI::Parse_Status parseInfo();
    bool parseServerError();
    void queryImage(const QUrl& url);
    void fakeImage();

    DiskCacheManager* m_cache;
    NetManager* m_nam;
    AbstractAPI* m_api;
    QString m_artist;
    QSize m_requestedSize;
    bool m_cached;
    QUrl m_cacheUrl;
    int m_size;

    QIODevice* m_cacheDev;
    std::unique_ptr<NetRequest> m_call;
    AbstractAPI::error_t m_error;
    QByteArray m_info;
    QByteArray m_image;
    AbstractArtistInfo::metadata_t m_meta;

    AbstractArtistInfo* m_p;
    int m_try;
  };

}
#endif /* ARTISTINFO_H */

