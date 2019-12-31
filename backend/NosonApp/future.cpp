/*
 *      Copyright (C) 2019 Jean-Luc Barriere
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

#include "future.h"
#include "sonos.h"

#include <QDebug>

using namespace nosonapp;

Promise::~Promise()
{
  //qDebug("%s: free %p", __FUNCTION__, this);
}

Future::Future(Promise * promise, Sonos * sonos)
: QObject(sonos)
, m_promise(promise)
, m_sonos(sonos)
, m_longOp(true)
{
  Q_ASSERT(m_promise);
  Q_ASSERT(m_sonos);
  QRunnable::setAutoDelete(false);
}

Future::~Future()
{
  delete m_promise;
}

bool Future::start(bool longOp)
{
  m_longOp = longOp;
  return m_sonos->startJob(this);
}

void Future::run()
{
  bool lp = m_longOp;
  emit started();
  if (lp)
    m_sonos->beginJob();
  m_promise->run();
  if (lp)
    m_sonos->endJob();
  emit finished(m_promise->result());
  deleteLater();
}
