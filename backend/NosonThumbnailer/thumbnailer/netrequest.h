/*
 *      Copyright (C) 2018-2019 Jean-Luc Barriere
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

#ifndef NETREQUEST_H
#define NETREQUEST_H

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QByteArray>
#include <QString>
#include <QUrl>
#include <QTimer>

namespace thumbnailer
{

  class NetManager;

  class NetRequest : public QObject
  {
    Q_OBJECT

  public:
    explicit NetRequest(QObject* parent = 0);
    virtual ~NetRequest();

    void launch(NetManager* nam);

    void redirect(bool enabled);

    void newReply(NetManager* nam, QNetworkReply* reply);

    bool atEnd();
    QByteArray readData();
    void cancel();

    const QNetworkRequest& getRequest()
    {
      return m_request;
    }

    void setUrl(const QUrl& url);

    void setOperation(QNetworkAccessManager::Operation operation);

    void setHeader(QNetworkRequest::KnownHeaders header, const QVariant& value);
    void setHeader(const QString &header, const QString &value);

    void setData(const QByteArray& data);

    QNetworkAccessManager::Operation getOperation() const
    {
      return m_operation;
    }

    const QByteArray& getData() const
    {
      return m_postData;
    }

    bool error() const
    {
      return m_httpReplyError;
    }

    int errorCode() const
    {
      return m_errorCode;
    }

    const QString& errorString() const
    {
      return m_errorString;
    }

    int httpStatusCode() const
    {
      return m_httpStatusCode;
    }

    QStringList getAllResponseHeaders() const;

    QString getResponseHeader(const QString& header) const;

    signals:
    void readyRead(NetRequest*);
    void finished(NetRequest*);

  private slots:
    void requestAborted();
    void replyFinished();
    void replyReadyRead();
#ifndef QT_NO_SSL
    void sslErrors(const QList<QSslError> &errors);
#endif

  private:
    bool m_enableRedirect;
    bool m_redirect;
    QNetworkAccessManager::Operation m_operation;
    QNetworkRequest m_request;
    QByteArray m_postData;
    NetManager* m_nam;
    QNetworkReply* m_reply;
    bool m_httpRequestAborted;
    bool m_httpReplyError;
    int m_errorCode;
    int m_httpStatusCode;
    QString m_errorString;
  };

}
#endif /* NETREQUEST_H */

