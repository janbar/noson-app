/*
 *      Copyright (C) 2015-2016 Jean-Luc Barriere
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published
 *  by the Free Software Foundation; either version 3, or (at your option)
 *  any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this library; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 51 Franklin Street, Fifth Floor, Boston,
 *  MA 02110-1301 USA
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#ifndef PLAYER_H
#define PLAYER_H

#include "../../lib/noson/noson/src/sonosplayer.h"

#include <QObject>

class Sonos;

class Player : public QObject
{
  Q_OBJECT
  Q_PROPERTY(bool muteMaster READ muteMaster NOTIFY renderingGroupChanged)
  Q_PROPERTY(int volumeMaster READ volumeMaster NOTIFY renderingGroupChanged)

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
  
  Q_PROPERTY(QString playbackState READ playbackState NOTIFY playbackStateChanged)
  Q_PROPERTY(QString playMode READ playMode NOTIFY playModeChanged)

public:
  explicit Player(QObject *parent = 0);
  ~Player();

  Q_INVOKABLE bool init(QObject* sonos);
  bool connected() { return m_connected; }
  Q_INVOKABLE void renewSubscriptions();
  Q_INVOKABLE bool ping();

  Q_INVOKABLE bool refreshShareIndex();
  Q_INVOKABLE bool configureSleepTimer(int seconds);
  Q_INVOKABLE int remainingSleepTimerDuration();

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

  Q_INVOKABLE bool addItemToFavorites(const QVariant& payload, const QString& description);
  Q_INVOKABLE bool destroyFavorite(const QString& FVid);
  Q_INVOKABLE bool playFavorite(const QVariant& payload);

  bool muteMaster() { return m_RCGroup.mute; }
  int muteLF() { return m_RCGroup.mute; }
  int muteRF() { return m_RCGroup.mute; }
  int volumeMaster() { return m_RCGroup.volume; }
  int volumeLF() { return m_RCGroup.volume; }
  int volumeRF() { return m_RCGroup.volume; }

  struct RCProperty
  {
    std::string uuid;
    std::string name;
    bool mute;
    int volume;
    double volumeFake;
  };

  typedef std::vector<RCProperty> RCTable;

  const RCTable& renderingTable() const { return m_RCTable; }

  Q_INVOKABLE bool setVolumeGroup(double volume);
  Q_INVOKABLE bool setVolume(const QString& uuid, double volume);

  Q_INVOKABLE int currentTrackPosition();

  void setCurrentMeta(const SONOS::AVTProperty& prop);
  QString currentMetaAlbum() { return m_currentMetaAlbum; }
  QString currentMetaArt() { return m_currentMetaArt; }
  QString currentMetaArtist() { return m_currentMetaArtist; }
  QString currentMetaSource() { return m_currentMetaSource; }
  QString currentMetaTitle() { return m_currentMetaTitle; }
  QString currentMetaURITitle() { return m_currentMetaURITitle; }
  int currentIndex() { return m_currentIndex; }
  int currentTrackDuration() { return m_currentTrackDuration; }
  QString playbackState() { return QString::fromUtf8(m_AVTProperty.TransportState.c_str()); }
  QString playMode() { return QString::fromUtf8(m_AVTProperty.CurrentPlayMode.c_str()); }

signals:
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

  void connectSonos(Sonos* sonos);
  void disconnectSonos(Sonos* sonos);
};

#endif // PLAYER_H
