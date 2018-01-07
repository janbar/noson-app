#ifndef DISKCACHEFACTORY_H
#define DISKCACHEFACTORY_H

#include <QQmlNetworkAccessManagerFactory>

class DiskCacheFactory : public QQmlNetworkAccessManagerFactory
{
public:
  DiskCacheFactory(int cacheSize);
  QNetworkAccessManager* create(QObject *parent);

private:
  int mCacheSize;
};

#endif // DISKCACHEFACTORY_H
