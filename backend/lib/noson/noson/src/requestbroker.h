/*
 *      Copyright (C) 2019 Jean-Luc Barriere
 *
 *  This file is part of Noson
 *
 *  Noson is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Noson is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef REQUESTBROKER_H
#define REQUESTBROKER_H

#include "local_config.h"
#include "sharedptr.h"
#include "streamreader.h"

#include <cstddef>
#include <string>
#include <list>

namespace NSROOT
{
  class EventHandlerThread;
  class TcpSocket;
  class WSRequestBroker;
  class RequestBroker;

  typedef SHARED_PTR<RequestBroker> RequestBrokerPtr;

  class RequestBroker
  {
  public:
    RequestBroker();
    virtual ~RequestBroker();

    struct opaque;

    struct handle
    {
      EventHandlerThread * handler;
      opaque * payload;
    };

    /**
     * @brief Handle an incoming request
     * @param handle the data to pass for callback
     * @param uri the URI of the requested resource
     * @return true if the request is processed, else false
     */
    virtual bool HandleRequest(handle * handle) = 0;

    /**
     * @brief Return the common name for this handler
     * @return the common name
     */
    virtual const char * CommonName() = 0;

    struct Resource {
      std::string uri;          ///< Absolute URI of the registered resource
      std::string title;        ///< A friendly name for the resource
      std::string description;  ///< Description of the resource
      std::string contentType;  ///< Default mime type
      std::string iconUri;      ///< URI of the icon
      std::string sourcePath;   ///< Path of the resource for delegate
      StreamReader * delegate;  ///< Pointer to delegate
      Resource();
    };
    typedef SHARED_PTR<Resource> ResourcePtr;
    typedef std::list<ResourcePtr> ResourceList;

    virtual ResourcePtr GetResource(const std::string& title) = 0;
    virtual ResourceList GetResourceList() = 0;
    virtual ResourcePtr RegisterResource(const std::string& title,
                                         const std::string& description,
                                         const std::string& path,
                                         StreamReader * delegate) = 0;
    virtual void UnregisterResource(const std::string& uri) = 0;

    void Abort() { m_aborted = true; }
    bool IsAborted() { return m_aborted; }
    unsigned Count200() { return static_cast<unsigned int>(m_200.GetValue()); }
    unsigned Count400() { return static_cast<unsigned int>(m_400.GetValue()); }
    unsigned Count404() { return static_cast<unsigned int>(m_404.GetValue()); }
    unsigned Count429() { return static_cast<unsigned int>(m_429.GetValue()); }
    unsigned Count500() { return static_cast<unsigned int>(m_500.GetValue()); }
    unsigned Count503() { return static_cast<unsigned int>(m_503.GetValue()); }

  protected:

    enum Status
    {
      Status_OK,
      Status_Partial_Content,
      Status_Bad_Request,
      Status_Not_Found,
      Status_Too_Many_Requests,
      Status_Internal_Server_Error,
      Status_Service_Unavailable,
    };

    /**
     * @brief Create the top header lines for the reply
     * @param status
     * @return The response header for the given status code
     */
    std::string MakeResponseHeader(Status status);

    /**
     * @brief Reply to the requester
     * @param handle
     * @param data the buffer containing bytes to send
     * @param size the size of data
     * @return true is all data have been sent, else false
     */
    static bool Reply(handle * handle, const char * data, size_t size);

    enum Method
    {
      Method_GET,
      Method_POST,
      Method_HEAD,
      Method_SUBSCRIBE,
      Method_UNSUBSCRIBE,
      Method_NOTIFY,
      Method_UNKNOWN,
    };

    /**
     * @param handle
     * @return The method invoked by the request
     */
    static Method GetRequestMethod(handle * handle);

    /**
     * @param handle
     * @return The URI intended by the request
     */
    static const std::string& GetRequestURI(handle * handle);

    /**
     * @param handle
     * @return The protocol of the request, i.e HTTP/1.1
     */
    static const std::string& GetRequestProtocol(handle * handle);

    /**
     * @param handle
     * @return The value of the requet header with the given name
     */
    static const std::string& GetRequestHeader(handle * handle, const std::string& name);

    /**
     * @param handle
     * @return true is the request has a content, else false
     */
    static bool HasContent(handle * handle);

    /**
     * @param handle
     * @return The size in byte of the content of the request
     */
    static size_t GetContentLength(handle * handle);

    /**
     * @param handle
     * @return The size in byte read from the content
     */
    static size_t GetConsumed(handle * handle);

    /**
     * @brief Read data from the request content
     * @param handle
     * @param data The string buffer to append read data
     * @return The size in byte read from the content
     */
    static size_t ReadContent(handle * handle, std::string& data);

    /**
     * @brief Helper to build the delegate url from a call uri
     * @param res the delegated resource
     * @param uri the call uri
     * @return the valid url to pass to the delegate
     */
    static std::string buildDelegateUrl(const Resource& res, const std::string& uri);

    /**
     * @brief Helper to build uri from the given root and path
     * @param rootUri the root uri
     * @param path the path to add
     * @return the uri
     */
    static std::string buildUri(const std::string& rootUri, const std::string& path);

  private:
    bool m_aborted;
    IntrinsicCounter m_200;
    IntrinsicCounter m_400;
    IntrinsicCounter m_404;
    IntrinsicCounter m_429;
    IntrinsicCounter m_500;
    IntrinsicCounter m_503;
  };

}

#endif // REQUESTBROKER_H
