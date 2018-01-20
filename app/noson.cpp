#include <QtGlobal>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickView>

#include "signalhandler.h"
#include "diskcache/diskcachefactory.h"

int main(int argc, char *argv[])
{
    int ret = 0;

    QGuiApplication::setApplicationName("noson");
    QGuiApplication::setOrganizationName("janbar");

    QGuiApplication app(argc, argv);
    SignalHandler *sh = new SignalHandler(&app);
    sh->catchSignal(SIGHUP, 0);
    sh->catchSignal(SIGALRM, 0);

    QQuickView view;
    view.setResizeMode(QQuickView::SizeRootObjectToView);
    view.setSource(QUrl("qrc:/main.qml"));

    // 100MB cache for network data
    view.engine()->setNetworkAccessManagerFactory(new DiskCacheFactory(100 * 1000 * 1000));

    view.show();

    ret = app.exec();
    return ret;
}
