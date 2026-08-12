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

#ifndef NOSONAPPCLOUDMEDIAMODEL_H
#define NOSONAPPCLOUDMEDIAMODEL_H

#include <QAbstractListModel>
#include <QNetworkAccessManager>
#include <QStack>
#include <QVariantMap>

QT_FORWARD_DECLARE_CLASS(QNetworkReply)

namespace nosonapp
{

class SonosAccount;

struct CloudItem
{
  QString title;
  QString subtitle;
  QString art;
  QString objectId;
  QString href;     // API-provided browse URL (authoritative), may be empty
  int type;         // ItemType
  bool isContainer;
  bool canPlay;
};

class CloudMediaModel : public QAbstractListModel
{
  Q_OBJECT
  Q_PROPERTY(int count READ rowCount NOTIFY countChanged)
  Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
  Q_PROPERTY(bool canGoBack READ canGoBack NOTIFY pathChanged)
  Q_PROPERTY(bool isRoot READ isRoot NOTIFY pathChanged)
  Q_PROPERTY(QString pathTitle READ pathTitle NOTIFY pathChanged)

public:
  enum Roles {
    PayloadRole = Qt::UserRole + 1,
    TitleRole,
    SubtitleRole,
    ArtRole,
    TypeRole,
    IsContainerRole,
    CanPlayRole,
    ObjectIdRole,
    HrefRole
  };

  enum ItemType {
    TypeUnknown = 0,
    TypeTrack,
    TypeAlbum,
    TypePlaylist,
    TypeArtist,
    TypeContainer
  };
  Q_ENUM(ItemType)

  explicit CloudMediaModel(QObject* parent = nullptr);
  ~CloudMediaModel() override;

  Q_INVOKABLE void init(nosonapp::SonosAccount* account, const QString& household);
  // Discover the account for the service (if needed) then load the home page.
  Q_INVOKABLE void start();

  int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  QVariant data(const QModelIndex& index, int role) const override;

  Q_INVOKABLE QVariantMap get(int row) const;

  Q_INVOKABLE void loadRoot();
  Q_INVOKABLE void search(const QString& term);
  Q_INVOKABLE void browse(const QString& objectId, const QString& title, int type, const QString& href);
  Q_INVOKABLE void back();
  Q_INVOKABLE void clear();

  bool busy() const { return m_busy; }
  bool canGoBack() const { return m_stack.count() > 1; }
  bool isRoot() const { return m_stack.count() <= 1; }
  QString pathTitle() const { return m_stack.isEmpty() ? QString() : m_stack.top().title; }

signals:
  void countChanged();
  void busyChanged();
  void pathChanged();
  void loaded(bool ok);
  void error(const QString& message);

protected:
  QHash<int, QByteArray> roleNames() const override;

private:
  struct Nav { int mode; QString id; QString title; QString term; int type; QString href; }; // mode 0=search 1=browse

  void requestSearch(const QString& term);
  void requestBrowse(const QString& objectId, int type, const QString& href);
  void requestRegistrations();
  static QString browseSegmentForType(int type);
  static QString swapContentHost(const QString& href);
  bool prepare(QNetworkRequest& req);
  void parseSearch(const QByteArray& body);
  void parseBrowse(const QByteArray& body);
  void resetItems(const QList<CloudItem>& items);
  QVariant buildPayload(const CloudItem& it) const;
  void setBusy(bool b);

  static int mapSearchType(const QString& t, bool& isContainer, bool& canPlay);
  static int mapBrowseType(const QString& t, bool& isContainer, bool& canPlay);

  SonosAccount* m_account;
  QNetworkAccessManager m_nam;
  QList<CloudItem> m_items;
  QStack<Nav> m_stack;
  bool m_busy;

  QString m_household;
  QString m_service;   // service type, e.g. "72711"
  QString m_sid;       // service id, e.g. "284" = (type-7)/256
  QString m_accountId; // account serial "sn", e.g. "6"
};

}

#endif // NOSONAPPCLOUDMEDIAMODEL_H
