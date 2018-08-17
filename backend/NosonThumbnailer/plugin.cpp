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

#include "plugin.h"

#include "albumartgenerator.h"
#include "artistartgenerator.h"

#include <memory>

#define CACHE_SIZE 100000000L

namespace thumbnailer
{

  namespace qml
  {
    ThumbnailerPlugin* ThumbnailerPlugin::_instance_ = nullptr;

    ThumbnailerPlugin::ThumbnailerPlugin(QObject *parent)
    : QQmlExtensionPlugin(parent)
    , m_thumbnailer(nullptr)
    {
      Q_ASSERT(ThumbnailerPlugin::_instance_ == nullptr);
      ThumbnailerPlugin::_instance_ = this;
    }

    void ThumbnailerPlugin::registerTypes(const char* uri)
    {
      qmlRegisterSingletonType<Controller>(uri, 1, 0, "Thumbnailer", ThumbnailerPlugin::thumbnailer);
    }

    void ThumbnailerPlugin::initializeEngine(QQmlEngine* engine, const char* uri)
    {
      QQmlExtensionPlugin::initializeEngine(engine, uri);

      m_thumbnailer.reset(new thumbnailer::Thumbnailer(engine->offlineStoragePath(), CACHE_SIZE, QString("")));

      try
      {
        engine->addImageProvider("albumart", new AlbumArtGenerator(m_thumbnailer));
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
        engine->addImageProvider("artistart", new ArtistArtGenerator(m_thumbnailer));
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

    QObject* ThumbnailerPlugin::thumbnailer(QQmlEngine *engine, QJSEngine *scriptEngine)
    {
      Q_UNUSED(engine)
      Q_UNUSED(scriptEngine)
      return new Controller(ThumbnailerPlugin::_instance_->m_thumbnailer);
    }

    Controller::Controller(std::shared_ptr<thumbnailer::Thumbnailer>& thumbnailer, QObject *parent)
    : QObject(parent)
    , m_p(thumbnailer)
    {
    }

    bool Controller::setApiKey(const QString &apiKey)
    {
      m_p->setApiKey(apiKey);
      return m_p->isValid();
    }

  } // namespace qml

} // namespace thumbnailer
