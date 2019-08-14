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

#ifndef ABSTRACTAPI_H
#define ABSTRACTAPI_H

#include "thumbnailerjob.h" // for ReplyStatus
#include "netrequest.h"

#include <QByteArray>
#include <QString>
#include <QMap>

#define THUMBNAILER_USER_AGENT  "thumbnailer/2.0 (io.github.janbar.noson)"

namespace thumbnailer
{

  class AbstractArtistInfo;
  class AbstractAlbumInfo;

  class AbstractAPI
  {
  public:
    virtual ~AbstractAPI() = default;
    virtual const char* name() = 0;
    virtual int delayOnQuotaExceeded() = 0;
    virtual int maxRetry() = 0;
    virtual bool configure(NetManager* nam, const QString& apiKey) = 0;
    virtual AbstractArtistInfo* newArtistInfo(const QString& artist) = 0;
    virtual AbstractAlbumInfo* newAlbumInfo(const QString& artist, const QString& album) = 0;

    typedef struct {
      ReplyStatus status;   ///< status of the response
      int errorCode;        ///< error no returned by the server
      QString errorString;  ///< error message returned by the server
    } error_t;

    typedef enum {
      Parse_Failed,     ///< parsing of the server response failed
      Parse_Succeeded,  ///< parsing succeeded and metadata have been filled
      Parse_RetryQuery, ///< parsing succeeded but an other query MUST be done to fill metadata
    } Parse_Status;

    static bool registerMe(AbstractAPI* api);
    static AbstractAPI* forName(const QString& apiName);

    static QString normalizeArtist(const QString& artist);
    static QString normalizeAlbum(const QString& album);

  private:

    struct Store
    {
      ~Store();
      QMap<QString, AbstractAPI*> map;
    };

    static Store apis;
  };

  class AbstractArtistInfo
  {
  public:
    AbstractArtistInfo(const QString& artist);
    virtual ~AbstractArtistInfo() = default;

    typedef struct {
      QString name;             ///< the name of artist
      QString mbid;             ///< the musicbrainz identifier for external use
      QString url;              ///< the link for further details
      QString image_small;
      QString image_medium;
      QString image_large;
      QString image_extralarge;
    } metadata_t;

    virtual void queryInfo(NetRequest* prepared) = 0;
    virtual AbstractAPI::Parse_Status parseInfo(const QByteArray& info, metadata_t& meta) = 0;
    virtual bool parseServerError(int statusCode, const QByteArray& info, AbstractAPI::error_t& error) = 0;

  protected:
    QString m_artist;
};

  class AbstractAlbumInfo
  {
  public:
    AbstractAlbumInfo(const QString& artist, const QString& album);
    virtual ~AbstractAlbumInfo() = default;

    typedef struct {
      QString title;            ///< the release title
      QString artist;           ///< the name of the main artist
      QString mbid;             ///< the musicbrainz identifier for external use
      QString releasedate;      ///< the release date
      QString url;              ///< the link for further details
      QString image_small;
      QString image_medium;
      QString image_large;
      QString image_extralarge;
    } metadata_t;

    virtual void queryInfo(NetRequest* prepared) = 0;
    virtual AbstractAPI::Parse_Status parseInfo(const QByteArray& info, metadata_t& meta) = 0;
    virtual bool parseServerError(int statusCode, const QByteArray& info, AbstractAPI::error_t& error) = 0;

  protected:
    QString m_artist;
    QString m_album;
  };

}

#endif /* ABSTRACTAPI_H */

