/*
 *      Copyright (C) 2026
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

#include "cloudmediamodel.h"
#include "sonosaccount.h"

#include <noson/digitalitem.h>
#include <noson/element.h>

#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrl>
#include <QUrlQuery>

Q_DECLARE_METATYPE(SONOS::DigitalItemPtr)

using namespace nosonapp;

static const char* CONTENT_BASE = "https://play.sonos.com/api/content";
static const char* BROWSER_UA   = "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 "
                                  "(KHTML, like Gecko) Chrome/140.0 Safari/537.36";
static const char* RINC_NS      = "urn:schemas-rinconnetworks-com:metadata-1-0/";

// YouTube Music's global service identity (same for every household/user).
static const char* SERVICE_YTM = "72711"; // service type
static const char* SID_YTM     = "284";   // service id = (72711-7)/256
// Household + account are discovered at runtime (never hardcoded): household
// from the local system (Muse household id), account from the household's
// /integrations/registrations listing.

CloudMediaModel::CloudMediaModel(QObject* parent)
: QAbstractListModel(parent)
, m_account(nullptr)
, m_busy(false)
, m_service(QString::fromUtf8(SERVICE_YTM))
, m_sid(QString::fromUtf8(SID_YTM))
{
}

CloudMediaModel::~CloudMediaModel()
{
}

void CloudMediaModel::init(SonosAccount* account, const QString& household)
{
  m_account = account;
  if (household != m_household)
  {
    m_household = household;
    m_accountId.clear(); // force re-discovery for the new household
  }
}

void CloudMediaModel::start()
{
  if (!m_account || !m_account->isReady())
  {
    emit error(tr("Not signed in"));
    return;
  }
  if (m_household.isEmpty())
  {
    emit error(tr("No Sonos household found"));
    return;
  }
  if (m_accountId.isEmpty())
    requestRegistrations(); // discovers the account, then loads the home page
  else
    loadRoot();
}

// Discover which account is linked to YouTube Music on this household.
void CloudMediaModel::requestRegistrations()
{
  QUrl url(QString::fromUtf8(CONTENT_BASE) +
           QStringLiteral("/v1/households/") + m_household +
           QStringLiteral("/integrations/registrations"));
  QNetworkRequest req(url);
  if (!prepare(req))
    return;
  setBusy(true);
  QNetworkReply* reply = m_account->nam()->get(req);
  connect(reply, &QNetworkReply::finished, this, [this, reply]() {
    reply->deleteLater();
    setBusy(false);
    if (reply->error() != QNetworkReply::NoError)
    {
      emit error(reply->errorString());
      return;
    }
    const QJsonArray arr = QJsonDocument::fromJson(reply->readAll()).array();
    for (const QJsonValue& v : arr)
    {
      const QJsonObject o = v.toObject();
      if (o.value(QStringLiteral("service-id")).toString() == m_service)
      {
        m_accountId = o.value(QStringLiteral("account-id")).toString();
        break;
      }
    }
    if (m_accountId.isEmpty())
      emit error(tr("YouTube Music is not connected to this household"));
    else
      loadRoot();
  });
}

QHash<int, QByteArray> CloudMediaModel::roleNames() const
{
  QHash<int, QByteArray> r;
  r[PayloadRole]     = "payload";
  r[TitleRole]       = "title";
  r[SubtitleRole]    = "subtitle";
  r[ArtRole]         = "art";
  r[TypeRole]        = "type";
  r[IsContainerRole] = "isContainer";
  r[CanPlayRole]     = "canPlay";
  r[ObjectIdRole]    = "objectId";
  r[HrefRole]        = "href";
  return r;
}

int CloudMediaModel::rowCount(const QModelIndex& parent) const
{
  if (parent.isValid())
    return 0;
  return m_items.count();
}

QVariant CloudMediaModel::data(const QModelIndex& index, int role) const
{
  if (!index.isValid() || index.row() >= m_items.count())
    return QVariant();
  const CloudItem& it = m_items.at(index.row());
  switch (role)
  {
  case PayloadRole:     return buildPayload(it);
  case TitleRole:       return it.title;
  case SubtitleRole:    return it.subtitle;
  case ArtRole:         return it.art;
  case TypeRole:        return it.type;
  case IsContainerRole: return it.isContainer;
  case CanPlayRole:     return it.canPlay;
  case ObjectIdRole:    return it.objectId;
  case HrefRole:        return it.href;
  default:              return QVariant();
  }
}

QVariantMap CloudMediaModel::get(int row) const
{
  QVariantMap m;
  if (row < 0 || row >= m_items.count())
    return m;
  const CloudItem& it = m_items.at(row);
  m["payload"]     = buildPayload(it);
  m["title"]       = it.title;
  m["subtitle"]    = it.subtitle;
  m["art"]         = it.art;
  m["type"]        = it.type;
  m["isContainer"] = it.isContainer;
  m["canPlay"]     = it.canPlay;
  m["objectId"]    = it.objectId;
  m["href"]        = it.href;
  return m;
}

void CloudMediaModel::clear()
{
  beginResetModel();
  m_items.clear();
  m_stack.clear();
  endResetModel();
  emit countChanged();
  emit pathChanged();
}

void CloudMediaModel::loadRoot()
{
  m_stack.clear();
  Nav n; n.mode = 1; n.id = QStringLiteral("root"); n.title = tr("YouTube Music"); n.type = TypeContainer; n.href = QString();
  m_stack.push(n);
  emit pathChanged();
  requestBrowse(QStringLiteral("root"), TypeContainer, QString());
}

void CloudMediaModel::search(const QString& term)
{
  if (term.trimmed().isEmpty())
    return;
  // Push onto the stack (keep the root/dashboard beneath) so the header's
  // up navigation returns to the service home after a search.
  Nav n; n.mode = 0; n.term = term; n.title = tr("Search: %1").arg(term); n.type = TypeContainer; n.href = QString();
  m_stack.push(n);
  emit pathChanged();
  requestSearch(term);
}

void CloudMediaModel::browse(const QString& objectId, const QString& title, int type, const QString& href)
{
  Nav n; n.mode = 1; n.id = objectId; n.title = title; n.type = type; n.href = href;
  m_stack.push(n);
  emit pathChanged();
  requestBrowse(objectId, type, href);
}

void CloudMediaModel::back()
{
  if (m_stack.count() <= 1)
    return;
  m_stack.pop();
  emit pathChanged();
  const Nav& n = m_stack.top();
  if (n.mode == 0)
    requestSearch(n.term);
  else
    requestBrowse(n.id, n.type, n.href);
}

// Fallback path segment when an item carries no href. Albums/containers browse
// fine via "containers" (verified); artists/playlists use their own segment.
QString CloudMediaModel::browseSegmentForType(int type)
{
  switch (type)
  {
  case TypePlaylist: return QStringLiteral("playlists");
  case TypeArtist:   return QStringLiteral("artists");
  default:           return QStringLiteral("containers");
  }
}

// The API's item hrefs point at api.ws.sonos.com (bearer-only); rewrite them to
// the play.sonos.com proxy so our session cookie authorizes them.
QString CloudMediaModel::swapContentHost(const QString& href)
{
  QString h = href;
  h.replace(QStringLiteral("https://api.ws.sonos.com/content/api"),
            QStringLiteral("https://play.sonos.com/api/content"));
  return h;
}

bool CloudMediaModel::prepare(QNetworkRequest& req)
{
  if (!m_account || !m_account->isReady())
  {
    emit error(tr("Not signed in"));
    return false;
  }
  // Cookies come from the shared jar in SonosAccount::nam() (which also
  // auto-updates the rotating JSESSIONID); do NOT set a manual Cookie header.
  req.setRawHeader("User-Agent", BROWSER_UA);
  req.setRawHeader("Accept", "application/json");
  req.setRawHeader("Referer", "https://play.sonos.com/");
  req.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
  return true;
}

void CloudMediaModel::requestSearch(const QString& term)
{
  QUrl url(QString::fromUtf8(CONTENT_BASE) +
           QStringLiteral("/v1/households/") + m_household + QStringLiteral("/search"));
  QUrlQuery q;
  q.addQueryItem(QStringLiteral("query"), term);
  q.addQueryItem(QStringLiteral("services"), m_service);
  q.addQueryItem(QStringLiteral("filterExplicit"), QStringLiteral("false"));
  url.setQuery(q);
  QNetworkRequest req(url);
  if (!prepare(req))
    return;
  setBusy(true);
  QNetworkReply* reply = m_account->nam()->get(req);
  connect(reply, &QNetworkReply::finished, this, [this, reply]() {
    reply->deleteLater();
    setBusy(false);
    if (reply->error() != QNetworkReply::NoError)
    {
      emit error(reply->errorString());
      emit loaded(false);
      return;
    }
    parseSearch(reply->readAll());
  });
}

void CloudMediaModel::requestBrowse(const QString& objectId, int type, const QString& href)
{
  QUrl url;
  if (!href.isEmpty())
  {
    // Authoritative: the API's own browse URL (carries filterExplicit/defaults).
    url = QUrl(swapContentHost(href));
  }
  else
  {
    url = QUrl(QString::fromUtf8(CONTENT_BASE) +
               QStringLiteral("/v2/households/") + m_household +
               QStringLiteral("/services/") + m_service +
               QStringLiteral("/accounts/") + m_accountId +
               QStringLiteral("/") + browseSegmentForType(type) + QStringLiteral("/") + objectId +
               QStringLiteral("/browse"));
    QUrlQuery q;
    q.addQueryItem(QStringLiteral("muse2"), QStringLiteral("true"));
    // Respect the household's explicit-content setting; without this the
    // server can filter a browse (e.g. an album) down to nothing.
    q.addQueryItem(QStringLiteral("filterExplicit"), QStringLiteral("false"));
    url.setQuery(q);
  }
  QNetworkRequest req(url);
  if (!prepare(req))
    return;
  setBusy(true);
  QNetworkReply* reply = m_account->nam()->get(req);
  connect(reply, &QNetworkReply::finished, this, [this, reply]() {
    reply->deleteLater();
    setBusy(false);
    if (reply->error() != QNetworkReply::NoError)
    {
      emit error(reply->errorString());
      emit loaded(false);
      return;
    }
    parseBrowse(reply->readAll());
  });
}

int CloudMediaModel::mapSearchType(const QString& t, bool& isContainer, bool& canPlay)
{
  const QString u = t.toUpper();
  if (u == QLatin1String("TRACK") || u == QLatin1String("SONG"))
  { isContainer = false; canPlay = true; return TypeTrack; }
  if (u == QLatin1String("ALBUM"))
  { isContainer = true; canPlay = true; return TypeAlbum; }
  if (u == QLatin1String("PLAYLIST"))
  { isContainer = true; canPlay = true; return TypePlaylist; }
  if (u == QLatin1String("ARTIST"))
  { isContainer = true; canPlay = false; return TypeArtist; }
  isContainer = true; canPlay = false; return TypeContainer;
}

int CloudMediaModel::mapBrowseType(const QString& t, bool& isContainer, bool& canPlay)
{
  QString u = t.toUpper();
  if (u.startsWith(QLatin1String("ITEM_")))
    u = u.mid(5);
  return mapSearchType(u, isContainer, canPlay);
}

static QString artFromImagesArray(const QJsonValue& images)
{
  // search: "images":[{"url":"..."}]
  if (images.isArray())
  {
    const QJsonArray a = images.toArray();
    if (!a.isEmpty())
      return a.first().toObject().value(QStringLiteral("url")).toString();
  }
  // browse: "images":{"tile1x1":"..."}
  if (images.isObject())
  {
    const QJsonObject o = images.toObject();
    if (o.contains(QStringLiteral("tile1x1")))
      return o.value(QStringLiteral("tile1x1")).toString();
    for (const QJsonValue& v : o)
      if (v.isString())
        return v.toString();
  }
  return QString();
}

void CloudMediaModel::parseSearch(const QByteArray& body)
{
  const QJsonObject root = QJsonDocument::fromJson(body).object();
  QList<CloudItem> items;
  const QJsonArray services = root.value(QStringLiteral("services")).toArray();
  for (const QJsonValue& sv : services)
  {
    const QJsonArray resources = sv.toObject().value(QStringLiteral("resources")).toArray();
    for (const QJsonValue& rv : resources)
    {
      const QJsonObject r = rv.toObject();
      const QJsonObject id = r.value(QStringLiteral("id")).toObject();
      CloudItem it;
      it.objectId = id.value(QStringLiteral("objectId")).toString();
      it.href = r.value(QStringLiteral("href")).toString();
      it.title = r.value(QStringLiteral("name")).toString();
      it.subtitle = r.value(QStringLiteral("summary")).toObject().value(QStringLiteral("content")).toString();
      it.art = artFromImagesArray(r.value(QStringLiteral("images")));
      it.type = mapSearchType(r.value(QStringLiteral("type")).toString(), it.isContainer, it.canPlay);
      if (r.contains(QStringLiteral("playable")) && !r.value(QStringLiteral("playable")).toBool())
        it.canPlay = false;
      if (!it.objectId.isEmpty() && !it.title.isEmpty())
        items.append(it);
    }
  }
  resetItems(items);
  emit loaded(true);
}

// Recursively collect any object that looks like a content item (has
// resource.id.objectId and a title/name). Used as a fallback for browse
// responses whose template nests items under an unknown key.
static void collectItemsRecursive(const QJsonValue& v, QList<QJsonObject>& out)
{
  if (v.isObject())
  {
    const QJsonObject o = v.toObject();
    const QString oid = o.value(QStringLiteral("resource")).toObject()
                         .value(QStringLiteral("id")).toObject()
                         .value(QStringLiteral("objectId")).toString();
    const bool hasTitle = o.contains(QStringLiteral("title")) || o.contains(QStringLiteral("name"));
    if (!oid.isEmpty() && hasTitle)
    {
      out.append(o);
      return; // matched item: don't descend into it
    }
    for (auto it = o.begin(); it != o.end(); ++it)
      collectItemsRecursive(it.value(), out);
  }
  else if (v.isArray())
  {
    for (const QJsonValue& e : v.toArray())
      collectItemsRecursive(e, out);
  }
}

void CloudMediaModel::parseBrowse(const QByteArray& body)
{
  const QJsonObject root = QJsonDocument::fromJson(body).object();

  // Collect item objects across the possible shapes:
  //  - a normal container browse: section.items[]
  //  - the provider home page:    sections.items[].items[]  (nested sections)
  //  - a bare list:               items[]
  QList<QJsonObject> raw;
  const QJsonArray secItems = root.value(QStringLiteral("section")).toObject()
                                  .value(QStringLiteral("items")).toArray();
  for (const QJsonValue& v : secItems)
    raw.append(v.toObject());
  const QJsonArray sections = root.value(QStringLiteral("sections")).toObject()
                                  .value(QStringLiteral("items")).toArray();
  for (const QJsonValue& sv : sections)
    for (const QJsonValue& v : sv.toObject().value(QStringLiteral("items")).toArray())
      raw.append(v.toObject());
  if (raw.isEmpty())
    for (const QJsonValue& v : root.value(QStringLiteral("items")).toArray())
      raw.append(v.toObject());
  // Fallback for unknown templates (e.g. album/playlist track pages): walk the
  // whole response, skipping the top-level container ("resource") itself.
  if (raw.isEmpty())
    for (auto it = root.begin(); it != root.end(); ++it)
      if (it.key() != QStringLiteral("resource"))
        collectItemsRecursive(it.value(), raw);

  QList<CloudItem> items;
  for (const QJsonObject& o : raw)
  {
    const QJsonObject res = o.value(QStringLiteral("resource")).toObject();
    const QJsonObject id = res.value(QStringLiteral("id")).toObject();
    CloudItem it;
    it.objectId = id.value(QStringLiteral("objectId")).toString();
    it.href = o.value(QStringLiteral("href")).toString();
    it.title = o.value(QStringLiteral("title")).toString();
    it.subtitle = o.value(QStringLiteral("subtitle")).toString();
    it.art = artFromImagesArray(o.value(QStringLiteral("images")));
    // Prefer resource.type (CONTAINER/PLAYLIST/ALBUM/ARTIST/TRACK); the
    // item-level type (ITEM_AUDIO, ...) is less specific for containers.
    QString t = res.value(QStringLiteral("type")).toString();
    if (t.isEmpty())
      t = o.value(QStringLiteral("type")).toString();
    it.type = mapBrowseType(t, it.isContainer, it.canPlay);
    if (!it.objectId.isEmpty() && !it.title.isEmpty())
      items.append(it);
  }
  resetItems(items);
  emit loaded(true);
}

void CloudMediaModel::resetItems(const QList<CloudItem>& items)
{
  beginResetModel();
  m_items = items;
  endResetModel();
  emit countChanged();
}

QVariant CloudMediaModel::buildPayload(const CloudItem& it) const
{
  using namespace SONOS;
  if (!it.canPlay || it.objectId.isEmpty())
    return QVariant();

  const std::string oid = it.objectId.toStdString();
  const std::string sn  = m_accountId.toStdString();
  const std::string sid = m_sid.toStdString();
  const std::string svc = m_service.toStdString();
  const std::string desc = std::string("SA_RINCON") + svc + "_X_#Svc" + svc + "-0-Token";

  DigitalItemPtr d;
  if (it.type == TypeTrack)
  {
    d = DigitalItemPtr(new DigitalItem(DigitalItem::Type_item, DigitalItem::SubType_audioItem));
    d->SetObjectID(std::string("00032020") + oid);
    d->SetParentID("00020000");
    ElementPtr res(new Element("res", std::string("x-sonosapi-hls-static:") + oid +
                               "?sid=" + sid + "&flags=8&sn=" + sn));
    res->SetAttribut("protocolInfo", "sonos.com-http:*:application/x-mpegURL:*");
    d->SetProperty(res);
    d->SetProperty("upnp:class", "object.item.audioItem.musicTrack");
  }
  else if (it.type == TypeAlbum)
  {
    d = DigitalItemPtr(new DigitalItem(DigitalItem::Type_container, DigitalItem::SubType_album));
    d->SetObjectID(std::string("0004206c") + oid);
    d->SetParentID("00020000");
    ElementPtr res(new Element("res", std::string("x-rincon-cpcontainer:1004206c") + oid +
                               "?sid=" + sid + "&flags=8300&sn=" + sn));
    res->SetAttribut("protocolInfo", "x-rincon-cpcontainer:*:*:*");
    d->SetProperty(res);
    d->SetProperty("upnp:class", "object.container.album.musicAlbum");
  }
  else if (it.type == TypePlaylist)
  {
    d = DigitalItemPtr(new DigitalItem(DigitalItem::Type_container, DigitalItem::SubType_playlistContainer));
    d->SetObjectID(std::string("0006206c") + oid);
    d->SetParentID("00020000");
    ElementPtr res(new Element("res", std::string("x-rincon-cpcontainer:1006206c") + oid +
                               "?sid=" + sid + "&flags=8300&sn=" + sn));
    res->SetAttribut("protocolInfo", "x-rincon-cpcontainer:*:*:*");
    d->SetProperty(res);
    d->SetProperty("upnp:class", "object.container.playlistContainer");
  }
  else
  {
    return QVariant();
  }

  d->SetProperty("dc:title", it.title.toStdString());
  if (!it.art.isEmpty())
    d->SetProperty("upnp:albumArtURI", it.art.toStdString());
  ElementPtr e(new Element("desc", desc));
  e->SetAttribut("id", "cdudn");
  e->SetAttribut("nameSpace", RINC_NS);
  d->SetProperty(e);

  return QVariant::fromValue<SONOS::DigitalItemPtr>(d);
}

void CloudMediaModel::setBusy(bool b)
{
  if (m_busy != b)
  {
    m_busy = b;
    emit busyChanged();
  }
}
