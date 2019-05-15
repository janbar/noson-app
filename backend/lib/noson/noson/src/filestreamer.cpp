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
#include "requestbroker.h"
#include "imageservice.h"
#include "data/datareader.h"
#include "private/debug.h"
#include "private/urlencoder.h"
#include "private/tokenizer.h"

#include <cstring>
#include <cstdio>
#include <cassert>

#define FILESTREAMER_URI      "/music/track"
#define FILESTREAMER_TIMEOUT  10000
#define FILESTREAMER_MAX_PB   5
#define SERVER_PRODUCT_NAME   "libnoson/" LIBVERSION

using namespace NSROOT;

FileStreamer::codec_type FileStreamer::codecTypeTab[] = {
  { "flac"          , "flac" , "audio/flac" },
  { "mpeg1layer3"   , "mp3"  , "audio/mpeg" },
  { "mpeg2layer3"   , "mp3"  , "audio/mpeg" },
  { "mpeg1layer2"   , "mp2"  , "audio/mpeg" },
  { "mpeg2layer2"   , "mp2"  , "audio/mpeg" },
  { "mpeg2aac"      , "aac"  , "audio/aac"  },
  { "mpeg4aac"      , "aac"  , "audio/aac"  },
};

int FileStreamer::codecTypeTabSize = sizeof(FileStreamer::codecTypeTab) / sizeof(FileStreamer::codec_type);

FileStreamer::file_type FileStreamer::fileTypeTab[] = {
  { "audio/flac"    , &FileStreamer::probeFLAC },
  { "audio/mpeg"    , &FileStreamer::probeMPEG },
  { "audio/aac"     , &FileStreamer::probeMPEG },
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


bool FileStreamer::HandleRequest(void* handle, const char* uri)
{
  std::string requrl(uri);
  ResourceList::const_iterator it = m_resources.begin();
  while (!IsAborted() && it != m_resources.end())
  {
    if (requrl.compare(0, (*it)->uri.length(), (*it)->uri) == 0)
    {
      std::vector<std::string> params;
      readParameters(requrl, params);
      std::string filePath = getParamValue(params, FILESTREAMER_PARAM_PATH);
      if (probe(filePath, (*it)->contentType))
        streamFile(handle, filePath, (*it)->contentType);
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

void FileStreamer::streamFile(void * handle, const std::string& filePath, const std::string& mimeType)
{
  assert(!mimeType.empty());
  int id = m_playbackCount.Add(1);
  FILE * file = nullptr;

  if (m_playbackCount.Load() > FILESTREAMER_MAX_PB)
    Reply429(handle);
  if (!(file = fopen(filePath.c_str(), "rb")))
  {
    DBG(DBG_WARN, "%s: opening file failed (%s)\n", __FUNCTION__, filePath.c_str());
    Reply500(handle);
  }
  else
  {
    DBG(DBG_INFO, "%s: open stream #%d (%s) type (%s)\n", __FUNCTION__, id, filePath.c_str(), mimeType.c_str());
    unsigned tb = 0; // count transfered bytes
    std::string resp;
    resp.assign("HTTP/1.1 200 OK\r\n")
        .append("Content-type: ").append(mimeType).append("\r\n")
        .append("Transfer-encoding: chunked\r\n")
        .append("Server: ").append(SERVER_PRODUCT_NAME).append("\r\n")
        .append("Connection: close\r\n")
        .append("\r\n");

    if (Reply(handle, resp.c_str(), resp.length()))
    {
      char buf[4008];
      int r = 0;
      while (!IsAborted() && (r = fread(buf + 5, 1, 4000, file)) > 0)
      {
        char str[6];
        snprintf(str, sizeof(str), "%03x\r\n", (unsigned)r & 0xfff);
        memcpy(buf, str, 5);
        memcpy(buf + 5 + r, "\r\n", 2);
        if (!Reply(handle, buf, r + 7))
          break;
        tb += r;
      }
      if (r == 0)
        Reply(handle, "0\r\n\r\n", 5);
    }
    DBG(DBG_INFO, "%s: close stream #%d length (%u)\n", __FUNCTION__, id, tb);
    fclose(file);
  }

  m_playbackCount.Sub(1);
}

void FileStreamer::Reply500(void* handle)
{
  std::string resp;
  resp.assign("HTTP/1.1 500 Internal Server Error\r\n")
      .append("Server: ").append(SERVER_PRODUCT_NAME).append("\r\n")
      .append("Connection: close\r\n")
      .append("\r\n");
  Reply(handle, resp.c_str(), resp.length());
}

void FileStreamer::Reply400(void* handle)
{
  std::string resp;
  resp.append("HTTP/1.1 400 Bad Request\r\n")
      .append("Server: ").append(SERVER_PRODUCT_NAME).append("\r\n")
      .append("Connection: close\r\n")
      .append("\r\n");
  Reply(handle, resp.c_str(), resp.length());
}

void FileStreamer::Reply429(void* handle)
{
  std::string resp;
  resp.append("HTTP/1.1 429 Too Many Requests\r\n")
      .append("Server: ").append(SERVER_PRODUCT_NAME).append("\r\n")
      .append("Connection: close\r\n")
      .append("\r\n");
  Reply(handle, resp.c_str(), resp.length());
}
