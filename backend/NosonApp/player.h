/*
 *      Copyright (C) 2015-2019 Jean-Luc Barriere
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

#ifndef PLAYER_H
#define PLAYER_H

#include "private/os/threads/threadpool.h"
#include <noson/sonosplayer.h>

#include <QObject>

class Sonos;

class Player : public QObject
{
  Q_OBJECT
  Q_PROPERTY(bool muteMaster READ muteMaster NOTIFY renderingGroupChanged)
  Q_PROPERTY(bool nightmode READ nightmode NOTIFY renderingGroupChanged)
  Q_PROPERTY(int volumeMaster READ volumeMaster NOTIFY renderingGroupChanged)
  Q_PROPERTY(int treble READ treble NOTIFY renderingGroupChanged)
  Q_PROPERTY(int bass READ bass NOTIFY renderingGroupChanged)

  // Read only
  Q_PROPERTY(bool connected READ connected NOTIFY connectedChanged())
  Q_PROPERTY(QString currentMetaAlbum READ currentMetaAlbum NOTIFY sourceChanged)
  Q_PROPERTY(QString currentMetaArt READ currentMetaArt NOTIFY sourceChanged)
  Q_PROPERTY(QString currentMetaArtist READ currentMetaArtist NOTIFY sourceChanged)
  Q_PROPERTY(QString currentMetaSource READ currentMetaSource NOTIFY sourceChanged)
  Q_PROPERTY(QString currentMetaTitle READ currentMetaTitle NOTIFY sourceChanged)
  Q_PROPERTY(QString currentMetaURITitle READ currentMetaURITitle NOTIFY sourceChanged)
  Q_PROPERTY(int currentIndex READ currentIndex NOTIFY sourceChanged)
  Q_PROPERTY(int currentTrackDuration READ currentTrackDuration NOTIFY sourceChanged)
  Q_PROPERTY(int currentProtocol READ currentProtocol NOTIFY sourceChanged)

  Q_PROPERTY(QString playbackState READ playbackState NOTIFY playbackStateChanged)
  Q_PROPERTY(QString playMode READ playMode NOTIFY playModeChanged)

public:
  explicit Player(QObject *parent = 0);
  ~Player();

  Q_INVOKABLE bool init(QObject* sonos);
  bool connected() const { return m_connected; }
  void beginJob();
  void endJob();
  Q_INVOKABLE void renewSubscriptions();
  Q_INVOKABLE bool ping();

  Q_INVOKABLE bool refreshShareIndex();
  Q_INVOKABLE bool configureSleepTimer(int seconds);
  Q_INVOKABLE int remainingSleepTimerDuration();

  Q_INVOKABLE bool startPlaySource(const QVariant& payload); // asynchronous
  Q_INVOKABLE bool play();
  Q_INVOKABLE bool stop();
  Q_INVOKABLE bool pause();
  Q_INVOKABLE bool previous();
  Q_INVOKABLE bool next();
  Q_INVOKABLE bool toggleRepeat();
  Q_INVOKABLE bool toggleShuffle();
  Q_INVOKABLE bool setSource(const QVariant& payload);
  Q_INVOKABLE bool toggleMute();
  Q_INVOKABLE bool toggleMute(const QString& uuid);

  Q_INVOKABLE bool toggleNightmode();
  Q_INVOKABLE bool toggleNightmode(const QString& uuid);

  Q_INVOKABLE bool startPlayStream(const QString& url, const QString& title); // asynchonous
  Q_INVOKABLE bool playStream(const QString& url, const QString& title);
  Q_INVOKABLE bool playLineIN();
  Q_INVOKABLE bool playDigitalIN();
  Q_INVOKABLE bool playQueue(bool start);
  Q_INVOKABLE bool seekTime(int timesec);
  Q_INVOKABLE bool seekTrack(int position);
  Q_INVOKABLE int addItemToQueue(const QVariant& payload, int position);
  Q_INVOKABLE int addMultipleItemsToQueue(const QVariantList& payloads);
  Q_INVOKABLE bool removeAllTracksFromQueue();
  Q_INVOKABLE bool removeTrackFromQueue(const QString& id, int containerUpdateID);
  Q_INVOKABLE bool reorderTrackInQueue(int trackNo, int newPosition, int containerUpdateID);

  Q_INVOKABLE bool saveQueue(const QString& title);
  Q_INVOKABLE bool createSavedQueue(const QString& title);
  Q_INVOKABLE int addItemToSavedQueue(const QString& SQid, const QVariant& payload, int containerUpdateID);
  Q_INVOKABLE bool removeTracksFromSavedQueue(const QString& SQid, const QVariantList& indexes, int containerUpdateID);
  Q_INVOKABLE bool reorderTrackInSavedQueue(const QString& SQid, int index, int newIndex, int containerUpdateID);
  Q_INVOKABLE bool destroySavedQueue(const QString& SQid);

  Q_INVOKABLE bool addItemToFavorites(const QVariant& payload, const QString& description, const QString& artURI);
  Q_INVOKABLE bool destroyFavorite(const QString& FVid);
  Q_INVOKABLE bool startPlayFavorite(const QVariant& payload); // asynchronous
  Q_INVOKABLE bool playFavorite(const QVariant& payload);

  bool nightmode() const { return m_RCGroup.nightmode; }
  bool muteMaster() const { return m_RCGroup.mute; }
  int muteLF() const { return m_RCGroup.mute; }
  int muteRF() const { return m_RCGroup.mute; }
  int volumeMaster() const { return m_RCGroup.volume; }
  int volumeLF() const { return m_RCGroup.volume; }
  int volumeRF() const { return m_RCGroup.volume; }
  int treble() const { return m_RCGroup.treble; }
  int bass() const { return m_RCGroup.bass; }

  struct RCProperty
  {
    std::string uuid;
    std::string name;
    bool mute = false;
    bool nightmode = false;
    int volume = 0;
    int treble = 0;
    int bass = 0;
    double volumeFake = 0.;
  };

  typedef std::vector<RCProperty> RCTable;

  const RCTable& renderingTable() const { return m_RCTable; }

  Q_INVOKABLE bool setTreble(double val);
  Q_INVOKABLE bool setBass(double val);

  Q_INVOKABLE bool setVolumeGroup(double volume);
  Q_INVOKABLE bool setVolume(const QString& uuid, double volume);

  Q_INVOKABLE int currentTrackPosition();

  void setCurrentMeta(const SONOS::AVTProperty& prop);
  const QString& currentMetaAlbum() const { return m_currentMetaAlbum; }
  const QString& currentMetaArt() const { return m_currentMetaArt; }
  const QString& currentMetaArtist() const { return m_currentMetaArtist; }
  const QString& currentMetaSource() const { return m_currentMetaSource; }
  const QString& currentMetaTitle() const { return m_currentMetaTitle; }
  const QString& currentMetaURITitle() const { return m_currentMetaURITitle; }
  int currentIndex() const { return m_currentIndex; }
  int currentTrackDuration() const { return m_currentTrackDuration; }
  int currentProtocol() const { return m_currentProtocol; } // returns SONOS::Protocol_t
  QString playbackState() const { return QString::fromUtf8(m_AVTProperty.TransportState.c_str()); }
  QString playMode() const { return QString::fromUtf8(m_AVTProperty.CurrentPlayMode.c_str()); }

signals:
  void jobFailed();
  void connectedChanged();
  void renderingChanged();
  void renderingGroupChanged();
  void sourceChanged();
  void statusChanged();
  void stopped();
  void playbackStateChanged();
  void playModeChanged();
  void sleepTimerChanged();

private slots:
    void handleTransportChange();
    void handleRenderingControlChange();
  
private:
  Sonos* m_sonos;
  SONOS::PlayerPtr m_player;
  SONOS::AVTProperty m_AVTProperty;

  RCTable m_RCTable; // Rendering control of subordinates
  RCProperty m_RCGroup; // Rendering control of the group

  bool m_connected;

  QString m_currentMetaAlbum;
  QString m_currentMetaArt;
  QString m_currentMetaArtist;
  QString m_currentMetaSource;
  QString m_currentMetaTitle;
  QString m_currentMetaURITitle;
  int m_currentIndex;
  int m_currentTrackDuration;
  int m_currentProtocol;

  void connectSonos(Sonos* sonos);
  void disconnectSonos(Sonos* sonos);
};

#endif // PLAYER_H
