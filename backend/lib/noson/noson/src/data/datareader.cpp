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
#include "datareader.h"
#include "favicon_ico.h"
#ifdef HAVE_PULSEAUDIO
#include "pulseaudio_png.h"
#endif

#include <string>
#include <cassert>

using namespace NSROOT;

DataReader::Resource DataReader::_uris[] = {
  { "/favicon.ico", "image/x-icon", favicon_ico, favicon_ico_len },
#ifdef HAVE_PULSEAUDIO
  { "/pulseaudio.png", "image/png", pulseaudio_png, pulseaudio_png_len },
#endif
};

unsigned DataReader::_uris_len = sizeof(_uris) / sizeof(Resource);

DataReader DataReader::_instance;

DataReader::DataReader()
{
}

DataReader * DataReader::Instance()
{
  return &_instance;
}

StreamReader::STREAM *DataReader::OpenStream(const std::string& streamUrl)
{
  // remove trailing args
  std::string uri = streamUrl.substr(0, streamUrl.find('?'));
  for (unsigned i = 0; i < _uris_len; ++i)
    if (uri.compare(_uris[i].uri) == 0)
    {
      STREAM * stream = new STREAM();
      stream->opaque = &_uris[i];
      stream->contentType = _uris[i].mime;
      stream->contentLength = _uris[i].size;
      stream->data = reinterpret_cast<const char*>(_uris[i].data);
      stream->size = 0;
      return stream;
    }
  return nullptr;
}

int DataReader::ReadStream(StreamReader::STREAM * stream)
{
  if (stream)
  {
    Resource * res = static_cast<Resource*>(stream->opaque);
    assert(res);
    if (!stream->data)
      stream->data = reinterpret_cast<const char*>(res->data);
    else
      stream->data += stream->size;
    int r = static_cast<int>(res->size - (stream->data - reinterpret_cast<const char*>(res->data)));
    stream->size = r > 0 ? static_cast<unsigned>(r) : 0;
    return r;
  }
  return -1;
}

void DataReader::CloseStream(StreamReader::STREAM * stream)
{
  if (stream)
  {
    stream->opaque = nullptr;
    delete stream;
  }
}
