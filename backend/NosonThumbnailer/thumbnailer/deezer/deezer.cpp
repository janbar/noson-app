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

#include <QDebug>

using namespace thumbnailer;

namespace
{
  // register the API
  bool dummy = AbstractAPI::registerMe(new DeezerAPI());
}

bool DeezerAPI::configure(NetManager* nam, const QString& apiKey)
{
  (void)nam;
  (void)apiKey;
  return true;
}

AbstractArtistInfo* DeezerAPI::newArtistInfo(const QString& artist)
{
  return new DEEZERArtistInfo(artist);
}

AbstractAlbumInfo* DeezerAPI::newAlbumInfo(const QString& artist, const QString& album)
{
  return new DEEZERAlbumInfo(artist, album);
}

bool DeezerAPI::parseServerError(int statusCode, const QByteArray& info, AbstractAPI::error_t& error)
{
  /* List of API errors (see https://developers.deezer.com/api/errors)
  4   : QUOTA
  100 : ITEMS_LIMIT_EXCEEDED
  200 : PERMISSION
  300 : TOKEN_INVALID
  500 : PARAMETER
  501 : PARAMETER_MISSING
  600 : QUERY_INVALID
  700 : SERVICE_BUSY
  800 : DATA_NOT_FOUND
  */
  (void)statusCode;

  if (info.isEmpty())
    return false;
  // Parse json content
  JSON::Document rootdoc(info.constData());
  if (!rootdoc.IsValid())
  {
    qDebug().noquote() << info;
    return false;
  }

  JSON::Node n_error = rootdoc.GetRoot().GetObjectValue("error");
  if (n_error.IsObject())
  {
    for (size_t a = 0; a < n_error.Size(); ++a)
    {
      QString attr = n_error.GetObjectKey(a);
      if (attr == "code")
        error.errorCode = n_error.GetObjectValue(a).GetIntValue();
      else if (attr ==  "message")
        error.errorString = n_error.GetObjectValue(a).GetStringValue();
    }
    switch (error.errorCode)
    {
    case 4:
      error.status = ReplyQuotaExceeded;
      break;
    case 100:
    case 700:
      error.status = ReplyServerError;
      break;
    case 800:
      error.status = ReplyNoDataFound;
      break;
    default:
      error.status = ReplyFatalError;
      break;
    }
    return true;
  }
  return false;
}
