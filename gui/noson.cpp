#if (defined(_WIN32) || defined(_WIN64))
#define WIN32_LEAN_AND_MEAN
#endif

#include <QtGlobal>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QSettings>
#ifdef SAILFISHOS
#include <sailfishapp/sailfishapp.h>
#include <QQuickView>
#else
#include <QtQuickControls2>
#endif
#include <QTranslator>
#include <QDebug>
#include <QDir>
#include <QProcess>
#include <QPixmap>
#include <QIcon>
#include <QTime>

#include "diskcache/diskcachefactory.h"

#define CACHE_SIZE 100000000L
#ifndef SAILFISHOS
#define ORG_NAME          "janbar"
#define APP_NAME          "io.github.janbar.noson"
#define APP_DISPLAY_NAME  "noson"
#else
#define ORG_NAME          "harbour-noson"
#define APP_NAME          "harbour-noson"
#define APP_DISPLAY_NAME  "noson"
#endif

#ifdef Q_OS_WIN
#include <WinSock2.h>
#else
#include "signalhandler.h"
#endif

#ifndef APP_VERSION
#define APP_VERSION "Undefined"
#endif

void setupApp(QGuiApplication& app);
void prepareTranslator(QGuiApplication& app, const QString& translationPath, const QString& translationPrefix, const QLocale& locale);
void doExit(int code);

#if defined(QT_STATICPLUGIN)
#include "../backend/NosonApp/plugin.h"
#include "../backend/NosonThumbnailer/plugin.h"
#include "../backend/NosonMediaScanner/plugin.h"
void importStaticPlugins(QQmlEngine* engine)
{
  { NosonAppPlugin e; e.initializeEngine(engine, "NosonApp"); e.registerTypes("NosonApp"); }
  { ThumbnailerPlugin e; e.initializeEngine(engine, "NosonThumbnailer"); e.registerTypes("NosonThumbnailer"); }
  { MediaScannerPlugin e; e.initializeEngine(engine, "NosonMediaScanner"); e.registerTypes("NosonMediaScanner"); }
}
#endif

int main(int argc, char *argv[])
{
    int ret = 0;
#ifdef Q_OS_WIN
    //Initialize Winsock
    WSADATA wsaData;
    if ((ret = WSAStartup(MAKEWORD(2, 2), &wsaData)))
        return ret;
#endif

    QGuiApplication::setApplicationName(APP_NAME);
    QGuiApplication::setApplicationDisplayName(APP_DISPLAY_NAME);
    QGuiApplication::setOrganizationName(ORG_NAME);
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QGuiApplication app(argc, argv);
    setupApp(app);

#if defined(SAILFISHOS)
    QScopedPointer<QQuickView> view(SailfishApp::createView());

#ifdef QT_STATICPLUGIN
    importStaticPlugins(view->engine());
#endif

    QObject::connect(view->engine(), &QQmlApplicationEngine::quit, &app, QCoreApplication::quit);
    // 100MB cache for network data
    view->engine()->setNetworkAccessManagerFactory(new DiskCacheFactory(CACHE_SIZE));
    // bind version string
    view->engine()->rootContext()->setContextProperty("VersionString", QString(APP_VERSION));
    view->setSource(QUrl("qrc:/sfos/harbour-noson.qml"));
    view->showFullScreen();

#else
    QSettings settings;
    QStringList availableStyles;
    QString style = QQuickStyle::name();
    if (!style.isEmpty())
        settings.setValue("style", style);
    else
    {
        if (settings.value("style").isNull())
        {
            QQuickStyle::setStyle("Material");
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

#if defined(Q_OS_ANDROID) || defined(Q_OS_IOS)
    // bind Android flag
    engine.rootContext()->setContextProperty("Android", QVariant(true));
    // select and bind styles available and known to work
    availableStyles.append("Default");
    availableStyles.append("Material");
#else
    // bind Android flag
    engine.rootContext()->setContextProperty("Android", QVariant(false));
    // select and bind styles available and known to work
    for (QString style : QQuickStyle::availableStyles())
    {
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
    }
#endif
    engine.rootContext()->setContextProperty("AvailableStyles", availableStyles);

    // handle signal exit(int) issued by the qml instance
    QObject::connect(&engine, &QQmlApplicationEngine::exit, doExit);

#if defined(QT_STATICPLUGIN)
    importStaticPlugins(&engine);
#endif

    engine.load(QUrl("qrc:/noson.qml"));
    if (engine.rootObjects().isEmpty()) {
        qWarning() << "Failed to load QML";
        return -1;
    }
#endif

    ret = app.exec();
#ifdef Q_OS_WIN
    WSACleanup();
#endif
    return ret;
}

void setupApp(QGuiApplication& app) {
#ifndef Q_OS_WIN
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
#if !defined(Q_OS_ANDROID) && !defined(Q_OS_IOS)
  if (code == 16)
  {
    // loop a short time to flush setting changes
    QTime syncTime = QTime::currentTime().addSecs(1);
    while (QTime::currentTime() < syncTime)
      QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents, 100);

    QStringList args = QCoreApplication::arguments();
    args.removeFirst();
    QProcess::startDetached(QCoreApplication::applicationFilePath(), args);
  }
#else
  (void)code;
#endif
  QCoreApplication::quit();
}
