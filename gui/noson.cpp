#include <QtGlobal>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QSettings>
#include <QQuickStyle>
#include <QTranslator>
#include <QDebug>
#include <QDir>

#include "diskcache/diskcachefactory.h"

#if (defined(_WIN32) || defined(_WIN64))
#define __WINDOWS__
#endif

#ifdef __WINDOWS__
#include <winsock2.h>
#else
#include "signalhandler.h"
#endif

void setupApp(QGuiApplication& app);
QDir getApplicationDir(QGuiApplication& app, const QString& subdir);
void prepareTranslator(QGuiApplication& app, const QString& translationPath, const QString& translationPrefix, const QLocale& locale);

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
            QQuickStyle::setStyle("Universal");
            settings.setValue("style", QQuickStyle::name());
        }
        QQuickStyle::setStyle(settings.value("style").toString());
    }

    QQmlApplicationEngine engine;
    // 100MB cache for network data
    engine.setNetworkAccessManagerFactory(new DiskCacheFactory(100 * 1000 * 1000));
    // bind arguments
    engine.rootContext()->setContextProperty("ApplicationArguments", app.arguments());
    // select and bind styles available and known to work
    QStringList availableStyles;
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
    engine.rootContext()->setContextProperty("AvailableStyles", availableStyles);

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
    QString translationPath = getApplicationDir(app, QString("Resources/translations")).absolutePath();
    prepareTranslator(app, translationPath, "qt", locale);
#endif
}

QDir getApplicationDir(QGuiApplication& app, const QString& subdir)
{
    QDir appDir(app.applicationDirPath());
    appDir.cdUp();
    appDir.cd(subdir);
    return appDir;
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
