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
#include "mediaextractor.h"

using namespace mediascanner;

MediaExtractor::MediaExtractor(void * handle, MediaExtractorCallback callback, MediaFilePtr& filePtr, bool debug)
: MediaRunnable(debug)
, m_handle(handle)
, m_callback(callback)
, m_filePtr(filePtr)
{
}

void MediaExtractor::run()
{
  if (m_callback)
  {
    MediaInfoPtr infoPtr(new MediaInfo());
    if (m_filePtr->parser->parse(m_filePtr.data(), infoPtr.data(), m_debug))
    {
      // default undefined tags
      if (infoPtr->album.isEmpty())
        infoPtr->album = TAG_UNDEFINED;
      if (infoPtr->artist.isEmpty())
        infoPtr->artist = TAG_UNDEFINED;
      if (infoPtr->genre.isEmpty())
        infoPtr->genre = TAG_UNDEFINED;

      //qDebug("parsing %s (%s) succeeded", m_filePtr->filePath.toUtf8().constData(), m_filePtr->parser->commonName());
      m_filePtr->mediaInfo.swap(infoPtr);
      m_filePtr->isValid = true;
      m_callback(m_handle, m_filePtr);

#if 0
      MediaInfo * info = m_filePtr->mediaInfo.data();
      qDebug("title       = %s", info->title.toUtf8().constData());
      qDebug("album       = %s", info->album.toUtf8().constData());
      qDebug("artist      = %s", info->artist.toUtf8().constData());
      qDebug("genre       = %s", info->genre.toUtf8().constData());
      qDebug("track no    = %d", info->trackNo);
      qDebug("year        = %d", info->year);
      qDebug("container   = %s", info->container.toUtf8().constData());
      qDebug("codec       = %s", info->codec.toUtf8().constData());
      qDebug("sample rate = %d", info->sampleRate);
      qDebug("bit rate    = %d", info->bitRate);
      qDebug("channels    = %d", info->channels);
      qDebug("duration    = %d", info->duration);
#endif
    }
    else
    {
      qWarning("parsing %s (%s) failed", m_filePtr->filePath.toUtf8().constData(), m_filePtr->parser->commonName());
      m_filePtr->isValid = false;
      m_callback(m_handle, m_filePtr);
    }

  }
}
