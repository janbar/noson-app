#ifndef CACHEREPLY_H
#define CACHEREPLY_H

#include <QNetworkReply>
#include <QNetworkCacheMetaData>

//QNetworkReply implementation which reads from the QNetworkDiskCache
//partly copied from QNetworkReplyDataImpl, which is private
class CacheReply : public QNetworkReply
{
public:
  CacheReply(QIODevice *cacheDev, const QNetworkRequest & req,
             QNetworkAccessManager::Operation op,
             QNetworkCacheMetaData meta, QObject *parent = 0);
  virtual ~CacheReply();

  void abort() Q_DECL_OVERRIDE;
  qint64 bytesAvailable() const Q_DECL_OVERRIDE;
  bool isSequential() const Q_DECL_OVERRIDE;
  qint64 size() const Q_DECL_OVERRIDE;
  qint64 readData(char *data, qint64 maxlen) Q_DECL_OVERRIDE;

private:
  QIODevice *mCacheDev;
};

#endif // CACHEREPLY_H
