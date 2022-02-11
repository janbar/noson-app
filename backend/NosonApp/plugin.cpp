/*
 *      Copyright (C) 2015-2019 Jean-Luc Barriere
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

#include "plugin.h"
#include "sonos.h"
#include "player.h"
#include "future.h"
#include "renderingmodel.h"
#include "alarmsmodel.h"
#include "qmlsortfiltermodel.h"

#include <QtQml>
#include <QtQml/QQmlContext>

using namespace nosonapp;

void NosonAppPlugin::registerTypes(const char *uri)
{
  // register utils types
  qmlRegisterType<QSortFilterProxyModelQML>(uri, 1, 0, "SortFilterModel");
  qmlRegisterUncreatableType<FilterBehavior>(uri, 1, 0, "FilterBehavior", "Not instantiable");
  qmlRegisterUncreatableType<SortBehavior>(uri, 1, 0, "SortBehavior", "Not instantiable");

  // register noson singletons
  qmlRegisterSingletonType<Sonos>(uri, 1, 0, "Sonos", Sonos::sonos_provider);
  qmlRegisterSingletonType<ZonesModel>(uri, 1, 0, "AllZonesModel", Sonos::allZonesModel_provider);
  qmlRegisterSingletonType<AlbumsModel>(uri, 1, 0, "AllAlbumsModel", Sonos::allAlbumsModel_provider);
  qmlRegisterSingletonType<ArtistsModel>(uri, 1, 0, "AllArtistsModel", Sonos::allArtistsModel_provider);
  qmlRegisterSingletonType<ComposersModel>(uri, 1, 0, "AllComposersModel", Sonos::allComposersModel_provider);
  qmlRegisterSingletonType<GenresModel>(uri, 1, 0, "AllGenresModel", Sonos::allGenresModel_provider);
  qmlRegisterSingletonType<PlaylistsModel>(uri, 1, 0, "AllPlaylistsModel", Sonos::allPlaylistsModel_provider);
  qmlRegisterSingletonType<FavoritesModel>(uri, 1, 0, "AllFavoritesModel", Sonos::allFavoritesModel_provider);
  qmlRegisterSingletonType<ServicesModel>(uri, 1, 0, "MyServicesModel", Sonos::MyServicesModel_provider);
  qmlRegisterSingletonType<AllServicesModel>(uri, 1, 0, "AllServicesModel", Sonos::allServicesModel_provider);
  //qmlRegisterSingletonType<TracksModel>(uri, 1, 0, "AllTracksModel", Sonos::allTracksModel_provider);

  // register noson instantiable types
  qmlRegisterType<Player>(uri, 1, 0, "ZonePlayer");
  qmlRegisterType<ZonesModel>(uri, 1, 0, "ZonesModel");
  qmlRegisterType<RoomsModel>(uri, 1, 0, "RoomsModel");
  qmlRegisterType<AlbumsModel>(uri, 1, 0, "AlbumsModel");
  qmlRegisterType<ArtistsModel>(uri, 1, 0, "ArtistsModel");
  qmlRegisterType<ComposersModel>(uri, 1, 0, "CompisersModel");
  qmlRegisterType<GenresModel>(uri, 1, 0, "GenresModel");
  qmlRegisterType<PlaylistsModel>(uri, 1, 0, "PlaylistsModel");
  qmlRegisterType<TracksModel>(uri, 1, 0, "TracksModel");
  qmlRegisterType<QueueModel>(uri, 1, 0, "QueueModel");
  qmlRegisterType<RenderingModel>(uri, 1, 0, "RenderingModel");
  qmlRegisterType<FavoritesModel>(uri, 1, 0, "FavoritesModel");
  qmlRegisterType<ServicesModel>(uri, 1, 0, "ServicesModel");
  qmlRegisterType<MediaModel>(uri, 1, 0, "MediaModel");
  qmlRegisterType<MediaAuth>(uri, 1, 0, "MediaAuth");
  qmlRegisterType<AlarmsModel>(uri, 1, 0, "AlarmsModel");
  qmlRegisterType<LibraryModel>(uri, 1, 0, "LibraryModel");

  qRegisterMetaType<Sonos*>("Sonos*");
  qRegisterMetaType<Player*>("Player*");
  qRegisterMetaType<Future*>("Future*");

  qRegisterMetaType<LibraryModel::DisplayType>("Library::DisplayType");
  qRegisterMetaType<LibraryModel::NodeType>("Library::NodeType");
  qRegisterMetaType<FavoritesModel::ItemType>("Favorites::ItemType");
}

void NosonAppPlugin::initializeEngine(QQmlEngine *engine, const char *uri)
{
  QQmlExtensionPlugin::initializeEngine(engine, uri);
}
