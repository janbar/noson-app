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

#ifndef NOSONAPPSONOS_H
#define NOSONAPPSONOS_H

#include <noson/sonossystem.h>

#include "tools.h"
#include "locked.h"
#include "future.h"
#include "zonesmodel.h"
#include "roomsmodel.h"
#include "albumsmodel.h"
#include "artistsmodel.h"
#include "composersmodel.h"
#include "genresmodel.h"
#include "tracksmodel.h"
#include "queuemodel.h"
#include "playlistsmodel.h"
#include "favoritesmodel.h"
#include "servicesmodel.h"
#include "mediamodel.h"
#include "allservicesmodel.h"

#include <QObject>
#include <QString>
#include <QQmlEngine>
#include <QThread>
#include <QThreadPool>

namespace nosonapp
{

class Sonos : public QObject, public ContentProvider<Sonos>
{
  Q_OBJECT
  Q_PROPERTY(int jobCount READ jobCount NOTIFY jobCountChanged)
  Q_PROPERTY(QString systemLocalURI READ systemLocalURI CONSTANT)

public:
  explicit Sonos(QObject *parent = 0);
  ~Sonos();

  Q_INVOKABLE void debug(int debug);

  ///////////////////////////////////////////////////////////////////////////////
  ///
  /// About futures

  Q_INVOKABLE nosonapp::Future* tryInit(int debug = 0);
  Q_INVOKABLE nosonapp::Future* tryRenewSubscriptions();
  Q_INVOKABLE nosonapp::Future* tryJoinZones(const QVariantList& zonePayloads, const QVariant& toZonePayload);
  Q_INVOKABLE nosonapp::Future* tryUnjoinZone(const QVariant& zonePayload);
  Q_INVOKABLE nosonapp::Future* tryUnjoinRooms(const QVariantList& roomPayloads);
  Q_INVOKABLE nosonapp::Future* tryCreateAlarm(const QVariant& alarmPayload);
  Q_INVOKABLE nosonapp::Future* tryUpdateAlarm(const QVariant& alarmPayload);
  Q_INVOKABLE nosonapp::Future* tryDestroyAlarm(const QString& id);
  Q_INVOKABLE nosonapp::Future* tryRefreshShareIndex();
  Q_INVOKABLE nosonapp::Future* tryDestroySavedQueue(const QString& SQid);
  Q_INVOKABLE nosonapp::Future* tryAddItemToFavorites(const QVariant& payload, const QString& description, const QString& artURI);
  Q_INVOKABLE nosonapp::Future* tryDestroyFavorite(const QString& FVid);

  ///////////////////////////////////////////////////////////////////////////////
  ///
  /// Synchonous

  Q_INVOKABLE bool init(int debug = 0);
  Q_INVOKABLE bool init(int debug, const QString& url);

  Q_INVOKABLE void setLocale(const QString& locale);

  Q_INVOKABLE QString getLocale();

  Q_INVOKABLE QString getLibVersion();

  Q_INVOKABLE void addServiceOAuth(const QString& type, const QString& sn, const QString& key, const QString& token, const QString& username);
  Q_INVOKABLE void deleteServiceOAuth(const QString& type, const QString& sn);

  Q_INVOKABLE void renewSubscriptions();

  Q_INVOKABLE QVariantList getZones();

  Q_INVOKABLE bool isConnected();

  Q_INVOKABLE QVariantList getZoneRooms(const QString& zoneId);

  Q_INVOKABLE bool joinRoom(const QVariant& roomPayload, const QVariant& toZonePayload);

  Q_INVOKABLE bool joinZone(const QVariant& zonePayload, const QVariant& toZonePayload);
  Q_INVOKABLE bool joinZones(const QVariantList& zonePayloads, const QVariant& toZonePayload);

  Q_INVOKABLE bool unjoinRoom(const QVariant& roomPayload);
  Q_INVOKABLE bool unjoinRooms(const QVariantList& roomPayloads);

  Q_INVOKABLE bool unjoinZone(const QVariant& zonePayload);

  Q_INVOKABLE bool createAlarm(const QVariant& alarmPayload);
  Q_INVOKABLE bool updateAlarm(const QVariant& alarmPayload);
  Q_INVOKABLE bool destroyAlarm(const QString& id);

  Q_INVOKABLE bool refreshShareIndex();
  Q_INVOKABLE bool destroySavedQueue(const QString& SQid);
  Q_INVOKABLE bool addItemToFavorites(const QVariant& payload, const QString& description, const QString& artURI);
  Q_INVOKABLE bool destroyFavorite(const QString& FVid);

  Q_INVOKABLE QString getObjectIDFromUriMetadata(const QVariant& itemPayload);

  Q_INVOKABLE bool isItemFromService(const QVariant& itemPayload);

  SONOS::System& getSystem();
  SONOS::ZonePtr findZone(const QString& zoneName);

  Q_INVOKABLE void runLoader();

  // Implements ContentProvider
  void beforeLoad();
  void afterLoad();
  void runContentLoader(ListModel<Sonos>* model);
  void loadContent(ListModel<Sonos>* model);
  void loadAllContent();
  void runContentLoaderForContext(ListModel<Sonos>* model, int id);
  void loadContentForContext(ListModel<Sonos>* model, int id);
  const char* getHost() const;
  unsigned getPort() const;
  QString getBaseUrl() const;
  void registerContent(ListModel<Sonos>* model, const QString& root);
  void unregisterContent(ListModel<Sonos>* model);

  // About jobs
  bool startJob(QRunnable* worker);
  int jobCount() { return *(m_jobCount.Get()); }
  void beginJob();
  void endJob();

  const QString& systemLocalURI() { return m_systemLocalURI; }

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

  static QObject* allComposersModel_provider(QQmlEngine *engine, QJSEngine *scriptEngine)
  {
    Q_UNUSED(engine)
    Q_UNUSED(scriptEngine)
    return new ComposersModel;
  }

  static QObject* allGenresModel_provider(QQmlEngine *engine, QJSEngine *scriptEngine)
  {
    Q_UNUSED(engine)
    Q_UNUSED(scriptEngine)
    return new GenresModel;
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
    return new AllServicesModel;
  }

  static QObject* MyServicesModel_provider(QQmlEngine *engine, QJSEngine *scriptEngine)
  {
    Q_UNUSED(engine)
    Q_UNUSED(scriptEngine)
    return new ServicesModel;
  }

  // Helpers
  Q_INVOKABLE static QString normalizedInputString(const QString& str)
  {
    return nosonapp::normalizedString(str);
  }

  Q_INVOKABLE static bool havePulseAudio()
  {
    return SONOS::System::HavePulseAudio();
  }

signals:
  void initDone(bool succeeded);
  void loadingStarted();
  void loadingFinished();
  void transportChanged();
  void renderingControlChanged();
  void topologyChanged();
  void alarmClockChanged();
  void shareIndexInProgress();
  void shareIndexFinished();

  void jobCountChanged();

private:
  typedef QList<RegisteredContent<Sonos> > ManagedContents;
  Locked<ManagedContents> m_library;
  unsigned m_shareUpdateID; // Current updateID of SONOS shares
  bool m_shareIndexInProgess;
  unsigned m_savedQueuesUpdateID; // Current updatedID of SONOS Playlists

  SONOS::System m_system;
  QThreadPool m_workerPool;
  LockedNumber<int> m_jobCount;
  QString m_systemLocalURI;

  Locked<QString> m_locale; // language_COUNTRY

  static void systemEventCB(void *handle);

  static void unregisterContent(ManagedContents& mc, ListModel<Sonos> *model);

  ///////////////////////////////////////////////////////////////////////////////
  ///
  /// About promises

  class PromiseInit : public Promise
  {
  public:
    PromiseInit(Sonos& sonos, int debug)
    : m_sonos(sonos), m_debug(debug) { }
    void run() override;
  private:
    Sonos& m_sonos;
    int m_debug;
  };

  class PromiseRenewSubscriptions : public Promise
  {
  public:
    PromiseRenewSubscriptions(Sonos& sonos)
    : m_sonos(sonos) { }
    void run() override;
  private:
    Sonos& m_sonos;
  };

  class PromiseJoinZones : public Promise
  {
  public:
    PromiseJoinZones(Sonos& sonos, const QVariantList& zonePayloads, const QVariant& toZonePayload)
    : m_sonos(sonos), m_zonePayloads(zonePayloads), m_toZonePayload(toZonePayload) { }
    void run() override;
  private:
    Sonos& m_sonos;
    const QVariantList m_zonePayloads;
    const QVariant m_toZonePayload;
  };

  class PromiseUnjoinZone : public Promise
  {
  public:
    PromiseUnjoinZone(Sonos& sonos, const QVariant& zonePayload)
    : m_sonos(sonos), m_zonePayload(zonePayload) { }
    void run() override;
  private:
    Sonos& m_sonos;
    const QVariant m_zonePayload;
  };

  class PromiseUnjoinRooms : public Promise
  {
  public:
    PromiseUnjoinRooms(Sonos& sonos, const QVariantList& roomPayloads)
    : m_sonos(sonos), m_roomPayloads(roomPayloads) { }
    void run() override;
  private:
    Sonos& m_sonos;
    const QVariantList m_roomPayloads;
  };

  class PromiseCreateAlarm : public Promise
  {
  public:
    PromiseCreateAlarm(Sonos& sonos, const QVariant& alarmPayload)
    : m_sonos(sonos), m_alarmPayload(alarmPayload) { }
    void run() override;
  private:
    Sonos& m_sonos;
    const QVariant m_alarmPayload;
  };

  class PromiseUpdateAlarm : public Promise
  {
  public:
    PromiseUpdateAlarm(Sonos& sonos, const QVariant& alarmPayload)
    : m_sonos(sonos), m_alarmPayload(alarmPayload) { }
    void run() override;
  private:
    Sonos& m_sonos;
    const QVariant m_alarmPayload;
  };

  class PromiseDestroyAlarm : public Promise
  {
  public:
    PromiseDestroyAlarm(Sonos& sonos, const QString& id)
    : m_sonos(sonos), m_id(id) { }
    void run() override;
  private:
    Sonos& m_sonos;
    const QString m_id;
  };

  class PromiseRefreshShareIndex : public Promise
  {
  public:
    PromiseRefreshShareIndex(Sonos& sonos)
    : m_sonos(sonos) { }
    void run() override;
  private:
    Sonos& m_sonos;
  };

  class PromiseDestroySavedQueue : public Promise
  {
  public:
    PromiseDestroySavedQueue(Sonos& sonos, const QString& SQid)
    : m_sonos(sonos), m_SQid(SQid) { }
    void run() override;
  private:
    Sonos& m_sonos;
    const QString m_SQid;
  };

  class PromiseAddItemToFavorites : public Promise
  {
  public:
    PromiseAddItemToFavorites(Sonos& sonos, const QVariant& payload, const QString& description, const QString& artURI)
    : m_sonos(sonos), m_payload(payload), m_description(description), m_artURI(artURI) { }
    void run() override;
  private:
    Sonos& m_sonos;
    QVariant m_payload;
    const QString m_description;
    const QString m_artURI;
  };

  class PromiseDestroyFavorite : public Promise
  {
  public:
    PromiseDestroyFavorite(Sonos& sonos, const QString& FVid)
    : m_sonos(sonos), m_FVid(FVid) { }
    void run() override;
  private:
    Sonos& m_sonos;
    const QString m_FVid;
  };

};

}

#endif // NOSONAPPSONOS_H
