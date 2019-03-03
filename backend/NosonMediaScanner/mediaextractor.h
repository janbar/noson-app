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
#ifndef MEDIAEXTRACTOR_H
#define MEDIAEXTRACTOR_H

#include "mediafile.h"
#include "mediainfo.h"
#include "mediarunnable.h"

#define TAG_UNDEFINED  "<Undefined>"

namespace mediascanner
{

typedef void (*MediaExtractorCallback)(void * handle, MediaFilePtr& filePtr);

class MediaExtractor : public MediaRunnable
{
public:
  MediaExtractor(void * handle, MediaExtractorCallback callback, MediaFilePtr& filePtr, bool debug);
  virtual ~MediaExtractor() override {}

  void run() override;

private:
  void * m_handle;
  MediaExtractorCallback m_callback;
  MediaFilePtr m_filePtr;
};

}

#endif /* MEDIAEXTRACTOR_H */
