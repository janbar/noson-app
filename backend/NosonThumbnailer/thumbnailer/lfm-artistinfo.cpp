/*
 *      Copyright (C) 2018 Jean-Luc Barriere
 *
 *  This file is part of Noson-App
 *
 *  Noson-App is free software: you can redistribute it and/or modify
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
 *  along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "lfm-artistinfo.h"
#include "netrequest.h"
#include "xmldict.h"
#include "diskcachemanager.h"

#include <QUrl>
#include <QDebug>
#include <QUrlQuery>

#define BASE_URL "https://ws.audioscrobbler.com/2.0/"
#define IMAGE_SIZE_SMALL      1
#define IMAGE_SIZE_MEDIUM     2
#define IMAGE_SIZE_LARGE      3
#define IMAGE_SIZE_EXTRALARGE 4
#define DEFAULT_SIZE IMAGE_SIZE_EXTRALARGE
#define EXPIRE_DAYS_SUCCEEDED 360
#define EXPIRE_DAYS_FAILED    8
#define ERRMSG_NOT_FOUND      "No image found"

using namespace thumbnailer;

ArtistInfo::ArtistInfo(DiskCacheManager* cache, NetManager* nam, const QString& apiKey, const QString& artist, const QSize& requestedSize, bool cached, QObject* parent)
: AbstractWorker(parent)
, m_cache(cache)
, m_nam(nam)
, m_apiKey(apiKey)
, m_artist(artist)
, m_requestedSize(requestedSize)
, m_cached(cached)
, m_size(0)
, m_cacheDev(nullptr)
, m_call(nullptr)
, m_error(ReplySuccess)
, m_errorCode(0)
{
  if (requestedSize.width() <= 0 && requestedSize.height() <= 0)
    m_requestedSize = QSize(DEFAULT_SIZE, DEFAULT_SIZE);

  if (m_requestedSize.width() <= 34 && m_requestedSize.height() <=  34)
    m_size = IMAGE_SIZE_SMALL;
  else if (m_requestedSize.width() <= 64 && m_requestedSize.height() <=  64)
    m_size = IMAGE_SIZE_MEDIUM;
  else if (m_requestedSize.width() <= 174 && m_requestedSize.height() <=  174)
    m_size = IMAGE_SIZE_LARGE;
  else
    m_size = IMAGE_SIZE_EXTRALARGE;

  QString str("image://artistinfo/?");
  QUrlQuery qry;
  qry.addQueryItem("artist", m_artist);
  qry.addQueryItem("size", QString::number(m_size));
  str.append(qry.toString());
  m_cacheUrl.setUrl(str);
}

ArtistInfo::~ArtistInfo()
{
  if (m_cacheDev)
  {
    m_cacheDev->close();
    delete m_cacheDev;
  }
}

void ArtistInfo::run()
{
  m_cacheDev = m_cache->queryData(m_cacheUrl, m_cached);
  if (m_cacheDev)
  {
    m_image = m_cacheDev->readAll();
    if (m_image.size() == 0)
    {
      m_error = ReplyServerError;
      m_errorCode = 0;
      m_errorString = ERRMSG_NOT_FOUND;
    }
    emit finished();
    return;
  }
  // using cache only
  if (m_cached)
  {
    m_error = ReplyServerError;
    m_errorCode = 0;
    m_errorString = ERRMSG_NOT_FOUND;
    emit finished();
    return;
  }
  queryInfo();
}

ReplyStatus ArtistInfo::error()
{
  return m_error;
}

int ArtistInfo::errorCode()
{
  return m_errorCode;
}

QString ArtistInfo::errorString()
{
  return m_errorString;
}

const QByteArray& ArtistInfo::image()
{
  return m_image;
}

void ArtistInfo::readInfo()
{
  m_info.append(m_call->readData());
}

void ArtistInfo::processInfo()
{
  if (m_call->error())
  {
    // network failure
    if (m_call->errorCode() < 200)
    {
      m_error = ReplyNetworkError;
      m_errorCode = m_call->errorCode();
      m_errorString = m_call->errorString();
      emit finished();
      return;
    }

    if (!m_call->atEnd())
      readInfo();

    if (!parseServerError())
    {
       m_error = ReplyServerError;
       m_errorCode = m_call->errorCode();
       m_errorString = m_call->errorString();
       emit finished();
       return;
    }
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
    switch(m_errorCode)
    {
      case 6:
      case 11:
      case 16:
        m_error = ReplyServerError;
        fakeImage();
        break;
      default:
        m_error = ReplyFatalError;
        break;
    }
    emit finished();
    return;
  }

  if (!m_call->atEnd())
    readInfo();

  if (m_info.isEmpty() || !parseInfo())
  {
    m_error = ReplyInvalid;
    m_errorCode = 0;
    m_errorString = "Invalid response";
    emit finished();
    return;
  }

  if (!m_meta.image_small.isEmpty() && m_size < 2)
    queryImage(QUrl(m_meta.image_small));
  else if (!m_meta.image_medium.isEmpty() && m_size < 3)
    queryImage(m_meta.image_medium);
  else if (!m_meta.image_large.isEmpty() && m_size < 4)
    queryImage(m_meta.image_large);
  else if (!m_meta.image_extralarge.isEmpty())
    queryImage(m_meta.image_extralarge);
  else
  {
    m_error = ReplyServerError;
    m_errorCode = 0;
    m_errorString = ERRMSG_NOT_FOUND;
    fakeImage();
    emit finished();
  }
}

void ArtistInfo::queryInfo()
{
  QUrlQuery post;
  post.addQueryItem("api_key", m_apiKey);
  QByteArray postData;
  postData.append(post.toString());

  QUrlQuery qry;
  qry.addQueryItem("method","artist.getinfo");
  qry.addQueryItem("artist", m_artist);
  QUrl url;
  url.setUrl(BASE_URL);
  url.setQuery(qry);

  m_call.reset(new NetRequest(m_nam));
  //connect(m_call.get(), SIGNAL(readyRead(NetRequest*)), this, SLOT(readInfo()));
  connect(m_call.get(), SIGNAL(finished(NetRequest*)), this, SLOT(processInfo()));
  m_call->setOperation(QNetworkAccessManager::PostOperation);
  m_call->setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
  m_call->setData(postData);
  m_call->startRequest(QUrl(url));
}

bool ArtistInfo::parseInfo()
{
  // Parse xml content
  tinyxml2::XMLDocument rootdoc;
  if (rootdoc.Parse(m_info.constData(), m_info.length()) != tinyxml2::XML_SUCCESS)
  {
    qDebug().noquote() << m_info;
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

  if (XMLNS::NameEqual(elem->Name(), "lfm") && elem->Attribute("status", "ok"))
  {
    elem = elem->FirstChildElement(nullptr);
    if (elem && XMLNS::NameEqual(elem->Name(), "artist"))
    {
      const tinyxml2::XMLElement* felem = elem->FirstChildElement(nullptr);
      while (felem)
      {
        if (XMLNS::NameEqual(felem->Name(), "name") && felem->GetText())
          m_meta.name = felem->GetText();
        else if (XMLNS::NameEqual(felem->Name(), "mbid") && felem->GetText())
          m_meta.mbid = felem->GetText();
        else if (XMLNS::NameEqual(felem->Name(), "url") && felem->GetText())
          m_meta.url = felem->GetText();
        else if (XMLNS::NameEqual(felem->Name(), "image") && felem->GetText())
        {
          if (felem->Attribute("size", "small"))
            m_meta.image_small = felem->GetText();
          else if (felem->Attribute("size", "medium"))
            m_meta.image_medium = felem->GetText();
          else if (felem->Attribute("size", "large"))
            m_meta.image_large = felem->GetText();
          else if (felem->Attribute("size", "extralarge"))
            m_meta.image_extralarge = felem->GetText();
        }
        felem = felem->NextSiblingElement(nullptr);
      }
      return true;
    }
  }
  return false;
}

void ArtistInfo::queryImage(const QUrl& url)
{
  m_call.reset(new NetRequest(m_nam));
  //connect(m_call.get(), SIGNAL(readyRead(NetRequest*)), this, SLOT(readImage()));
  connect(m_call.get(), SIGNAL(finished(NetRequest*)), this, SLOT(processImage()));
  m_call->startRequest(url);
}

bool ArtistInfo::parseServerError()
{
  // Parse xml content
  tinyxml2::XMLDocument rootdoc;
  if (rootdoc.Parse(m_info.constData(), m_info.length()) != tinyxml2::XML_SUCCESS)
  {
    qDebug().noquote() << m_info;
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
      m_errorCode = elem->IntAttribute("code");
      if (elem->GetText())
        m_errorString = elem->GetText();
      return true;
    }
  }
  return false;
}

void ArtistInfo::fakeImage()
{
  // insert cache fake
  int days = EXPIRE_DAYS_FAILED - (EXPIRE_DAYS_FAILED/2) + (std::rand() % EXPIRE_DAYS_FAILED);
  QIODevice* cacheDev = m_cache->createData(m_cacheUrl, QDateTime::currentDateTime().addDays(days));
  m_cache->insertData(cacheDev);
}

void ArtistInfo::readImage()
{
  m_image.append(m_call->readData());
}

void ArtistInfo::processImage()
{
  if (m_call->error())
  {
    // network failure
    if (m_call->errorCode() < 200)
    {
      m_error = ReplyNetworkError;
      m_errorCode = m_call->errorCode();
      m_errorString = m_call->errorString();
      emit finished();
      return;
    }
    // server failure
    m_error = ReplyServerError;
    m_errorCode = m_call->errorCode();
    m_errorString = m_call->errorString();
    fakeImage();
    emit finished();
    return;
  }
  if (!m_call->atEnd())
    readImage();
  // insert cache entry
  int days = EXPIRE_DAYS_SUCCEEDED - (EXPIRE_DAYS_SUCCEEDED/2) + (std::rand() % EXPIRE_DAYS_SUCCEEDED);
  QIODevice* cacheDev = m_cache->createData(m_cacheUrl, QDateTime::currentDateTime().addDays(days));
  cacheDev->write(m_image);
  m_cache->insertData(cacheDev);
  emit finished();
}
