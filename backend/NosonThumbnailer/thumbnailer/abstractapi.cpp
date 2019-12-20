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

#include "abstractapi.h"

#include <QDebug>

#include "lastfm/lastfm.h"
#include "deezer/deezer.h"

using namespace thumbnailer;

AbstractAPI::Store AbstractAPI::apis;

namespace {
  // registering APIs
  bool init()
  {
    AbstractAPI::registerMe(new LastfmAPI());
    AbstractAPI::registerMe(new DeezerAPI());
    return 0;
  }
  int dummy = init();
}

AbstractAPI::Store::~Store()
{
  for (auto& e : map)
    delete e;
  map.clear();
}

bool AbstractAPI::registerMe(AbstractAPI* api)
{
  qInfo().noquote() << "thumbnailer: register API [" + QString(api->name()) + "]";
  QMap<QString, AbstractAPI*>::iterator it = AbstractAPI::apis.map.insert(api->name(), api);
  return (it != apis.map.end());
}

AbstractAPI* AbstractAPI::forName(const QString& apiName)
{
  QMap<QString, AbstractAPI*>::const_iterator it = apis.map.find(apiName);
  return (it != apis.map.end() ? *it : nullptr);
}

QString AbstractAPI::normalizeArtist(const QString &artist)
{
  int s = artist.indexOf('/');
  int b = artist.indexOf(' ');
  if (b >= 0 && s > b)
    return artist.mid(0, s).remove('"');
  return QString(artist).remove('"');
}

QString AbstractAPI::normalizeAlbum(const QString &album)
{
  return QString(album).remove('"');
}

AbstractArtistInfo::AbstractArtistInfo(const QString& artist)
: m_artist(artist)
{
}

AbstractAlbumInfo::AbstractAlbumInfo(const QString& artist, const QString& album)
: m_artist(artist)
, m_album(album)
{
}
