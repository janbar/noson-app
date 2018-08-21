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

#ifndef LFMARTISTINFO_H
#define LFMARTISTINFO_H

#include "thumbnailerjob.h"

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
    ArtistInfo(DiskCacheManager* cache, NetManager* nam, const QString& apiKey, const QString& artist, const QSize& requestedSize, bool cached, QObject* parent = 0);
    ~ArtistInfo();

    void run();

    ReplyStatus error() const;

    int errorCode() const;

    QString errorString() const;

    const QByteArray& image() const;

    bool isCached() const;

    signals:
    void doRequest(NetRequest*, QUrl);

  private slots:
    void readInfo();
    void processInfo();
    void readImage();
    void processImage();

  private:
    void queryInfo();
    bool parseInfo();
    void queryImage(const QUrl& url);
    bool parseServerError();
    void fakeImage();

    DiskCacheManager* m_cache;
    NetManager* m_nam;
    QString m_apiKey;
    QString m_artist;
    QSize m_requestedSize;
    bool m_cached;
    QUrl m_cacheUrl;
    int m_size;

    QIODevice* m_cacheDev;
    std::unique_ptr<NetRequest> m_call;
    ReplyStatus m_error;
    int m_errorCode;
    QString m_errorString;
    QByteArray m_info;
    QByteArray m_image;

    struct {
      QString name;
      QString mbid;
      QString url;
      QString image_small;
      QString image_medium;
      QString image_large;
      QString image_extralarge;
    } m_meta;
  };

}
#endif /* LFMARTISTINFO_H */

