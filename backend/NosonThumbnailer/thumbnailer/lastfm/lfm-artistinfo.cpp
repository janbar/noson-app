/*
 *      Copyright (C) 2018-2019 Jean-Luc Barriere
 *
 *  This file is part of Noson-App
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
 *  along with Noson.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "lastfm.h"
#include "../tinyxml2.h"
#include "../xmldict.h"

#include <QUrl>
#include <QDebug>
#include <QUrlQuery>

#define BASE_URL "https://ws.audioscrobbler.com/2.0/"

using namespace thumbnailer;

LFMArtistInfo::LFMArtistInfo(const QString& apiKey, const QString& artist)
: AbstractArtistInfo(artist)
, m_apiKey(apiKey)
{
}

void LFMArtistInfo::queryInfo(NetRequest* prepared)
{
  QUrlQuery post;
  post.addQueryItem("api_key", m_apiKey);
  QString postData;
  postData.append(post.toString());

  QUrlQuery qry;
  qry.addQueryItem("method","artist.getinfo");
  qry.addQueryItem("artist", AbstractAPI::normalizeArtist(m_artist));
  QUrl url;
  url.setUrl(BASE_URL);
  url.setQuery(qry);

  prepared->setOperation(QNetworkAccessManager::PostOperation);
  prepared->setHeader(QNetworkRequest::UserAgentHeader, THUMBNAILER_USER_AGENT);
  prepared->setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
  prepared->setHeader("Accept", "text/xml");
  prepared->setHeader("Accept-Charset", "utf-8");
  prepared->setData(postData.toUtf8());
  prepared->setUrl(QUrl(url));
}

AbstractAPI::Parse_Status LFMArtistInfo::parseInfo(const QByteArray& info, AbstractArtistInfo::metadata_t& meta)
{
  if (info.isEmpty())
    return AbstractAPI::Parse_Failed;
  // Parse xml content
  tinyxml2::XMLDocument rootdoc;
  if (rootdoc.Parse(info.constData(), info.length()) != tinyxml2::XML_SUCCESS)
  {
    qDebug().noquote() << info;
    return AbstractAPI::Parse_Failed;
  }

  const tinyxml2::XMLElement* elem; // an element
  if (!(elem = rootdoc.RootElement()))
  {
    tinyxml2::XMLPrinter out;
    rootdoc.Accept(&out);
    qDebug().noquote() << out.CStr();
    return AbstractAPI::Parse_Failed;
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
          meta.name = felem->GetText();
        else if (XMLNS::NameEqual(felem->Name(), "mbid") && felem->GetText())
          meta.mbid = felem->GetText();
        else if (XMLNS::NameEqual(felem->Name(), "url") && felem->GetText())
          meta.url = felem->GetText();
        else if (XMLNS::NameEqual(felem->Name(), "image") && felem->GetText())
        {
          if (felem->Attribute("size", "small"))
            meta.image_small = felem->GetText();
          else if (felem->Attribute("size", "medium"))
            meta.image_medium = felem->GetText();
          else if (felem->Attribute("size", "large"))
            meta.image_large = felem->GetText();
          else if (felem->Attribute("size", "extralarge"))
            meta.image_extralarge = felem->GetText();
        }
        felem = felem->NextSiblingElement(nullptr);
      }
      return AbstractAPI::Parse_Succeeded;
    }
  }
  return AbstractAPI::Parse_Failed;
}

bool LFMArtistInfo::parseServerError(int statusCode, const QByteArray& info, AbstractAPI::error_t& error)
{
  return LastfmAPI::parseServerError(statusCode, info, error);
}
