#include "requestbroker.h"

#include "private/socket.h"

using namespace NSROOT;

RequestBroker::RequestBroker()
: m_aborted(false)
{
}

RequestBroker::~RequestBroker() { }

bool RequestBroker::Reply(void *handle, const char *data, size_t size)
{
  TcpSocket* socket = static_cast<TcpSocket*>(handle);
  if (socket)
    return socket->SendData(data, size);
  return false;
}

std::string RequestBroker::buildDelegateUrl(const RequestBroker::Resource& res, const std::string& uri)
{
  size_t a = uri.find('?');
  if (a != std::string::npos)
    return res.sourcePath + uri.substr(a);
  return res.sourcePath;
}

std::string RequestBroker::buildUri(const std::string &rootUri, const std::string &path)
{
  unsigned rpath = 0;
  while (rpath < path.length() && path.at(rpath) == '/') ++rpath;
  return std::string(rootUri).append(path.substr(rpath));
}

RequestBroker::Resource::Resource()
: uri()
, title()
, description()
, contentType("application/octet-stream")
, iconUri()
, sourcePath()
, delegate(nullptr)
{
}
