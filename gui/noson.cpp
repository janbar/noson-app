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
            QQuickStyle::setStyle("Material");
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
