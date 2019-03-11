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
#include "mediarunnable.h"

using namespace mediascanner;

MediaRunnable::MediaRunnable(bool debug)
: QRunnable()
, m_debug(debug)
, m_timer(nullptr)
, m_timeout(0)
{
}

MediaRunnable::~MediaRunnable()
{
  if (m_timer)
    delete m_timer;
}

void MediaRunnable::setTimeout(qint64 timeout)
{
  if (timeout < 0)
    m_timeout = 0;
  else if (!m_timer)
  {
    m_timer = new QElapsedTimer();
    m_timer->start();
  }
  m_timeout = timeout;
}

qint64 MediaRunnable::timeLeft()
{
  if (!m_timer)
    return 0;
  return m_timeout - m_timer->elapsed();
}
