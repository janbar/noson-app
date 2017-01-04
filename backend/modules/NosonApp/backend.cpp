/*
 *      Copyright (C) 2015-2016 Jean-Luc Barriere
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published
 *  by the Free Software Foundation; either version 3, or (at your option)
 *  any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this library; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 51 Franklin Street, Fifth Floor, Boston,
 *  MA 02110-1301 USA
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#include "backend.h"
#include "sonos.h"
#include "player.h"
#include "renderingmodel.h"

#include <QtQml>
#include <QtQml/QQmlContext>

void BackendPlugin::registerTypes(const char *uri)
{
  Q_ASSERT(uri == QLatin1String("NosonApp"));

  qmlRegisterSingletonType<Sonos>(uri, 1, 0, "Sonos", Sonos::sonos_provider);
  qmlRegisterSingletonType<ZonesModel>(uri, 1, 0, "AllZonesModel", Sonos::allZonesModel_provider);
  qmlRegisterSingletonType<AlbumsModel>(uri, 1, 0, "AllAlbumsModel", Sonos::allAlbumsModel_provider);
  qmlRegisterSingletonType<ArtistsModel>(uri, 1, 0, "AllArtistsModel", Sonos::allArtistsModel_provider);
  qmlRegisterSingletonType<GenresModel>(uri, 1, 0, "AllGenresModel", Sonos::allGenresModel_provider);
  qmlRegisterSingletonType<RadiosModel>(uri, 1, 0, "AllRadiosModel", Sonos::allRadiosModel_provider);
  qmlRegisterSingletonType<PlaylistsModel>(uri, 1, 0, "AllPlaylistsModel", Sonos::allPlaylistsModel_provider);
  qmlRegisterSingletonType<FavoritesModel>(uri, 1, 0, "AllFavoritesModel", Sonos::allFavoritesModel_provider);
  qmlRegisterSingletonType<ServicesModel>(uri, 1, 0, "AllServicesModel", Sonos::allServicesModel_provider);
  //qmlRegisterSingletonType<TracksModel>(uri, 1, 0, "AllTracksModel", Sonos::allTracksModel_provider);

  qmlRegisterType<Player>(uri, 1, 0, "ZonePlayer");
  qmlRegisterType<ZonesModel>(uri, 1, 0, "ZonesModel");
  qmlRegisterType<RoomsModel>(uri, 1, 0, "RoomsModel");
  qmlRegisterType<AlbumsModel>(uri, 1, 0, "AlbumsModel");
  qmlRegisterType<ArtistsModel>(uri, 1, 0, "ArtistsModel");
  qmlRegisterType<GenresModel>(uri, 1, 0, "GenresModel");
  qmlRegisterType<RadiosModel>(uri, 1, 0, "RadiosModel");
  qmlRegisterType<PlaylistsModel>(uri, 1, 0, "PlaylistsModel");
  qmlRegisterType<TracksModel>(uri, 1, 0, "TracksModel");
  qmlRegisterType<QueueModel>(uri, 1, 0, "QueueModel");
  qmlRegisterType<RenderingModel>(uri, 1, 0, "RenderingModel");
  qmlRegisterType<FavoritesModel>(uri, 1, 0, "FavoritesModel");
  //qmlRegisterUncreatableType<FavoriteType>(uri, 1, 0, "FavoriteType", "enums");
  qmlRegisterType<ServicesModel>(uri, 1, 0, "ServicesModel");
  qmlRegisterType<MediaModel>(uri, 1, 0, "MediaModel");
}

void BackendPlugin::initializeEngine(QQmlEngine *engine, const char *uri)
{
  QQmlExtensionPlugin::initializeEngine(engine, uri);
}
