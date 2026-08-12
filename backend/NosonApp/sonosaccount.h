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

#ifndef NOSONAPPSONOSACCOUNT_H
#define NOSONAPPSONOSACCOUNT_H

#include <QObject>
#include <QString>
#include <QDateTime>
#include <QNetworkAccessManager>

QT_FORWARD_DECLARE_CLASS(QWebEngineProfile)
QT_FORWARD_DECLARE_CLASS(QTimer)
QT_FORWARD_DECLARE_CLASS(QNetworkReply)

namespace nosonapp
{

class SonosAccount : public QObject
{
  Q_OBJECT
  Q_PROPERTY(int status READ status NOTIFY statusChanged)
  Q_PROPERTY(QString email READ email NOTIFY statusChanged)
  Q_PROPERTY(QString expires READ expiresString NOTIFY statusChanged)
  Q_PROPERTY(QObject* webProfile READ webProfileObject NOTIFY webProfileChanged)
  Q_PROPERTY(QString signinUrl READ signinUrl CONSTANT)

public:
  enum Status {
    SignedOut = 0, // no stored cookie
    Expired   = 1, // stored cookie has passed its expiry; must sign in again
    Working   = 2, // a login/refresh is in flight
    Ready     = 3  // have a valid, refreshed cookie
  };
  Q_ENUM(Status)

  explicit SonosAccount(QObject* parent = nullptr);
  ~SonosAccount() override;

  int status() const { return m_status; }
  QString email() const { return m_email; }
  QString expiresString() const { return m_expires.toString(Qt::ISODate); }
  QString signinUrl() const;

  // The QtWebEngine profile the login WebEngineView must use so that we can
  // observe the (HttpOnly) session cookie it sets. Lazily created.
  QObject* webProfileObject();

  // The raw "name=value" cookie header value (legacy; requests now use the
  // shared cookie jar via nam() instead). Empty when not Ready.
  QString cookieHeader() const;

  // Shared network manager whose cookie jar carries the Sonos session cookies
  // and auto-updates from every response's Set-Cookie (handles JSESSIONID
  // rotation). Content requests must go through this so the session stays live.
  QNetworkAccessManager* nam() { return &m_nam; }

  bool isReady() const { return m_status == Ready; }

  // Boot entry point: load stored cookie; if valid, refresh; if expired,
  // signal the UI to re-login; if none, stay SignedOut.
  Q_INVOKABLE void restore();

  // Begin an interactive login: prepares the web profile + cookie capture.
  // The QML side opens signinUrl() in a WebEngineView bound to webProfile().
  Q_INVOKABLE void beginLogin();

  // Force a background refresh now.
  Q_INVOKABLE void refresh();

  // Force the web profile to re-emit every stored cookie (cookieAdded), so the
  // login view can harvest the session cookie once it lands back on Sonos.
  Q_INVOKABLE void harvestCookies();

  // Clear the stored session and profile cookies.
  Q_INVOKABLE void logout();

signals:
  void statusChanged();
  void webProfileChanged();
  void loginSucceeded();
  void loginFailed(const QString& reason);
  void needLogin(); // stored session expired / invalid

private slots:
  void onSessionReply(QNetworkReply* reply);

private:
  void ensureProfile();
  void seedJar();
  void setStatus(Status s);
  void store();
  void setCookie(const QString& value, const QDateTime& expiry);
  void armTimer();

  QNetworkAccessManager m_nam;
  QWebEngineProfile* m_profile;
  QTimer* m_timer;
  Status m_status;
  QString m_cookie;      // value of __Secure-next-auth.session-token
  QString m_jsession;    // value of JSESSIONID (needed by the v2 content API)
  QDateTime m_expires;   // when the session lapses
  QString m_email;
};

}

#endif // NOSONAPPSONOSACCOUNT_H
