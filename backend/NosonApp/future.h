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

#ifndef NOSONAPPFUTURE_H
#define NOSONAPPFUTURE_H

#include <QObject>
#include <QVariant>
#include <QRunnable>

namespace nosonapp
{

class Sonos;

class Promise : public QRunnable
{
public:
  virtual ~Promise();

  const QVariant& result() const { return m_result; }

protected:
  void setResult(const QVariant& result) { m_result = result; }

private:
  QVariant m_result;
};

class Future : public QObject, public QRunnable
{
  Q_OBJECT
public:
  explicit Future(Promise * promise, Sonos * sonos);
  virtual ~Future();

  Q_INVOKABLE bool start(bool longOp = true);

  void run() override;

signals:
  void started();
  void finished(QVariant result);

private:
  Promise * m_promise;
  Sonos * m_sonos;
  bool m_longOp;
};

}

#endif // NOSONAPPFUTURE_H
