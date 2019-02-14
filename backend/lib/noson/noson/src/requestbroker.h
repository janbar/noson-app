#ifndef REQUESTBROKER_H
#define REQUESTBROKER_H

#include "local_config.h"
#include "sharedptr.h"

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
      std::string uri;
      std::string title;
      std::string contentType;
      std::string description;
      std::string iconUri;
      std::string sourceUrl;
      const char * data;
      unsigned int dataSize;
      Resource();
    };
    typedef SHARED_PTR<Resource> ResourcePtr;
    typedef std::list<ResourcePtr> ResourceList;

    virtual ResourcePtr GetResource(const std::string& title) = 0;
    virtual ResourceList GetResourceList() = 0;

    virtual ResourcePtr RegisterResource(const std::string& sourceUrl) = 0;
    virtual void UnregisterResource(const std::string& uri) = 0;

    void Abort() { m_aborted = true; }
    bool IsAborted() { return m_aborted; }

  protected:
    bool Reply(void* handle, const char* data, size_t size);

  private:
    bool m_aborted;
  };

}

#endif // REQUESTBROKER_H
