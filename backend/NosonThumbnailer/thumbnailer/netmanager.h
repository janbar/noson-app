/*
 *      Copyright (C) 2018 Jean-Luc Barriere
 *
 *  This file is part of Noson-App
 *
 *  Noson-App is free software: you can redistribute it and/or modify
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
 *  along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef NETMANAGER_H
#define NETMANAGER_H

#include "netrequest.h"

#include <QNetworkAccessManager>

namespace thumbnailer
{

  class NetManager : public QObject
  {
    Q_OBJECT
  public:
    NetManager(QObject* parent = 0);
    ~NetManager();

    QNetworkAccessManager* networkAccessManager();

  signals:
    void request(NetRequest*);

  private slots:
    void onRequest(NetRequest* wr);

  private:
    QNetworkAccessManager* m_nam;
  };

}
#endif /* NETMANAGER_H */

