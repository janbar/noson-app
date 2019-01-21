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

#include <QNetworkReply>

#include "netmanager.h"

using namespace thumbnailer;

NetManager::NetManager(QObject* parent)
: QObject(parent)
, m_nam(new QNetworkAccessManager())
{
  connect(this, SIGNAL(request(NetRequest*)), this, SLOT(onRequest(NetRequest*)));
}

NetManager::~NetManager()
{
  delete m_nam;
}

QNetworkAccessManager* NetManager::networkAccessManager()
{
  return m_nam;
}

void NetManager::onRequest(NetRequest* wr)
{
  QNetworkReply* reply;
  switch (wr->getOperation())
  {
    case QNetworkAccessManager::GetOperation:
      reply = m_nam->get(wr->getRequest());
      break;
    case QNetworkAccessManager::HeadOperation:
      reply = m_nam->head(wr->getRequest());
      break;
    case QNetworkAccessManager::PostOperation:
      reply = m_nam->post(wr->getRequest(), wr->getData());
      break;
    case QNetworkAccessManager::PutOperation:
      reply = m_nam->put(wr->getRequest(), wr->getData());
      break;
    case QNetworkAccessManager::DeleteOperation:
      reply = m_nam->deleteResource(wr->getRequest());
      break;
    default:
      qDebug().noquote() << "NetManager::onRequest(): Unknown operation";
      QMetaObject::invokeMethod(wr, "requestAborted", Qt::QueuedConnection);
      return;
  }
  wr->newReply(this, reply);
}
