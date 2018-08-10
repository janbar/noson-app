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

#include "thumbnailerjob.h"

#include <QDebug>

using namespace thumbnailer;

Job::Job(AbstractWorker* worker, QObject* parent)
: QObject(parent)
, m_worker(worker)
{
  worker->setParent(this);
  connect(m_worker, SIGNAL(finished()), this, SLOT(finishedWorker()));
}

Job::~Job()
{
  // Job is owner of the worker
}

void Job::start()
{
  m_worker->run();
}

ReplyStatus Job::error()
{
  return m_worker->error();
}

int Job::errorCode()
{
  return m_worker->errorCode();
}

QString Job::errorString()
{
  return m_worker->errorString();
}

const QByteArray& Job::image()
{
  return m_worker->image();
}
