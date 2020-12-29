/*
 *      Copyright (C) 2019 Jean-Luc Barriere
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#include "plugin.h"
#include "mediascanner.h"
#include "aggregate/artists.h"
#include "aggregate/genres.h"
#include "aggregate/albums.h"
#include "aggregate/tracks.h"
#include "aggregate/composers.h"

#include <QtQml>
#include <QtQml/QQmlContext>
#include <QQmlEngine>
#include <memory>

#define CACHE_SIZE    50000000L

using namespace mediascanner;

void MediaScannerPlugin::registerTypes(const char* uri)
{
  // register the singleton
  qmlRegisterSingletonType<MediaScanner>(uri, 1, 0, "MediaScanner", MediaScannerPlugin::createMediaScanner);
  qmlRegisterType<mediascanner::Artists>(uri, 1, 0, "ArtistList");
  qmlRegisterType<mediascanner::Genres>(uri, 1, 0, "GenreList");
  qmlRegisterType<mediascanner::Albums>(uri, 1, 0, "AlbumList");
  qmlRegisterType<mediascanner::Tracks>(uri, 1, 0, "TrackList");
  qmlRegisterType<mediascanner::Composers>(uri, 1, 0, "ComposerList");
}

void MediaScannerPlugin::initializeEngine(QQmlEngine* engine, const char* uri)
{
  QQmlExtensionPlugin::initializeEngine(engine, uri);
}

QObject * MediaScannerPlugin::createMediaScanner(QQmlEngine *engine, QJSEngine *scriptEngine)
{
  Q_UNUSED(scriptEngine)
  return MediaScanner::instance(engine);
}
