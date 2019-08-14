
#include "lastfm.h"
#include "../tinyxml2.h"
#include "../xmldict.h"

using namespace thumbnailer;

namespace
{
  // register the API
  bool dummy = AbstractAPI::registerMe(new LastfmAPI());
}

bool LastfmAPI::configure(NetManager* nam, const QString& apiKey)
{
  (void)nam; // no check done for the key
  m_apikey = apiKey;
  return !apiKey.isEmpty();
}

AbstractArtistInfo* LastfmAPI::newArtistInfo(const QString& artist)
{
  return new LFMArtistInfo(m_apikey, artist);
}

AbstractAlbumInfo* LastfmAPI::newAlbumInfo(const QString& artist, const QString& album)
{
  return new LFMAlbumInfo(m_apikey, artist, album);
}

bool LastfmAPI::parseServerError(int statusCode, const QByteArray& info, AbstractAPI::error_t& error)
{
  /*
  2 : Invalid service - This service does not exist
  3 : Invalid Method - No method with that name in this package
  4 : Authentication Failed - You do not have permissions to access the service
  5 : Invalid format - This service doesn't exist in that format
  6 : Invalid parameters - Your request is missing a required parameter
  7 : Invalid resource specified
  8 : Operation failed - Something else went wrong
  9 : Invalid session key - Please re-authenticate
  10 : Invalid API key - You must be granted a valid key by last.fm
  11 : Service Offline - This service is temporarily offline. Try again later.
  13 : Invalid method signature supplied
  16 : There was a temporary error processing your request. Please try again
  26 : Suspended API key - Access for your account has been suspended, please contact Last.fm
  29 : Rate limit exceeded - Your IP has made too many requests in a short period
  */
  (void)statusCode;

  if (info.isEmpty())
    return false;
  // Parse xml content
  tinyxml2::XMLDocument rootdoc;
  if (rootdoc.Parse(info.constData(), info.length()) != tinyxml2::XML_SUCCESS)
  {
    qDebug().noquote() << info;
    return false;
  }

  const tinyxml2::XMLElement* elem; // an element
  if (!(elem = rootdoc.RootElement()))
  {
    tinyxml2::XMLPrinter out;
    rootdoc.Accept(&out);
    qDebug().noquote() << out.CStr();
    return false;
  }

  if (XMLNS::NameEqual(elem->Name(), "lfm") && elem->Attribute("status", "failed"))
  {
    elem = elem->FirstChildElement(nullptr);
    if (elem && XMLNS::NameEqual(elem->Name(), "error"))
    {
      error.errorCode = elem->IntAttribute("code");
      if (elem->GetText())
        error.errorString = elem->GetText();
      switch (error.errorCode)
      {
      case 6:
      case 11:
      case 16:
        error.status = ReplyServerError;
        break;
      case 29:
        error.status = ReplyQuotaExceeded;
        break;
      default:
        error.status = ReplyFatalError;
        break;
      }
      return true;
    }
  }
  return false;
}
