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
#include "mediascanner.h"
#include "mediascannerengine.h"
#include "mediaparser.h"
#include "flacparser.h"
#include "id3parser.h"
#include "m4aparser.h"
#include "oggparser.h"
#include "listmodel.h"

#include <QDebug>

static int mediaFilePtr_id = qRegisterMetaType<mediascanner::MediaFilePtr>("MediaFilePtr");

using namespace mediascanner;

MediaScanner * MediaScanner::_instance = nullptr;

MediaScanner::MediaScanner(QObject *parent)
: QObject(parent)
, m_engine(new MediaScannerEngine(this))
, m_debug(false)
{
  m_engine->addParser(new FLACParser);
  m_engine->addParser(new ID3Parser);
  m_engine->addParser(new OGGParser);
#ifdef ENABLE_MP4PARSER
  m_engine->addParser(new M4AParser);
#endif
}

MediaScanner* MediaScanner::instance(QObject* parent)
{
  if (_instance == nullptr)
    _instance = new MediaScanner(parent);
  return _instance;
}

MediaScanner::~MediaScanner()
{
  if (m_engine->isRunning())
    m_engine->stop();
  delete m_engine;
}

void MediaScanner::start(int maxThread /*=MEDIASCANNER_MAX_THREAD*/) {
  m_engine->setMaxThread(maxThread);
  if (!m_engine->isRunning())
  {
    m_engine->start();
  }
}

void MediaScanner::debug(bool enable)
{
  m_debug = enable;
}

bool MediaScanner::emptyState() const
{
  return m_engine ? m_engine->emptyState() : true;
}

bool MediaScanner::working() const
{
  return m_engine ? m_engine->working() : false;
}

void MediaScanner::registerModel(ListModel * model)
{
  if (model)
  {
    if (isDebug())
      qDebug("%s: %p", __FUNCTION__, model);
    connect(this, &MediaScanner::put, model, &ListModel::onFileAdded);
    connect(this, &MediaScanner::remove, model, &ListModel::onFileRemoved);
  }
}

void MediaScanner::unregisterModel(ListModel * model)
{
  if (model)
  {
    if (isDebug())
      qDebug("%s: %p", __FUNCTION__, model);
    disconnect(this, &MediaScanner::put, model, &ListModel::onFileAdded);
    disconnect(this, &MediaScanner::remove, model, &ListModel::onFileRemoved);
  }
}

QList<MediaFilePtr> MediaScanner::allParsedFiles() const
{
  return m_engine->allParsedFiles();
}
