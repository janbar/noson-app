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

#include "sonosaccount.h"

#include <QWebEngineProfile>
#include <QWebEngineCookieStore>
#include <QNetworkCookie>
#include <QNetworkCookieJar>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QTimer>
#include <qt6keychain/keychain.h>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrl>

using namespace nosonapp;

static const char* COOKIE_NAME = "__Secure-next-auth.session-token";
static const char* SESSION_URL = "https://play.sonos.com/api/auth/session";
static const char* SIGNIN_URL  = "https://play.sonos.com/en-us/login?callbackUrl=https%3A%2F%2Fplay.sonos.com";
static const char* BROWSER_UA  = "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 "
                                 "(KHTML, like Gecko) Chrome/140.0 Safari/537.36";
// Refresh cadence: well inside the observed ~10-day rolling window.
static const int   REFRESH_MS  = 12 * 60 * 60 * 1000;
static const char* KEYCHAIN_SERVICE = "noson-app";
static const char* KEYCHAIN_KEY     = "sonos-session";

SonosAccount::SonosAccount(QObject* parent)
: QObject(parent)
, m_profile(nullptr)
, m_timer(new QTimer(this))
, m_status(SignedOut)
{
  m_timer->setInterval(REFRESH_MS);
  connect(m_timer, &QTimer::timeout, this, &SonosAccount::refresh);
  // Note: replies are handled per-request (below), NOT via the QNAM global
  // finished signal, because CloudMediaModel shares this same QNAM.
}

SonosAccount::~SonosAccount()
{
}

QString SonosAccount::signinUrl() const
{
  return QString::fromUtf8(SIGNIN_URL);
}

QObject* SonosAccount::webProfileObject()
{
  ensureProfile();
  return m_profile;
}

void SonosAccount::ensureProfile()
{
  if (m_profile)
    return;
  // Observe the DEFAULT profile: a QML WebEngineView with no explicit profile
  // uses QWebEngineProfile::defaultProfile(), so this is the store its cookies
  // (including the HttpOnly session cookie) actually land in. Creating our own
  // profile and binding it to the view proved unreliable.
  m_profile = QWebEngineProfile::defaultProfile();
  m_profile->setHttpUserAgent(QString::fromUtf8(BROWSER_UA));
  connect(m_profile->cookieStore(), &QWebEngineCookieStore::cookieAdded,
            this, [this](const QNetworkCookie& cookie) {
      qWarning("[SonosAccount] cookieAdded name=%s domain=%s",
               cookie.name().constData(), cookie.domain().toUtf8().constData());
      // The v2 content API needs the server session cookie too; capture it.
      if (cookie.name() == "JSESSIONID")
      {
        m_jsession = QString::fromUtf8(cookie.value());
        qWarning("[SonosAccount] captured JSESSIONID");
        seedJar();
      }
      // Match by name only: the session cookie is host-only, so domain() is
      // often empty and must not be used to filter it out.
      if (cookie.name() != COOKIE_NAME)
        return;
      QDateTime exp = cookie.expirationDate();
      if (!exp.isValid())
        exp = QDateTime::currentDateTimeUtc().addDays(10); // sane fallback
      qWarning("[SonosAccount] captured session cookie (expiry %s)",
               exp.toString(Qt::ISODate).toUtf8().constData());
      setCookie(QString::fromUtf8(cookie.value()), exp);
      seedJar();
      store();
      setStatus(Ready);
      armTimer();
      emit loginSucceeded();
      // pull the account email for display (best-effort)
      refresh();
    });
  emit webProfileChanged();
}

QString SonosAccount::cookieHeader() const
{
  if (m_cookie.isEmpty())
    return QString();
  QString h = QString::fromUtf8(COOKIE_NAME) + QStringLiteral("=") + m_cookie;
  // The v2 content API (browse) also requires the server session cookie;
  // the v1 search endpoint works without it, which is why only browse 401'd.
  if (!m_jsession.isEmpty())
    h += QStringLiteral("; JSESSIONID=") + m_jsession;
  return h;
}

void SonosAccount::restore()
{
  // Read the stored session from the OS keychain (async).
  QKeychain::ReadPasswordJob* job = new QKeychain::ReadPasswordJob(QString::fromUtf8(KEYCHAIN_SERVICE), this);
  job->setKey(QString::fromUtf8(KEYCHAIN_KEY));
  connect(job, &QKeychain::Job::finished, this, [this, job]() {
    job->deleteLater();
    if (job->error() == QKeychain::NoError)
    {
      const QJsonObject o = QJsonDocument::fromJson(job->textData().toUtf8()).object();
      m_cookie = o.value(QStringLiteral("cookie")).toString();
      m_jsession = o.value(QStringLiteral("jsession")).toString();
      m_expires = QDateTime::fromString(o.value(QStringLiteral("expires")).toString(), Qt::ISODate).toUTC();
      m_email = o.value(QStringLiteral("email")).toString();
    }
    if (m_cookie.isEmpty())
    {
      setStatus(SignedOut);
      return;
    }
    if (m_expires.isValid() && QDateTime::currentDateTimeUtc() >= m_expires)
    {
      setStatus(Expired);
      emit needLogin();
      return;
    }
    // Valid so far: seed the jar with the stored cookie, then roll it forward
    // (the refresh response re-seeds a fresh JSESSIONID into the jar).
    seedJar();
    refresh();
  });
  job->start();
}

void SonosAccount::beginLogin()
{
  webProfileObject(); // ensure profile + capture hook exist
  setStatus(Working);
}

void SonosAccount::refresh()
{
  if (m_cookie.isEmpty())
  {
    setStatus(SignedOut);
    return;
  }
  setStatus(Working);
  seedJar(); // ensure the jar carries the current session cookie
  QNetworkRequest req((QUrl(QString::fromUtf8(SESSION_URL))));
  // No manual Cookie header: the shared jar sends + auto-updates the cookies.
  req.setRawHeader("User-Agent", BROWSER_UA);
  req.setRawHeader("Accept", "application/json");
  req.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
  QNetworkReply* reply = m_nam.get(req);
  connect(reply, &QNetworkReply::finished, this, [this, reply]() { onSessionReply(reply); });
}

void SonosAccount::onSessionReply(QNetworkReply* reply)
{
  reply->deleteLater();
  const QByteArray body = reply->readAll();

  if (reply->error() != QNetworkReply::NoError)
  {
    // network/auth failure: keep the cookie but mark it needing attention
    setStatus(m_cookie.isEmpty() ? SignedOut : Expired);
    emit needLogin();
    return;
  }

  // Rotated Set-Cookie (NextAuth rolls the expiry on each session read).
  const QVariant sc = reply->header(QNetworkRequest::SetCookieHeader);
  if (sc.isValid())
  {
    const QList<QNetworkCookie> cookies = qvariant_cast<QList<QNetworkCookie> >(sc);
    for (const QNetworkCookie& c : cookies)
    {
      if (c.name() == COOKIE_NAME)
      {
        QDateTime exp = c.expirationDate();
        if (!exp.isValid())
          exp = QDateTime::currentDateTimeUtc().addDays(10);
        setCookie(QString::fromUtf8(c.value()), exp);
      }
      else if (c.name() == "JSESSIONID")
      {
        m_jsession = QString::fromUtf8(c.value());
      }
    }
  }

  // An empty session body ({}) means the session is dead; sign in again.
  const QJsonDocument doc = QJsonDocument::fromJson(body);
  const QJsonObject obj = doc.object();
  if (obj.isEmpty() || !obj.contains(QStringLiteral("user")))
  {
    setStatus(Expired);
    emit needLogin();
    return;
  }

  if (obj.contains(QStringLiteral("expires")))
  {
    const QDateTime e = QDateTime::fromString(obj.value(QStringLiteral("expires")).toString(), Qt::ISODate);
    if (e.isValid())
      m_expires = e.toUTC();
  }
  const QJsonObject acct = obj.value(QStringLiteral("user")).toObject()
                              .value(QStringLiteral("account")).toObject();
  m_email = acct.value(QStringLiteral("email")).toString();

  store();
  setStatus(Ready);
  armTimer();
}

void SonosAccount::harvestCookies()
{
  ensureProfile();
  if (m_profile && m_profile->cookieStore())
    m_profile->cookieStore()->loadAllCookies();
}

// Push the current session cookies into the shared jar for play.sonos.com.
// The jar then sends them on every request and updates them from each
// response's Set-Cookie (so a rotated JSESSIONID is carried forward).
void SonosAccount::seedJar()
{
  if (m_cookie.isEmpty())
    return;
  QList<QNetworkCookie> cs;
  QNetworkCookie sc(COOKIE_NAME, m_cookie.toUtf8());
  sc.setPath("/");
  cs << sc;
  if (!m_jsession.isEmpty())
  {
    QNetworkCookie js("JSESSIONID", m_jsession.toUtf8());
    js.setPath("/");
    cs << js;
  }
  m_nam.cookieJar()->setCookiesFromUrl(cs, QUrl(QStringLiteral("https://play.sonos.com/")));
}

void SonosAccount::logout()
{
  m_cookie.clear();
  m_jsession.clear();
  m_expires = QDateTime();
  m_email.clear();
  if (m_profile && m_profile->cookieStore())
    m_profile->cookieStore()->deleteAllCookies();
  QKeychain::DeletePasswordJob* job = new QKeychain::DeletePasswordJob(QString::fromUtf8(KEYCHAIN_SERVICE), this);
  job->setKey(QString::fromUtf8(KEYCHAIN_KEY));
  connect(job, &QKeychain::Job::finished, job, &QObject::deleteLater);
  job->start();
  m_timer->stop();
  setStatus(SignedOut);
}

void SonosAccount::setCookie(const QString& value, const QDateTime& expiry)
{
  m_cookie = value;
  m_expires = expiry.toUTC();
}

void SonosAccount::setStatus(Status s)
{
  if (m_status != s)
  {
    m_status = s;
    emit statusChanged();
  }
  else
  {
    // still emit so bound properties (email/expires) refresh
    emit statusChanged();
  }
}

void SonosAccount::armTimer()
{
  if (!m_timer->isActive())
    m_timer->start();
}

// Persist the session in the OS keychain (async, fire-and-forget).
void SonosAccount::store()
{
  QJsonObject o;
  o[QStringLiteral("cookie")] = m_cookie;
  o[QStringLiteral("jsession")] = m_jsession;
  o[QStringLiteral("expires")] = m_expires.toString(Qt::ISODate);
  o[QStringLiteral("email")] = m_email;
  QKeychain::WritePasswordJob* job = new QKeychain::WritePasswordJob(QString::fromUtf8(KEYCHAIN_SERVICE), this);
  job->setKey(QString::fromUtf8(KEYCHAIN_KEY));
  job->setTextData(QString::fromUtf8(QJsonDocument(o).toJson(QJsonDocument::Compact)));
  connect(job, &QKeychain::Job::finished, job, &QObject::deleteLater);
  job->start();
}
