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
#include <map>

#include "imageservice.h"
#include "private/debug.h"
#include "data/favicon_ico.h"
#ifdef HAVE_PULSEAUDIO
#include "data/pulseaudio_png.h"
#endif

#include <cstring>

#define IMAGESERVICE_FAVICON  "/favicon.ico"
#define IMAGESERVICE_IMAGES   "/images/"

using namespace NSROOT;

ImageService::ImageService()
: SONOS::RequestBroker()
, m_resources()
{
  LoadInternalResources(m_resources);
}

bool ImageService::HandleRequest(void* handle, const char* uri)
{
  if (!IsAborted())
  {
    std::string requrl(uri);
    if (requrl.compare(0, strlen(IMAGESERVICE_IMAGES), IMAGESERVICE_IMAGES) == 0 ||
            requrl.compare(0, strlen(IMAGESERVICE_FAVICON), IMAGESERVICE_FAVICON) == 0)
    {
      ReplyContent(handle, requrl.substr(0, requrl.find('?')));
      return true;
    }
  }
  return false;
}

RequestBroker::ResourcePtr ImageService::GetResource(const std::string& title)
{
  for (ResourceMap::iterator it = m_resources.begin(); it != m_resources.end(); ++it)
  {
    if (it->second->title == title)
      return it->second;
  }
  return ResourcePtr();
}

RequestBroker::ResourceList ImageService::GetResourceList()
{
  ResourceList list;
  for (ResourceMap::iterator it = m_resources.begin(); it != m_resources.end(); ++it)
    list.push_back(it->second);
  return list;
}

RequestBroker::ResourcePtr ImageService::RegisterResource(const std::string& sourceUrl)
{
  (void)sourceUrl;
  return ResourcePtr();
}

void ImageService::UnregisterResource(const std::string& uri)
{
  (void)uri;
}

void ImageService::ReplyContent(void * handle, const std::string& uri)
{
  ResourceMap::const_iterator it = m_resources.find(uri);
  if (it == m_resources.end())
    Reply400(handle);
  else if (!it->second || !it->second->data)
    Reply500(handle);
  else
  {
    ResourcePtr res = it->second;
    std::string resp;
    resp.assign("HTTP/1.1 200 OK\r\n")
        .append("Content-type: ").append(res->contentType).append("\r\n")
        .append("Content-length: ").append(std::to_string(res->dataSize)).append("\r\n")
        .append("Server: Linux UPnP/1.0 Noson/1.0 (ACR_noson)\r\n")
        .append("Connection: close\r\n")
        .append("\r\n");
    if (Reply(handle, resp.c_str(), resp.length()))
      Reply(handle, res->data, res->dataSize);
  }
}

void ImageService::Reply500(void* handle)
{
  std::string resp;
  resp.assign("HTTP/1.1 500 Internal Server Error\r\n")
      .append("Server: Linux UPnP/1.0 Noson/1.0 (ACR_noson)\r\n")
      .append("Connection: close\r\n")
      .append("\r\n");
  Reply(handle, resp.c_str(), resp.length());
}

void ImageService::Reply400(void* handle)
{
  std::string resp;
  resp.append("HTTP/1.1 400 Bad Request\r\n")
      .append("Server: Linux UPnP/1.0 Noson/1.0 (ACR_noson)\r\n")
      .append("Connection: close\r\n")
      .append("\r\n");
  Reply(handle, resp.c_str(), resp.length());
}

void ImageService::LoadInternalResources(ResourceMap& map)
{
  {
    ResourcePtr ptr(new Resource());
    ptr->title = "favicon";
    ptr->uri = IMAGESERVICE_FAVICON;
    ptr->contentType = "image/x-icon";
    ptr->data = (char*)favicon_ico;
    ptr->dataSize = favicon_ico_len;
    map.insert(std::make_pair(ptr->uri, ptr));
  }
#ifdef HAVE_PULSEAUDIO
  {
    ResourcePtr ptr(new Resource());
    ptr->title = "pulseaudio icon";
    ptr->uri = IMAGESERVICE_IMAGES "pulseaudio.png";
    ptr->contentType = "image/png";
    ptr->data = (char*)pulseaudio_png;
    ptr->dataSize = pulseaudio_png_len;
    map.insert(std::make_pair(ptr->uri, ptr));
  }
#endif
}
