#include <QtGlobal>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QSettings>
#include <QQuickStyle>
#include <QTranslator>
#include <QDebug>
#include <Qdir>

#if (defined(_WIN32) || defined(_WIN64))
#define __WINDOWS__
#endif

#ifdef __WINDOWS__
#include <winsock2.h>
#endif

void setupApp(QGuiApplication& app);
QDir getApplicationDir(QGuiApplication& app, QString subdir);
void prepareTranslator(QGuiApplication& app, QString translationPath, QString translationPrefix);

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
        QQuickStyle::setStyle(settings.value("style").toString());

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("availableStyles", QQuickStyle::availableStyles());
    engine.load(QUrl("qrc:/noson.qml"));
    if (engine.rootObjects().isEmpty())
        return -1;

    ret = app.exec();
#ifdef __WINDOWS__
    WSACleanup();
#endif /* __WINDOWS__ */
    return ret;
}

void setupApp(QGuiApplication& app) {
    // Install noson translations
    QTranslator * translator = new QTranslator();
    translator->load(QLocale(), QString("noson"), QString("_"), QString(":/i18n"));
    app.installTranslator(translator);

#ifdef Q_OS_MAC
    static QString relTranslationDir = "Resources/translations";
    // setup translators
    QString translationPath = getApplicationDir(app, relTranslationDir).absolutePath();
    prepareTranslator(app, translationPath, "qt");
#endif
}

QDir getApplicationDir(QGuiApplication& app, QString subdir)
{
    QDir appDir(app.applicationDirPath());
    appDir.cdUp();
    appDir.cd(subdir);
    return appDir;
}

void prepareTranslator(QGuiApplication& app, QString translationPath, QString translationPrefix)
{
    QLocale locale = QLocale();
    QTranslator * translator = new QTranslator();
    if (!translator->load(locale, translationPrefix, QString("_"), translationPath))
    {
        qWarning() << "no file found for translations '"+ translationPath + "/" + translationPrefix + "_" + locale.name().left(2) + ".qm ' (using default).";
    }
}
