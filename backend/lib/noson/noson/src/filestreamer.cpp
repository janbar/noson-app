/*
 *      Copyright (C) 2018-2019 Jean-Luc Barriere
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
#include "filestreamer.h"
#include "imageservice.h"
#include "data/datareader.h"
#include "private/debug.h"
#include "private/urlencoder.h"
#include "private/tokenizer.h"

#include <cstring>
#include <cstdio>
#include <cassert>
#include <cinttypes>
#include <climits>

#define FILESTREAMER_TIMEOUT  10000
#define FILESTREAMER_MAX_PB   5
#define FILESTREAMER_CHUNK    16384

using namespace NSROOT;

FileStreamer::codec_type FileStreamer::codecTypeTab[] = {
  { "flac"          , "flac" , "audio/flac" },
  { "mpeg1layer3"   , "mp3"  , "audio/mpeg" },
  { "mpeg2layer3"   , "mp3"  , "audio/mpeg" },
  { "mpeg1layer2"   , "mp2"  , "audio/mpeg" },
  { "mpeg2layer2"   , "mp2"  , "audio/mpeg" },
  { "mpeg2aac"      , "aac"  , "audio/aac"  },
  { "mpeg4aac"      , "aac"  , "audio/aac"  },
  { "vorbis"        , "ogg"  , "application/ogg" },
};

int FileStreamer::codecTypeTabSize = sizeof(FileStreamer::codecTypeTab) / sizeof(FileStreamer::codec_type);

FileStreamer::file_type FileStreamer::fileTypeTab[] = {
  { "audio/flac"        , &FileStreamer::probeFLAC  , Transfer_Chunked },
  { "audio/mpeg"        , &FileStreamer::probeMPEG  , Transfer_Chunked },
  { "audio/aac"         , &FileStreamer::probeMPEG  , Transfer_Chunked },
  { "application/ogg"   , &FileStreamer::probeOGGS  , Transfer_ByRange },
};

int FileStreamer::fileTypeTabSize = sizeof(FileStreamer::fileTypeTab) / sizeof(FileStreamer::file_type);

FileStreamer::FileStreamer()
: SONOS::RequestBroker()
, m_resources()
, m_playbackCount(0)
{
  // declare the static resources for available codecs
  for (int i = 0; i < codecTypeTabSize; ++i)
  {
    ResourcePtr ptr = ResourcePtr(new Resource());
    ptr->uri.assign(FILESTREAMER_URI).append(".").append(codecTypeTab[i].suffix);
    ptr->title = codecTypeTab[i].codec;
    ptr->description = "Stream file";
    ptr->contentType = codecTypeTab[i].mime;
    m_resources.push_back(ptr);
  }
}


bool FileStreamer::HandleRequest(handle * handle)
{
  const std::string& requrl = RequestBroker::GetRequestURI(handle);
  ResourceList::const_iterator it = m_resources.begin();
  while (!IsAborted() && it != m_resources.end())
  {
    if (requrl.compare(0, (*it)->uri.length(), (*it)->uri) == 0)
    {
      std::vector<std::string> params;
      readParameters(requrl, params);
      std::string filePath = getParamValue(params, FILESTREAMER_PARAM_PATH);
      if (probe(filePath, (*it)->contentType))
      {
        // define the transfer type to use
        TransferType transfer = Transfer_Chunked;
        for (int t = 0; t < fileTypeTabSize; ++t)
        {
          if ((*it)->contentType.compare(fileTypeTab[t].mime) == 0)
          {
            transfer = fileTypeTab[t].transfer;
            break;
          }
        }
        
        switch (RequestBroker::GetRequestMethod(handle))
        {
        case Method_GET:
          if (transfer == Transfer_ByRange)
            streamFileByRange(handle, filePath, (*it)->contentType, RequestBroker::GetRequestHeader(handle, "RANGE"));
          else
            streamFileByChunk(handle, filePath, (*it)->contentType);
          return true;
        case Method_HEAD:
        {
          std::string resp;
          resp.assign(RequestBroker::MakeResponseHeader(Status_OK))
              .append("Content-type: ").append((*it)->contentType).append("\r\n");
          if (transfer == Transfer_ByRange)
            resp.append("Content-length: ").append(std::to_string(getFileLength(filePath))).append("\r\n");
          resp.append("\r\n");
          RequestBroker::Reply(handle, resp.c_str(), resp.length());
          return true;
        }
        default:
          return false; // unhandled method
        }
      }
      else
      {
        DBG(DBG_WARN, "%s: probing file failed (%s)\n", __FUNCTION__, filePath.c_str());
        Reply500(handle);
      }
      return true;
    }
    ++it;
  }
  return false;
}

RequestBroker::ResourcePtr FileStreamer::GetResource(const std::string& title)
{
  for (ResourceList::iterator it = m_resources.begin(); it != m_resources.end(); ++it)
  {
    if ((*it)->title == title)
      return (*it);
  }
  return ResourcePtr();
}

RequestBroker::ResourceList FileStreamer::GetResourceList()
{
  ResourceList list;
  for (ResourceList::iterator it = m_resources.begin(); it != m_resources.end(); ++it)
    list.push_back((*it));
  return list;
}

RequestBroker::ResourcePtr FileStreamer::RegisterResource(const std::string& title,
                                                           const std::string& description,
                                                           const std::string& path,
                                                           StreamReader * delegate)
{
  (void)title;
  (void)description;
  (void)path;
  (void)delegate;
  return ResourcePtr();
}

void FileStreamer::UnregisterResource(const std::string& uri)
{
  (void)uri;
}

const FileStreamer::codec_type * FileStreamer::GetCodec(const std::string& codecName)
{
  for (int i = 0; i < codecTypeTabSize; ++i)
    if (codecName.compare(codecTypeTab[i].codec) == 0)
      return &codecTypeTab[i];
  return nullptr;
}

std::string FileStreamer::MakeFileStreamURI(const std::string& filePath, const std::string& codecName)
{
  std::string streamUri;
  // find the resource for my codec else return null
  ResourcePtr res = GetResource(codecName);
  if (!res)
    return streamUri;
  // encode the file path
  std::string pathParm(urlencode(filePath));
  // make the stream uri
  if (res->uri.find('?') != std::string::npos)
    streamUri.assign(res->uri).append("&path=").append(pathParm);
  else
    streamUri.assign(res->uri).append("?path=").append(pathParm);

  return streamUri;
}

void FileStreamer::readParameters(const std::string& streamUrl, std::vector<std::string>& params)
{
  size_t s = streamUrl.find('?');
  if (s != std::string::npos)
    tokenize(streamUrl.substr(s + 1), "&", params, true);
}

std::string FileStreamer::getParamValue(const std::vector<std::string>& params, const std::string& name)
{
  size_t lval = name.length() + 1;
  for (const std::string& str : params)
  {
    if (str.length() > lval && str.at(name.length()) == '=' && str.compare(0, name.length(), name) == 0)
      return urldecode(str.substr(lval));
  }
  return std::string();
}

size_t FileStreamer::getFileLength(FILE * file)
{
  size_t ret = 0;
  if (file)
  {
    int64_t p = 0;
    int64_t c = ftell(file);
    if (c >= 0)
    {
      if (fseek(file, 0, SEEK_END) == 0 && (p = ftell(file)) > 0)
        ret = (size_t)p;
      fseek(file, c, SEEK_SET);
    }
  }
  return ret;
}

size_t FileStreamer::getFileLength(const std::string& filePath)
{
  size_t ret = 0;
  FILE * file = fopen(filePath.c_str(), "rb");
  if (file)
  {
    ret = getFileLength(file);
    fclose(file);
  }
  return ret;
}

bool FileStreamer::probe(const std::string& filePath, const std::string& mimeType)
{
  for (int i = 0; i < fileTypeTabSize; ++i)
  {
    if (mimeType.compare(fileTypeTab[i].mime) == 0 && fileTypeTab[i].probe(filePath))
      return true;
  }
  return false;
}

bool FileStreamer::probeFLAC(const std::string& filePath)
{
  bool ret = false;
  FILE * file = fopen(filePath.c_str(), "rb");
  if (file)
  {
    char buf[4];
    if (fread(buf, 1, 4, file) == 4 && memcmp(buf, "fLaC", 4) == 0)
      ret = true;
    fclose(file);
  }
  return ret;
}

bool FileStreamer::probeMPEG(const std::string& filePath)
{
  bool ret = false;
  FILE * file = fopen(filePath.c_str(), "rb");
  if (file)
  {
    unsigned char buf[10];
    if (fread(buf, 1, 10, file) == 10)
    {
      // unstack id3 tags
      while (memcmp(buf, "ID3", 3) == 0)
      {
        unsigned offset = 0;
        for (int i = 6; i < 10; ++i)
          offset |= buf[i] << ((9 - i) * 7);
        if ((buf[5] & 0x10) == 0x10) // check for extended footer
          offset += 10;
        if (!offset || fseek(file, offset, SEEK_CUR) != 0 || fread(buf, 1, 10, file) != 10)
        {
          fclose(file);
          return false;
        }
      }
      // check mpeg synchro
      if (buf[0] == 0xff)
      {
        unsigned type = buf[1] & 0xfe;
        if (type == 0xfa || // version 1 layer 3
            type == 0xfc || // version 1 layer 2
            type == 0xf2 || // version 2 layer 3
            type == 0xf4 || // version 2 layer 2
            type == 0xf8 || // version 2 aac
            type == 0xf0)   // version 4 aac
          ret = true;
      }
    }
    fclose(file);
  }
  return ret;
}

bool FileStreamer::probeOGGS(const std::string& filePath)
{
  bool ret = false;
  FILE * file = fopen(filePath.c_str(), "rb");
  if (file)
  {
    char buf[4];
    if (fread(buf, 1, 4, file) == 4 && memcmp(buf, "OggS", 4) == 0)
      ret = true;
    fclose(file);
  }
  return ret;
}

FileStreamer::range FileStreamer::bytesRange(const std::string &rangeValue, size_t size)
{
  range rg = { 0L, 0L };
  long int a = 0, b = LONG_MAX;
  sscanf(rangeValue.c_str(), "bytes=%li%li", &a, &b);
  if (a < 0)
  {
    rg.start = (size > (size_t)(0L - a) ? size + a : 0);
    rg.end = size - 1;
  }
  else
  {
    rg.start = (size_t)a;
    rg.end = (size_t)(b < 0 ? 0L - b : b);
    if (rg.end >= size)
      rg.end = size - 1;
  }
  return rg;
}

void FileStreamer::streamFileByChunk(handle * handle, const std::string& filePath, const std::string& mimeType)
{
  assert(!mimeType.empty());
  int id = m_playbackCount.Add(1);
  FILE * file = nullptr;

  if (m_playbackCount.Load() > FILESTREAMER_MAX_PB)
  {
    Reply429(handle);
  }
  if (!(file = fopen(filePath.c_str(), "rb")))
  {
    DBG(DBG_WARN, "%s: opening file failed (%s)\n", __FUNCTION__, filePath.c_str());
    Reply500(handle);
  }
  else
  {
    DBG(DBG_INFO, "%s: open stream #%d (%s) type (%s)\n", __FUNCTION__, id, filePath.c_str(), mimeType.c_str());
    size_t tb = 0; // count transfered bytes
    std::string resp;
    resp.assign(RequestBroker::MakeResponseHeader(Status_OK))
        .append("Content-type: ").append(mimeType).append("\r\n")
        .append("Transfer-encoding: chunked\r\n")
        .append("\r\n");

    if (RequestBroker::Reply(handle, resp.c_str(), resp.length()))
    {
      char * buf = new char [FILESTREAMER_CHUNK + 16];
      size_t r = 0;
      while (!IsAborted() && (r = fread(buf + 7, 1, FILESTREAMER_CHUNK, file)) > 0)
      {
        char str[8];
        snprintf(str, sizeof(str), "%05x\r\n", (unsigned)r & 0xfffff);
        memcpy(buf, str, 7);
        memcpy(buf + r + 7, "\r\n", 2);
        if (!RequestBroker::Reply(handle, buf, r + 7 + 2))
          break;
        tb += r;
      }
      delete [] buf;
      if (r == 0)
        RequestBroker::Reply(handle, "0\r\n\r\n", 5);
    }
    DBG(DBG_INFO, "%s: close stream #%d length (%lu)\n", __FUNCTION__, id, (long unsigned)tb);
    fclose(file);
  }

  m_playbackCount.Sub(1);
}

void FileStreamer::streamFileByRange(handle * handle, const std::string& filePath, const std::string& mimeType, const std::string& rangeValue)
{
  assert(!mimeType.empty());
  int id = m_playbackCount.Add(1);
  FILE * file = nullptr;

  if (m_playbackCount.Load() > FILESTREAMER_MAX_PB)
  {
    Reply429(handle);
  }
  if (!(file = fopen(filePath.c_str(), "rb")))
  {
    DBG(DBG_WARN, "%s: opening file failed (%s)\n", __FUNCTION__, filePath.c_str());
    Reply500(handle);
  }
  else
  {
    DBG(DBG_INFO, "%s: open stream #%d (%s) type (%s) range (%s)\n", __FUNCTION__, id, filePath.c_str(), mimeType.c_str(), rangeValue.c_str());
    size_t tb = 0; // count transfered bytes
    size_t flen = getFileLength(file);
    range rg = bytesRange(rangeValue, flen);
    if (rg.end >= rg.start && fseek(file, rg.start, SEEK_SET) == 0)
    {
      size_t len = rg.end - rg.start + 1;
      std::string resp;
      if (len != flen)
      {
        resp.assign(RequestBroker::MakeResponseHeader(Status_Partial_Content))
            .append("Content-range: bytes ")
            .append(std::to_string(rg.start)).append("-").append(std::to_string(rg.end))
            .append("/").append(std::to_string(flen)).append("\r\n");
      }
      else
      {
        resp.assign(RequestBroker::MakeResponseHeader(Status_OK));
      }
      resp.append("Content-type: ").append(mimeType).append("\r\n")
          .append("Content-length: ").append(std::to_string(len)).append("\r\n")
          .append("\r\n");

      if (RequestBroker::Reply(handle, resp.c_str(), resp.length()))
      {
        char * buf = new char [FILESTREAMER_CHUNK];
        size_t r = 0;
        size_t chunk = (len > FILESTREAMER_CHUNK ? FILESTREAMER_CHUNK : len);
        while (!IsAborted() && chunk > 0 && (r = fread(buf, 1, chunk, file)) > 0)
        {
          if (!RequestBroker::Reply(handle, buf, r))
            break;
          tb += r;
          len -= r;
          if (len < FILESTREAMER_CHUNK)
            chunk = len;
        }
        delete [] buf;
      }
    }
    else
    {
      Reply500(handle);
      DBG(DBG_WARN, "%s: invalid seek (%s) (%lu-%lu)\n", __FUNCTION__, filePath.c_str(), (long unsigned)rg.start, (long unsigned)rg.end);
    }
    DBG(DBG_INFO, "%s: close stream #%d length (%lu)\n", __FUNCTION__, id, (long unsigned)tb);
    fclose(file);
  }

  m_playbackCount.Sub(1);
}

void FileStreamer::Reply500(handle * handle)
{
  std::string resp;
  resp.assign(RequestBroker::MakeResponseHeader(Status_Internal_Server_Error))
      .append("\r\n");
  RequestBroker::Reply(handle, resp.c_str(), resp.length());
}

void FileStreamer::Reply400(handle * handle)
{
  std::string resp;
  resp.append(RequestBroker::MakeResponseHeader(Status_Bad_Request))
      .append("\r\n");
  RequestBroker::Reply(handle, resp.c_str(), resp.length());
}

void FileStreamer::Reply429(handle * handle)
{
  std::string resp;
  resp.append(RequestBroker::MakeResponseHeader(Status_Too_Many_Requests))
      .append("\r\n");
  RequestBroker::Reply(handle, resp.c_str(), resp.length());
}
