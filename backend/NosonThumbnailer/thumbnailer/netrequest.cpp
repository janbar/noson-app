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

#include "netrequest.h"
#include "netmanager.h"

#include <QNetworkReply>
#include <QUrl>
#include <QDebug>

using namespace thumbnailer;

NetRequest::NetRequest(QObject* parent)
: QObject(parent)
, m_enableRedirect(true)
, m_redirect(false)
, m_operation(QNetworkAccessManager::GetOperation)
, m_nam(nullptr)
, m_reply(nullptr)
, m_httpRequestAborted(false)
, m_httpReplyError(false)
, m_errorCode(QNetworkReply::NoError)
{
}

NetRequest::~NetRequest()
{
  if (m_reply)
    m_reply->deleteLater();
}

void NetRequest::launch(NetManager* nam)
{
  emit nam->request(this);
}

void NetRequest::redirect(bool enabled)
{
  m_enableRedirect = enabled;
}

void NetRequest::newReply(NetManager* nam, QNetworkReply* reply)
{
  m_httpRequestAborted = false;
  m_httpReplyError = false;
  m_nam = nam;
  m_reply = reply;
  connect(m_reply, &QNetworkReply::finished, this, &NetRequest::replyFinished);
  connect(m_reply, &QIODevice::readyRead, this, &NetRequest::replyReadyRead);
#ifndef QT_NO_SSL
  connect(m_reply, &QNetworkReply::sslErrors, this, &NetRequest::sslErrors);
#endif
}

bool NetRequest::atEnd()
{
  Q_ASSERT(m_reply);
  return m_reply->atEnd();
}

QByteArray NetRequest::readData()
{
  Q_ASSERT(m_reply);
  return m_reply->peek(m_reply->bytesAvailable());
}

void NetRequest::cancel()
{
  Q_ASSERT(m_reply);
  m_httpRequestAborted = true;
  m_reply->abort();
}

void NetRequest::setUrl(const QUrl &url)
{
  m_request.setUrl(url);
}

void NetRequest::setOperation(QNetworkAccessManager::Operation operation)
{
  m_operation = operation;
}

void NetRequest::setHeader(QNetworkRequest::KnownHeaders header, const QVariant &value)
{
  m_request.setHeader(header, value);
}

void NetRequest::setHeader(const QString &header, const QString &value)
{
  m_request.setRawHeader(header.toUtf8(), value.toUtf8());
}

void NetRequest::setData(const QByteArray &data)
{
  m_postData = data;
}

QStringList NetRequest::getAllResponseHeaders() const
{
  Q_ASSERT(m_reply);
  QStringList list;
  foreach(QNetworkReply::RawHeaderPair e, m_reply->rawHeaderPairs())
  {
    QByteArray h;
    h.append(e.first).append(": ").append(e.second);
    list.push_back(QString::fromUtf8(h));
  }
  return list;
}

QString NetRequest::getResponseHeader(const QString& header) const
{
  Q_ASSERT(m_reply);
  QByteArray h = header.toUtf8();
  if (m_reply->hasRawHeader(h))
    return QString::fromUtf8(m_reply->rawHeader(h));
  return QString();
}

void NetRequest::requestAborted()
{
  m_httpRequestAborted = true;
  m_httpReplyError = true;
  m_errorCode = QNetworkReply::OperationCanceledError;
  m_errorString = "Invalid request";
  emit finished(this);
}

void NetRequest::replyFinished()
{
  if (m_httpRequestAborted)
  {
    m_reply->deleteLater();
    m_reply = nullptr;
    return;
  }

  if (m_reply->error())
  {
    m_httpReplyError = true;
    m_errorCode = m_reply->error();
    m_errorString = m_reply->errorString();
  }
  else
  {
    const QVariant redirectionTarget = m_reply->attribute(QNetworkRequest::RedirectionTargetAttribute);

    if (!redirectionTarget.isNull())
    {
      const QUrl redirectedUrl = m_request.url().resolved(redirectionTarget.toUrl());
      if (m_enableRedirect)
      {
        if (!m_redirect)
        {
          m_reply->deleteLater();
          m_reply = nullptr;
          m_redirect = true;
          setUrl(redirectedUrl);
          launch(m_nam);
          return;
        }
        m_httpReplyError = true;
        m_errorCode = QNetworkReply::NetworkError::TooManyRedirectsError;
        m_errorString = "Too many redirects";
      }
      else
      {
        m_httpReplyError = true;
        m_errorCode = QNetworkReply::NetworkError::InsecureRedirectError;
        m_errorString = "Redirection denied: " + m_request.url().toString();
      }
    }
  }

  emit finished(this);
}

void NetRequest::replyReadyRead()
{
  // this slot gets called every time the QNetworkReply has new data.
  emit readyRead(this);
}

#ifndef QT_NO_SSL

void NetRequest::sslErrors(const QList<QSslError> &errors)
{
  QString errorString;

  foreach(const QSslError &error, errors)
  {
    if (!errorString.isEmpty())
      errorString += '\n';
    errorString += error.errorString();
  }
  qWarning() << errorString;
  m_reply->ignoreSslErrors();
}

#endif
