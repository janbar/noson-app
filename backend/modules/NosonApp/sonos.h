/*
 *      Copyright (C) 2015-2016 Jean-Luc Barriere
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

#ifndef SONOS_H
#define SONOS_H

#include "../../lib/noson/noson/src/sonossystem.h"
#include "../../lib/noson/noson/src/private/os/threads/threadpool.h"
#include "../../lib/noson/noson/src/locked.h"

#include "tools.h"
#include "zonesmodel.h"
#include "roomsmodel.h"
#include "albumsmodel.h"
#include "artistsmodel.h"
#include "genresmodel.h"
#include "tracksmodel.h"
#include "queuemodel.h"
#include "radiosmodel.h"
#include "playlistsmodel.h"
#include "favoritesmodel.h"
#include "servicesmodel.h"
#include "mediamodel.h"

#include <QObject>
#include <QString>
#include <QQmlEngine>

class Sonos : public QObject
{
  Q_OBJECT

public:
  explicit Sonos(QObject *parent = 0);
  ~Sonos();

  Q_INVOKABLE bool init(int debug = 0);

  Q_INVOKABLE void setLocale(const QString& locale);

  Q_INVOKABLE QString getLocale();

  Q_INVOKABLE void addServiceOAuth(const QString& type, const QString& sn, const QString& key, const QString& token);

  Q_INVOKABLE void renewSubscriptions();

  Q_INVOKABLE ZonesModel* getZones();

  Q_INVOKABLE bool connectZone(const QString& zoneName);

  Q_INVOKABLE QString getZoneName() const;

  Q_INVOKABLE QString getZoneShortName() const;

  Q_INVOKABLE bool joinRoom(const QVariant& roomPayload, const QVariant& toZonePayload);

  Q_INVOKABLE bool joinZone(const QVariant& zonePayload, const QVariant& toZonePayload);

  Q_INVOKABLE bool unjoinRoom(const QVariant& roomPayload);

  Q_INVOKABLE bool unjoinZone(const QVariant& zonePayload);

  const SONOS::System& getSystem() const;
  const SONOS::PlayerPtr& getPlayer() const;

  Q_INVOKABLE void runLoader();
  void loadEmptyModels();

  void runModelLoader(ListModel* model);
  void loadModel(ListModel* model);

  void registerModel(ListModel* model, const QString& root);
  void unregisterModel(ListModel* model);

  // Define singleton provider functions
  static QObject* sonos_provider(QQmlEngine *engine, QJSEngine *scriptEngine)
  {
    Q_UNUSED(engine)
    Q_UNUSED(scriptEngine)
    return new Sonos;
  }

  static QObject* allZonesModel_provider(QQmlEngine *engine, QJSEngine *scriptEngine)
  {
    Q_UNUSED(engine)
    Q_UNUSED(scriptEngine)
    return new ZonesModel;
  }

  static QObject* allAlbumsModel_provider(QQmlEngine *engine, QJSEngine *scriptEngine)
  {
    Q_UNUSED(engine)
    Q_UNUSED(scriptEngine)
    return new AlbumsModel;
  }

  static QObject* allArtistsModel_provider(QQmlEngine *engine, QJSEngine *scriptEngine)
  {
    Q_UNUSED(engine)
    Q_UNUSED(scriptEngine)
    return new ArtistsModel;
  }

  static QObject* allGenresModel_provider(QQmlEngine *engine, QJSEngine *scriptEngine)
  {
    Q_UNUSED(engine)
    Q_UNUSED(scriptEngine)
    return new GenresModel;
  }

  static QObject* allRadiosModel_provider(QQmlEngine *engine, QJSEngine *scriptEngine)
  {
    Q_UNUSED(engine)
    Q_UNUSED(scriptEngine)
    return new RadiosModel;
  }

  static QObject* allPlaylistsModel_provider(QQmlEngine *engine, QJSEngine *scriptEngine)
  {
    Q_UNUSED(engine)
    Q_UNUSED(scriptEngine)
    return new PlaylistsModel;
  }

  static QObject* allFavoritesModel_provider(QQmlEngine *engine, QJSEngine *scriptEngine)
  {
    Q_UNUSED(engine)
    Q_UNUSED(scriptEngine)
    return new FavoritesModel;
  }

  static QObject* allServicesModel_provider(QQmlEngine *engine, QJSEngine *scriptEngine)
  {
    Q_UNUSED(engine)
    Q_UNUSED(scriptEngine)
    return new ServicesModel;
  }

  // Helpers
  Q_INVOKABLE static QString normalizedInputString(const QString& str)
  {
    return normalizedString(str);
  }

signals:
  void loadingStarted();
  void loadingFinished();
  void transportChanged();
  void renderingControlChanged();
  void topologyChanged();

private:
  struct RegisteredContent
  {
    RegisteredContent(ListModel* _model, const QString& _root)
    : model(_model)
    , root(_root) { }
    ListModel* model;
    QString root;
  };
  typedef QList<RegisteredContent> ManagedContents;
  SONOS::Locked<ManagedContents> m_library;
  unsigned m_shareUpdateID; // Current updateID of SONOS shares

  SONOS::System m_system;
  SONOS::OS::CThreadPool m_threadpool;

  SONOS::Locked<QString> m_locale; // language_COUNTRY

  static void playerEventCB(void* handle);
  static void topologyEventCB(void* handle);
};

#endif // SONOS_H
