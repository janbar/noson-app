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
  class RequestBroker;

  typedef SHARED_PTR<RequestBroker> RequestBrokerPtr;

  class RequestBroker
  {
  public:
    RequestBroker();
    virtual ~RequestBroker();

    /**
     * @brief Handle an incoming request
     * @param handle the data to pass for callback
     * @param uri the URI of the requested resource
     * @return true if the request is processed, else false
     */
    virtual bool HandleRequest(void* handle, const char* uri) = 0;

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

  protected:
    bool Reply(void* handle, const char* data, size_t size);

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
  };

}

#endif // REQUESTBROKER_H
