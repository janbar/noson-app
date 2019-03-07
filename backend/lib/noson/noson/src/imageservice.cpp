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
#include "imageservice.h"
#include "private/debug.h"
#include "data/datareader.h"
#include "filepicreader.h"
#include "private/urlencoder.h"

#include <map>
#include <cstring>

/* Important: It MUST match with the static declaration from datareader.cpp */
#define IMAGESERVICE_FAVICON  "/favicon.ico"
#define IMAGESERVICE_IMAGES   "/images/"
#define RESOURCE_FILEPICTURE  "filePicture"

using namespace NSROOT;

ImageService::ImageService()
: SONOS::RequestBroker()
, m_resources()
{
  // initialize the static resource for favicon
  {
    ResourcePtr ptr(new Resource());
    ptr->uri = IMAGESERVICE_FAVICON;
    ptr->title = "favicon";
    ptr->sourcePath = IMAGESERVICE_FAVICON;
    ptr->delegate = DataReader::Instance();
    m_resources.insert(std::make_pair(ptr->uri, ptr));
  }

  // register the picture extractor for local media file
  RegisterResource(RESOURCE_FILEPICTURE, "The file picture reader", "/file", FilePicReader::Instance());
}

bool ImageService::HandleRequest(void* handle, const char* uri)
{
  if (!IsAborted())
  {
    std::string requrl(uri);
    if (requrl.compare(0, strlen(IMAGESERVICE_IMAGES), IMAGESERVICE_IMAGES) == 0 ||
            requrl.compare(0, strlen(IMAGESERVICE_FAVICON), IMAGESERVICE_FAVICON) == 0)
    {
      ReplyContent(handle, uri);
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

RequestBroker::ResourcePtr ImageService::RegisterResource(const std::string& title,
                                                          const std::string& description,
                                                          const std::string& path,
                                                          StreamReader * delegate)
{
  ResourcePtr ptr(new Resource());
  ptr->title = title;
  ptr->description = description;
  ptr->sourcePath = path;
  ptr->delegate = delegate;
  ptr->uri = RequestBroker::buildUri(IMAGESERVICE_IMAGES, path);
  m_resources.insert(std::make_pair(ptr->uri, ptr));
  return ptr;
}

void ImageService::UnregisterResource(const std::string& uri)
{
  (void)uri;
}

std::string ImageService::MakeFilePictureURI(const std::string& filePath)
{
  std::string pictureUri;
  // find the resource for extracting picture
  ResourcePtr res = GetResource(RESOURCE_FILEPICTURE);
  if (!res)
    return pictureUri;
  // encode the file path
  std::string pathParm(urlencode(filePath));
  // make the picture uri
  if (res->uri.find('?') != std::string::npos)
    pictureUri.assign(res->uri).append("&path=").append(pathParm).append("&type=3");
  else
    pictureUri.assign(res->uri).append("?path=").append(pathParm).append("&type=3");

  return pictureUri;
}

void ImageService::ReplyContent(void * handle, const std::string& uri)
{
  // extract the resource uri without trailing args
  std::string resUri = uri.substr(0, uri.find('?'));
  ResourceMap::const_iterator it = m_resources.find(resUri);
  if (it == m_resources.end())
    Reply400(handle);
  else if (!it->second || !it->second->delegate)
    Reply500(handle);
  else
  {
    const RequestBroker::ResourcePtr& res = it->second;
    StreamReader::STREAM * stream = res->delegate->OpenStream(RequestBroker::buildDelegateUrl(*res, uri));
    if (stream && stream->contentLength)
    {
      // override content type with stream type
      const char * contentType = stream->contentType != nullptr ? stream->contentType : res->contentType.c_str();
      std::string resp;
      resp.assign("HTTP/1.1 200 OK\r\n")
          .append("Content-type: ").append(contentType).append("\r\n")
          .append("Content-length: ").append(std::to_string(stream->contentLength)).append("\r\n")
          .append("Server: Linux UPnP/1.0 Noson/1.0 (ACR_noson)\r\n")
          .append("Connection: close\r\n")
          .append("\r\n");
      if (Reply(handle, resp.c_str(), resp.length()))
      {
        while (res->delegate->ReadStream(stream) > 0)
          Reply(handle, stream->data, stream->size);
      }
      res->delegate->CloseStream(stream);
    }
    else if (stream)
    {
      Reply404(handle);
    }
    else
    {
      Reply500(handle);
    }
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

void ImageService::Reply404(void* handle)
{
  std::string resp;
  resp.append("HTTP/1.1 404 Not Found\r\n")
      .append("Server: Linux UPnP/1.0 Noson/1.0 (ACR_noson)\r\n")
      .append("Connection: close\r\n")
      .append("\r\n");
  Reply(handle, resp.c_str(), resp.length());
}
