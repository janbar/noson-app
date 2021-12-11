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

#include "deezer.h"
#include "../jsonparser.h"

#include <QUrl>
#include <QDebug>
#include <QUrlQuery>

#define BASE_URL "https://api.deezer.com/search/album"

using namespace thumbnailer;

DEEZERAlbumInfo::DEEZERAlbumInfo(const QString& artist, const QString& album)
: AbstractAlbumInfo(artist, album)
{
}

void DEEZERAlbumInfo::queryInfo(NetRequest* prepared)
{
  // see https://developers.deezer.com/api
  QUrlQuery qry;
  qry.addQueryItem("output", "json");
  qry.addQueryItem("limit", "1");
  QString q;
  q.append("artist:\"").append(AbstractAPI::normalizeArtist(m_artist)).append('"');
  q.append(" album:\"").append(AbstractAPI::normalizeAlbum(m_album)).append('"');
  qry.addQueryItem("q", q);
  QUrl url;
  url.setUrl(BASE_URL);
  url.setQuery(qry);

  prepared->setOperation(QNetworkAccessManager::GetOperation);
  prepared->setHeader(QNetworkRequest::UserAgentHeader, THUMBNAILER_USER_AGENT);
  prepared->setHeader("Accept", "application/json");
  prepared->setHeader("Accept-Charset", "utf-8");
  prepared->setUrl(QUrl(url));
}

AbstractAPI::Parse_Status DEEZERAlbumInfo::parseInfo(const QByteArray& info, AbstractAlbumInfo::metadata_t& meta)
{
  if (info.isEmpty())
    return AbstractAPI::Parse_Failed;
  // Parse json content
  JSON::Document rootdoc(info.constData());
  if (!rootdoc.IsValid())
  {
    qDebug().noquote() << info;
    return AbstractAPI::Parse_Failed;
  }

  JSON::Node n_data = rootdoc.GetRoot().GetObjectValue("data");
  if (n_data.IsArray())
  {
    for (size_t i = 0; i < n_data.Size(); ++i)
    {
      JSON::Node n_row = n_data.GetArrayElement(i);
      if (n_row.IsObject() && n_row.GetObjectValue("type").GetStringValue() == "album")
      {
        for (size_t a = 0; a < n_row.Size(); ++a)
        {
          QString attr = n_row.GetObjectKey(a);
          if (attr == "title")
            meta.title = n_row.GetObjectValue(a).GetStringValue();
          else if (attr == "link")
            meta.url = n_row.GetObjectValue(a).GetStringValue();
          else if (attr == "cover_small")
            meta.image_small = n_row.GetObjectValue(a).GetStringValue();
          else if (attr == "cover_medium")
            meta.image_medium = n_row.GetObjectValue(a).GetStringValue();
          else if (attr == "cover_big")
            meta.image_large = n_row.GetObjectValue(a).GetStringValue();
          else if (attr == "cover_xl")
            meta.image_extralarge = n_row.GetObjectValue(a).GetStringValue();
          else if (attr == "artist")
          {
            JSON::Node n_artist = n_row.GetObjectValue(a);
            if (n_artist.IsObject())
              meta.artist = n_artist.GetObjectValue("name").GetStringValue();
          }
        }
        break; // read first album
      }
    }
    return AbstractAPI::Parse_Succeeded;
  }
  return AbstractAPI::Parse_Failed;
}

bool DEEZERAlbumInfo::parseServerError(int statusCode, const QByteArray& info, AbstractAPI::error_t& error)
{
  return DeezerAPI::parseServerError(statusCode, info, error);
}
