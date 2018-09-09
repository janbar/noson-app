#include <QtGlobal>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QSettings>
#include <QQuickStyle>
#include <QTranslator>
#include <QDebug>
#include <QDir>
#include <QProcess>
#include <QPixmap>
#include <QIcon>

#include "diskcache/diskcachefactory.h"

#include <NosonThumbnailer/plugin.h>
#include <NosonThumbnailer/albumartgenerator.h>
#include <NosonThumbnailer/artistartgenerator.h>

#include <NosonApp/sonos.h>
#include <NosonApp/player.h>
#include <NosonApp/renderingmodel.h>
#include <NosonApp/alarmsmodel.h>
#include <NosonApp/qmlsortfiltermodel.h>

#define CACHE_SIZE 100000000L

#if (defined(_WIN32) || defined(_WIN64))
#define __WINDOWS__
#endif

#ifdef __WINDOWS__
#include <winsock2.h>
#else
#include "signalhandler.h"
#endif

#ifndef APP_VERSION
#define APP_VERSION "Undefined"
#endif

void setupApp(QGuiApplication& app);
void prepareTranslator(QGuiApplication& app, const QString& translationPath, const QString& translationPrefix, const QLocale& locale);
void doExit(int code);

int main(int argc, char *argv[])
{
    int ret = 0;
#ifdef __WINDOWS__
    //Initialize Winsock
    WSADATA wsaData;
    if ((ret = WSAStartup(MAKEWORD(2, 2), &wsaData)))
        return ret;
#endif /* __WINDOWS__ */

    QGuiApplication::setApplicationName("noson");
    QGuiApplication::setOrganizationName("janbar");
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QGuiApplication app(argc, argv);
    setupApp(app);

    QSettings settings;
    QString style = QQuickStyle::name();
    if (!style.isEmpty())
        settings.setValue("style", style);
    else
    {
        if (settings.value("style").isNull())
        {
#ifdef Q_OS_ANDROID
            QQuickStyle::setStyle("Material");
#else
            QQuickStyle::setStyle("Universal");
#endif
            settings.setValue("style", QQuickStyle::name());
        }
        QQuickStyle::setStyle(settings.value("style").toString());
    }

    QQmlApplicationEngine engine;
    // 100MB cache for network data
    engine.setNetworkAccessManagerFactory(new DiskCacheFactory(CACHE_SIZE));
    // bind version string
    engine.rootContext()->setContextProperty("VersionString", QString(APP_VERSION));
    // bind arguments
    engine.rootContext()->setContextProperty("ApplicationArguments", app.arguments());
    // bind Android flag
#ifdef Q_OS_ANDROID
    engine.rootContext()->setContextProperty("Android", QVariant(true));
#else
    engine.rootContext()->setContextProperty("Android", QVariant(false));
#endif
    // select and bind styles available and known to work
    QStringList availableStyles;
    for (QString style : QQuickStyle::availableStyles())
    {
#ifdef Q_OS_ANDROID
      if (style == "Default")
        availableStyles.append(style);
      else if (style == "Material")
        availableStyles.append(style);
#else
      if (style == "Default")
        availableStyles.append(style);
      else if (style == "Fusion")
        availableStyles.append(style);
      else if (style == "Imagine")
        availableStyles.append(style);
      else if (style == "Material")
        availableStyles.append(style);
      else if (style == "Universal")
        availableStyles.append(style);
#endif
    }
    engine.rootContext()->setContextProperty("AvailableStyles", availableStyles);

    std::shared_ptr<thumbnailer::Thumbnailer> thumbnailer = std::make_shared<thumbnailer::Thumbnailer>(engine.offlineStoragePath(), CACHE_SIZE, QString(""));
    engine.rootContext()->setContextProperty("Thumbnailer", new thumbnailer::qml::Proxy(thumbnailer));
    engine.addImageProvider("albumart", new thumbnailer::qml::AlbumArtGenerator(thumbnailer));
    engine.addImageProvider("artistart", new thumbnailer::qml::ArtistArtGenerator(thumbnailer));

    // register utils types
    qmlRegisterType<QSortFilterProxyModelQML>("NosonApp", 1, 0, "SortFilterModel");
    qmlRegisterUncreatableType<FilterBehavior>("NosonApp", 1, 1, "FilterBehavior", "Not instantiable");
    qmlRegisterUncreatableType<SortBehavior>("NosonApp", 1, 1, "SortBehavior", "Not instantiable");

    // register noson singletons
    qmlRegisterSingletonType<Sonos>("NosonApp", 1, 0, "Sonos", Sonos::sonos_provider);
    qmlRegisterSingletonType<ZonesModel>("NosonApp", 1, 0, "AllZonesModel", Sonos::allZonesModel_provider);
    qmlRegisterSingletonType<AlbumsModel>("NosonApp", 1, 0, "AllAlbumsModel", Sonos::allAlbumsModel_provider);
    qmlRegisterSingletonType<ArtistsModel>("NosonApp", 1, 0, "AllArtistsModel", Sonos::allArtistsModel_provider);
    qmlRegisterSingletonType<ComposersModel>("NosonApp", 1, 0, "AllComposersModel", Sonos::allComposersModel_provider);
    qmlRegisterSingletonType<GenresModel>("NosonApp", 1, 0, "AllGenresModel", Sonos::allGenresModel_provider);
    qmlRegisterSingletonType<PlaylistsModel>("NosonApp", 1, 0, "AllPlaylistsModel", Sonos::allPlaylistsModel_provider);
    qmlRegisterSingletonType<FavoritesModel>("NosonApp", 1, 0, "AllFavoritesModel", Sonos::allFavoritesModel_provider);
    qmlRegisterSingletonType<ServicesModel>("NosonApp", 1, 0, "MyServicesModel", Sonos::MyServicesModel_provider);
    qmlRegisterSingletonType<AllServicesModel>("NosonApp", 1, 0, "AllServicesModel", Sonos::allServicesModel_provider);
    //qmlRegisterSingletonType<TracksModel>("NosonApp", 1, 0, "AllTracksModel", Sonos::allTracksModel_provider);

    // register noson instantiable types
    qmlRegisterType<Player>("NosonApp", 1, 0, "ZonePlayer");
    qmlRegisterType<ZonesModel>("NosonApp", 1, 0, "ZonesModel");
    qmlRegisterType<RoomsModel>("NosonApp", 1, 0, "RoomsModel");
    qmlRegisterType<AlbumsModel>("NosonApp", 1, 0, "AlbumsModel");
    qmlRegisterType<ArtistsModel>("NosonApp", 1, 0, "ArtistsModel");
    qmlRegisterType<ComposersModel>("NosonApp", 1, 0, "CompisersModel");
    qmlRegisterType<GenresModel>("NosonApp", 1, 0, "GenresModel");
    qmlRegisterType<PlaylistsModel>("NosonApp", 1, 0, "PlaylistsModel");
    qmlRegisterType<TracksModel>("NosonApp", 1, 0, "TracksModel");
    qmlRegisterType<QueueModel>("NosonApp", 1, 0, "QueueModel");
    qmlRegisterType<RenderingModel>("NosonApp", 1, 0, "RenderingModel");
    qmlRegisterType<FavoritesModel>("NosonApp", 1, 0, "FavoritesModel");
    //qmlRegisterUncreatableType<FavoriteType>("NosonApp", 1, 0, "FavoriteType", "enums");
    qmlRegisterType<ServicesModel>("NosonApp", 1, 0, "ServicesModel");
    qmlRegisterType<MediaModel>("NosonApp", 1, 0, "MediaModel");
    qmlRegisterType<MediaAuth>("NosonApp", 1, 0, "MediaAuth");
    qmlRegisterType<AlarmsModel>("NosonApp", 1, 0, "AlarmsModel");

    // handle signal exit(int) issued by the qml instance
    QObject::connect(&engine, &QQmlApplicationEngine::exit, doExit);

    engine.load(QUrl("qrc:/noson.qml"));
    if (engine.rootObjects().isEmpty()) {
        qWarning() << "Failed to load QML";
        return -1;
    }

    ret = app.exec();
#ifdef __WINDOWS__
    WSACleanup();
#endif /* __WINDOWS__ */
    return ret;
}

void setupApp(QGuiApplication& app) {
#ifndef __WINDOWS__
    SignalHandler *sh = new SignalHandler(&app);
    sh->catchSignal(SIGHUP, 0);
    sh->catchSignal(SIGALRM, 0);
#endif
    // set translators
    QLocale locale = QLocale::system();
    prepareTranslator(app, QString(":/i18n"), QString("noson"), locale);
#ifdef Q_OS_MAC
    QDir appDir(app.applicationDirPath());
    if (appDir.cdUp() && appDir.cd("Resources/translations"))
      prepareTranslator(app, appDir.absolutePath(), "qt", locale);
#elif defined(Q_OS_ANDROID)
    prepareTranslator(app, "assets:/translations", "qt", locale);
#endif
    app.setWindowIcon(QIcon(QPixmap(":/images/noson.png")));
}

void prepareTranslator(QGuiApplication& app, const QString& translationPath, const QString& translationPrefix, const QLocale& locale)
{
    QTranslator * translator = new QTranslator();
    if (!translator->load(locale, translationPrefix, QString("_"), translationPath))
    {
        qWarning() << "no file found for translations '"+ translationPath + "/" + translationPrefix + "_" + locale.name().left(2) + ".qm' (using default).";
    }
    else
    {
        qInfo() << "using file '"+ translationPath + "/" + translationPrefix + "_" + locale.name().left(2) + ".qm ' for translations.";
        app.installTranslator(translator);
    }
}

void doExit(int code)
{
#ifndef Q_OS_ANDROID
  if (code == 16)
  {
    QStringList args = QCoreApplication::arguments();
    args.removeFirst();
    QProcess::startDetached(QCoreApplication::applicationFilePath(), args);
  }
#endif
  QCoreApplication::quit();
}
