/*
 *      Copyright (C) 2018-2019 Jean-Luc Barriere
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

#include "albumartgenerator.h"
#include "artistartgenerator.h"

#include <QtQml>
#include <QtQml/QQmlContext>

#include <memory>

#define CACHE_SIZE 100000000L

std::shared_ptr<thumbnailer::Thumbnailer> ThumbnailerPlugin::g_thumbnailer(nullptr);

ThumbnailerPlugin::ThumbnailerPlugin(QObject *parent)
: QQmlExtensionPlugin(parent)
{
}

void ThumbnailerPlugin::registerTypes(const char* uri)
{
  qmlRegisterSingletonType<thumbnailer::Proxy>(uri, 1, 0, "Thumbnailer", ThumbnailerPlugin::proxy);
}

void ThumbnailerPlugin::initializeEngine(QQmlEngine* engine, const char* uri)
{
  Q_ASSERT(g_thumbnailer.get() == nullptr);

  QQmlExtensionPlugin::initializeEngine(engine, uri);

  g_thumbnailer.reset(new thumbnailer::Thumbnailer(engine->offlineStoragePath(), CACHE_SIZE, QString("")));

  try
  {
    engine->addImageProvider("albumart", new thumbnailer::qml::AlbumArtGenerator(g_thumbnailer));
  }
  // LCOV_EXCL_START
  catch (const std::exception& e)
  {
    qWarning() << "ThumbnailerPlugin::initializeEngine(): Failed to register albumart image provider: " << e.what();
  }
  catch (...)
  {
    qWarning() << "ThumbnailerPlugin::initializeEngine(): Failed to register "
            "albumart image provider: unknown exception";
  }
  // LCOV_EXCL_STOP

  try
  {
    engine->addImageProvider("artistart", new thumbnailer::qml::ArtistArtGenerator(g_thumbnailer));
  }
  // LCOV_EXCL_START
  catch (const std::exception& e)
  {
    qWarning() << "ThumbnailerPlugin::initializeEngine(): Failed to register artistart image provider: " << e.what();
  }
  catch (...)
  {
    qWarning() << "ThumbnailerPlugin::initializeEngine(): Failed to register "
            "artistart image provider: unknown exception";
  }
  // LCOV_EXCL_STOP
}

QObject* ThumbnailerPlugin::proxy(QQmlEngine *engine, QJSEngine *scriptEngine)
{
  Q_UNUSED(engine)
  Q_UNUSED(scriptEngine)
  return new thumbnailer::Proxy(ThumbnailerPlugin::g_thumbnailer);
}

thumbnailer::Proxy::Proxy(std::shared_ptr<thumbnailer::Thumbnailer>& thumbnailer, QObject *parent)
: QObject(parent)
, m_p(thumbnailer)
{
}

bool thumbnailer::Proxy::setApiKey(const QString &apiKey)
{
  m_p->setApiKey(apiKey);
  return m_p->isValid();
}
