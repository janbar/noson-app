/*
 *      Copyright (C) 2019 Jean-Luc Barriere
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#ifndef MEDIADELAYEDRUNNABLE_H
#define MEDIADELAYEDRUNNABLE_H

#include <QRunnable>
#include <QElapsedTimer>

namespace mediascanner
{

class MediaRunnable : public QRunnable
{
public:
  MediaRunnable(bool debug);
  virtual ~MediaRunnable() override;
  void setTimeout(qint64 timeout);
  qint64 timeLeft();

protected:
    bool m_debug;

private:
  QElapsedTimer * m_timer;
  qint64 m_timeout;
};

}


#endif /* MEDIADELAYEDRUNNABLE_H */

