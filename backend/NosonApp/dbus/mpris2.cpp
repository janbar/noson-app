/*
 *      Copyright (C) 2019 Jean-Luc Barriere
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

#include "mpris2.h"

#include <QGuiApplication>
#include <QDBusConnection>
#include <QDebug>

#include "mpris2_player.h"
#include "mpris2_root.h"
#include "player.h"

#define MPRIS_OBJECT_PATH     "/org/mpris/MediaPlayer2"
#define DBUS_MEDIAPLAYER_SVC  "org.mpris.MediaPlayer2"
#define DBUS_FREEDESKTOP_SVC  "org.freedesktop.DBus.Properties"

using namespace nosonapp;

QString Mpris2::serviceName = QString(DBUS_MEDIAPLAYER_SVC ".%1").arg(QGuiApplication::applicationDisplayName());
QString Mpris2::servicePath = QString("/%1").arg(QString(QGuiApplication::applicationName()).replace('.', '/'));

Mpris2::Mpris2(Player* app, QObject* parent)
: QObject(parent)
, m_player(app)
{
  new Mpris2Root(this);
  new Mpris2Player(this);

  if (!QDBusConnection::sessionBus().registerService(serviceName))
  {
    qWarning() << "Failed to register" << serviceName << "on the session bus";
    return;
  }

  QDBusConnection::sessionBus().registerObject(MPRIS_OBJECT_PATH, this);

  if (m_player)
  {
    QObject::connect(m_player, SIGNAL(connectedChanged()), SLOT(connectionStateChanged()));
    QObject::connect(m_player, SIGNAL(playbackStateChanged()), SLOT(playbackStateChanged()));
    QObject::connect(m_player, SIGNAL(renderingGroupChanged()), SLOT(volumeChanged()));
    QObject::connect(m_player, SIGNAL(playModeChanged()), SLOT(playModeChanged()));
    QObject::connect(m_player, SIGNAL(sourceChanged()), SLOT(currentTrackChanged()));
    //QObject::connect(m_player, SIGNAL(?), SIGNAL(Seeked(qlonglong)));
  }
}

Mpris2::~Mpris2()
{
  QDBusConnection::sessionBus().unregisterService(serviceName);
}

void Mpris2::connectionStateChanged()
{
  m_metadata = QVariantMap();
  emitNotification("Metadata");
  emitNotification("CanPlay", CanPlay());
  emitNotification("CanPause", CanPause());
  emitNotification("PlaybackStatus", PlaybackStatus());
  if (m_player->playbackState() == "PLAYING")
    emitNotification("CanSeek", CanSeek());
}

void Mpris2::playbackStateChanged()
{
  emitNotification("CanPlay", CanPlay());
  emitNotification("CanPause", CanPause());
  emitNotification("PlaybackStatus", PlaybackStatus());
  if (m_player->playbackState() == "PLAYING")
    emitNotification("CanSeek", CanSeek());
}

void Mpris2::volumeChanged()
{
  emitNotification("Volume");
}

void Mpris2::playModeChanged()
{
  emitNotification("Shuffle");
  emitNotification("LoopStatus");
  emitNotification("CanGoNext", CanGoNext());
  emitNotification("CanGoPrevious", CanGoPrevious());
}

void Mpris2::emitNotification(const QString& name, const QVariant& val)
{
  emitNotification(name, val, "org.mpris.MediaPlayer2.Player");
}

void Mpris2::emitNotification(const QString& name, const QVariant& val,
                              const QString& mprisEntity)
{
  QDBusMessage msg = QDBusMessage::createSignal(MPRIS_OBJECT_PATH, DBUS_FREEDESKTOP_SVC, "PropertiesChanged");
  QVariantMap map;
  map.insert(name, val);
  QVariantList args = QVariantList() << mprisEntity << map << QStringList();
  msg.setArguments(args);
  QDBusConnection::sessionBus().send(msg);
}

void Mpris2::emitNotification(const QString& name)
{
  QVariant value;
  if (name == "PlaybackStatus")
    value = PlaybackStatus();
  else if (name == "LoopStatus")
    value = LoopStatus();
  else if (name == "Shuffle")
    value = Shuffle();
  else if (name == "Metadata")
    value = Metadata();
  else if (name == "Volume")
    value = Volume();
  else if (name == "Position")
    value = Position();
  else if (name == "CanGoNext")
    value = CanGoNext();
  else if (name == "CanGoPrevious")
    value = CanGoPrevious();
  else if (name == "CanSeek")
    value = CanSeek();
  else if (name == "CanPlay")
    value = CanPlay();
  else if (name == "CanPause")
    value = CanPause();

  if (value.isValid())
    emitNotification(name, value);
}

QString Mpris2::Identity() const
{
  return QCoreApplication::applicationName();
}

QString Mpris2::desktopEntryAbsolutePath() const
{
  QString appId = DesktopEntry();
  QStringList xdg_data_dirs = QString(getenv("XDG_DATA_DIRS")).split(":");
  xdg_data_dirs.append("/usr/local/share/");
  xdg_data_dirs.append("/usr/share/");

  for (const QString& directory : xdg_data_dirs)
  {
    QString path = QString("%1/applications/%2.desktop")
            .arg(directory, appId);
    if (QFile::exists(path))
      return path;
  }
  return QString();
}

QString Mpris2::DesktopEntry() const
{
  return QGuiApplication::applicationName().toLower();
}

QStringList Mpris2::SupportedUriSchemes() const
{
  static QStringList res = QStringList()
          << "file"
          << "http";
  return res;
}

QStringList Mpris2::SupportedMimeTypes() const
{
  static QStringList res = QStringList()
          << "audio/aac"
          << "audio/mp3"
          << "audio/flac"
          << "audio/ogg"
          << "application/ogg"
          << "audio/x-mp3"
          << "audio/x-flac"
          << "application/x-ogg";
  return res;
}

void Mpris2::Raise()
{
}

void Mpris2::Quit()
{
  //qApp->quit();
}

QString Mpris2::PlaybackStatus() const
{
  QString state = m_player->playbackState();
  if (state == "PLAYING")
    return "Playing";
  if (state == "PAUSED_PLAYBACK")
    return "Paused";
  return "Stopped";
}

QString Mpris2::LoopStatus() const
{
  QString mode = m_player->playMode();
  if (mode == "SHUFFLE")
    return "Playlist";
  if (mode == "REPEAT_ALL")
    return "Playlist";
  if (mode == "REPEAT_ONE")
    return "Track";
  return "None";
}

void Mpris2::SetLoopStatus(const QString& value)
{
  QString mode = m_player->playMode();
  if ((value == "None" && (mode == "REPEAT_ALL" || mode == "SHUFFLE" || mode == "REPEAT_ONE")) ||
          (value == "Playlist" && (mode == "NORMAL" || mode == "SHUFFLE_NOREPEAT")))
    m_player->toggleRepeat();
}

double Mpris2::Rate() const
{
  return 1.0;
}

void Mpris2::SetRate(double rate)
{
  if (rate == 0)
    m_player->pause();
}

bool Mpris2::Shuffle() const
{
  QString mode = m_player->playMode();
  return (mode == "SHUFFLE" || mode == "SHUFFLE_NOREPEAT");
}

void Mpris2::SetShuffle(bool enable)
{
  QString mode = m_player->playMode();
  if ((mode == "SHUFFLE" || mode == "SHUFFLE_NOREPEAT") != enable)
    m_player->toggleShuffle();
}

QVariantMap Mpris2::Metadata() const
{
  return m_metadata;
}

QString Mpris2::makeTrackId(int index) const
{
  return QString("%1/track/%2").arg(servicePath).arg(QString::number(index));
}

void Mpris2::currentTrackChanged()
{
  emitNotification("CanPlay");
  emitNotification("CanPause");
  emitNotification("CanGoNext", CanGoNext());
  emitNotification("CanGoPrevious", CanGoPrevious());
  emitNotification("CanSeek", CanSeek());

  m_metadata = QVariantMap();
  addMetadata("mpris:trackid", makeTrackId(m_player->currentIndex()), &m_metadata);
  addMetadata("mpris:length", (qint64)(1000000L * m_player->currentTrackDuration()), &m_metadata);
  addMetadata("mpris:artUrl", m_player->currentMetaArt(), &m_metadata);
  addMetadata("xesam:title", m_player->currentMetaTitle(), &m_metadata);
  addMetadata("xesam:album", m_player->currentMetaAlbum(), &m_metadata);
  addMetadataAsList("xesam:artist", m_player->currentMetaArtist(), &m_metadata);

  emitNotification("Metadata", m_metadata);
}

double Mpris2::Volume() const
{
  return (double) (m_player->volumeMaster()) / 100.0;
}

void Mpris2::SetVolume(double value)
{
  m_player->setVolumeGroup(value * 100.0);
}

qlonglong Mpris2::Position() const
{
  return 1000000L * m_player->currentTrackPosition();
}

double Mpris2::MaximumRate() const
{
  return 1.0;
}

double Mpris2::MinimumRate() const
{
  return 1.0;
}

bool Mpris2::CanGoNext() const
{
  return (m_player->currentTrackDuration() > 0 && m_player->numberOfTracks() > (m_player->currentIndex() + 1));
}

bool Mpris2::CanGoPrevious() const
{
  return (m_player->currentTrackDuration() > 0 && m_player->currentIndex() > 0);
}

bool Mpris2::CanPlay() const
{
  return true;
}

bool Mpris2::CanPause() const
{
  return true;
}

bool Mpris2::CanSeek() const
{
  switch (m_player->currentProtocol())
  {
  case 1: // x-rincon-stream
  case 2: // x-rincon-mp3radio
  case 5: // x-sonos-htastream
  case 14: // http-get
  case 17: // http
    return false;
  default:
    return (m_player->currentTrackDuration() > 0);
  }
}

bool Mpris2::CanControl() const
{
  return true;
}

void Mpris2::Next()
{
  if (CanGoNext())
    m_player->next();
}

void Mpris2::Previous()
{
  if (CanGoPrevious())
    m_player->previous();
}

void Mpris2::Pause()
{
  if (CanPause() && m_player->playbackState() == "PLAYING")
    m_player->pause();
}

void Mpris2::PlayPause()
{
  if (CanPause())
  {
    QString state = m_player->playbackState();
    if (state == "PLAYING")
      m_player->pause();
    else if (state == "STOPPED" || state == "PAUSED_PLAYBACK")
      m_player->play();
  }
}

void Mpris2::Stop()
{
  m_player->stop();
}

void Mpris2::Play()
{
  if (CanPlay())
  {
    m_player->play();
  }
}

void Mpris2::Seek(qlonglong offset)
{
  if (CanSeek())
    m_player->seekTime(m_player->currentTrackPosition() + offset / 1000000L);
}

void Mpris2::SetPosition(const QDBusObjectPath& trackId, qlonglong offset)
{
  if (CanSeek() && trackId.path() == makeTrackId(m_player->currentIndex()) && offset >= 0)
    m_player->seekTime(offset / 1000000L);
}

void Mpris2::OpenUri(const QString& uri)
{
}
