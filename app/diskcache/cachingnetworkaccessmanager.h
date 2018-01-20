#ifndef CACHINGNETWORKACCESSMANAGER_H
#define CACHINGNETWORKACCESSMANAGER_H

#include <QNetworkAccessManager>

class CachingNetworkAccessManager : public QNetworkAccessManager
{
public:
  CachingNetworkAccessManager(QObject *parent = 0);

  QNetworkReply *createRequest(Operation op, const QNetworkRequest & req, QIODevice * outgoingData) Q_DECL_OVERRIDE;

private:
  QStringList mUrlIgnoreList;
  bool shouldIgnoreUrl(const QString& url);
};

#endif // CACHINGNETWORKACCESSMANAGER_H
