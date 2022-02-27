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

#ifndef NOSONAPPPLAYER_H
#define NOSONAPPPLAYER_H

#include <noson/sonosplayer.h>

#include "queuemodel.h"
#include "future.h"

#include <QObject>
#include <QList>

namespace nosonapp
{

class Sonos;
class Mpris2;

class Player : public QObject, public ContentProvider<Player>
{
  Q_OBJECT
  Q_PROPERTY(int renderingCount READ renderingCount NOTIFY renderingCountChanged)
  Q_PROPERTY(bool muteMaster READ muteMaster NOTIFY renderingGroupChanged)
  Q_PROPERTY(bool nightmode READ nightmode NOTIFY renderingGroupChanged)
  Q_PROPERTY(bool loudness READ loudness NOTIFY renderingGroupChanged)
  Q_PROPERTY(bool outputFixed READ outputFixed NOTIFY renderingGroupChanged)
  Q_PROPERTY(int volumeMaster READ volumeMaster NOTIFY renderingGroupChanged)
  Q_PROPERTY(int treble READ treble NOTIFY renderingGroupChanged)
  Q_PROPERTY(int bass READ bass NOTIFY renderingGroupChanged)

  // Read only
  Q_PROPERTY(int pid READ pid NOTIFY connectedChanged)
  Q_PROPERTY(bool connected READ connected NOTIFY connectedChanged)
  Q_PROPERTY(QString zoneId READ zoneId NOTIFY connectedChanged)
  Q_PROPERTY(QString zoneName READ zoneName NOTIFY connectedChanged)
  Q_PROPERTY(QString zoneShortName READ zoneShortName NOTIFY connectedChanged)
  Q_PROPERTY(QString controllerURI READ controllerURI NOTIFY connectedChanged)
  Q_PROPERTY(QString currentMetaAlbum READ currentMetaAlbum NOTIFY sourceChanged)
  Q_PROPERTY(QString currentMetaArt READ currentMetaArt NOTIFY sourceChanged)
  Q_PROPERTY(QString currentMetaArtist READ currentMetaArtist NOTIFY sourceChanged)
  Q_PROPERTY(QString currentMetaSource READ currentMetaSource NOTIFY sourceChanged)
  Q_PROPERTY(QString currentMetaTitle READ currentMetaTitle NOTIFY sourceChanged)
  Q_PROPERTY(QString currentMetaURITitle READ currentMetaURITitle NOTIFY sourceChanged)
  Q_PROPERTY(int currentIndex READ currentIndex NOTIFY sourceChanged)
  Q_PROPERTY(int currentTrackDuration READ currentTrackDuration NOTIFY sourceChanged)
  Q_PROPERTY(int currentProtocol READ currentProtocol NOTIFY sourceChanged)
  Q_PROPERTY(bool currentInQueue READ currentInQueue NOTIFY sourceChanged)

  Q_PROPERTY(int numberOfTracks READ numberOfTracks NOTIFY playbackStateChanged)
  Q_PROPERTY(QString playbackState READ playbackState NOTIFY playbackStateChanged)
  Q_PROPERTY(QString playMode READ playMode NOTIFY playModeChanged)

public:
  explicit Player(QObject *parent = nullptr);
  ~Player();

  Q_INVOKABLE bool init(nosonapp::Sonos* sonos, const QString& zoneName);
  Q_INVOKABLE bool init(nosonapp::Sonos* sonos, const QVariant& zone);

  bool init(Sonos* sonos, const SONOS::ZonePtr& zone);
  bool connected() const { return m_connected; }
  QString controllerURI() const { return m_controllerURI; }

  SONOS::ZonePtr zone() const;
  int pid() const { return m_pid; }
  void setPid(int pid) { m_pid = pid; }

  Q_INVOKABLE void enableMPRIS2();
  Q_INVOKABLE void disableMPRIS2();

  Q_INVOKABLE bool ping();
  Q_INVOKABLE QString zoneId() const;
  Q_INVOKABLE QString zoneName() const;
  Q_INVOKABLE QString zoneShortName() const;
  Q_INVOKABLE QString coordinatorName() const;

  ///////////////////////////////////////////////////////////////////////////////
  ///
  /// About futures

  Q_INVOKABLE nosonapp::Future* tryPing();
  Q_INVOKABLE nosonapp::Future* tryConfigureSleepTimer(int seconds);
  Q_INVOKABLE nosonapp::Future* tryRemainingSleepTimerDuration();
  Q_INVOKABLE nosonapp::Future* tryPlay();
  Q_INVOKABLE nosonapp::Future* tryStop();
  Q_INVOKABLE nosonapp::Future* tryPause();
  Q_INVOKABLE nosonapp::Future* tryPrevious();
  Q_INVOKABLE nosonapp::Future* tryNext();
  Q_INVOKABLE nosonapp::Future* tryToggleRepeat();
  Q_INVOKABLE nosonapp::Future* tryToggleShuffle();
  Q_INVOKABLE nosonapp::Future* tryToggleMute();
  Q_INVOKABLE nosonapp::Future* tryToggleMute(const QString& uuid);
  Q_INVOKABLE nosonapp::Future* tryToggleNightmode();
  Q_INVOKABLE nosonapp::Future* tryToggleNightmode(const QString& uuid);
  Q_INVOKABLE nosonapp::Future* tryToggleLoudness();
  Q_INVOKABLE nosonapp::Future* tryToggleLoudness(const QString& uuid);
  Q_INVOKABLE nosonapp::Future* tryToggleOutputFixed(const QString& uuid);
  Q_INVOKABLE nosonapp::Future* tryPlayLineIN();
  Q_INVOKABLE nosonapp::Future* tryPlayDigitalIN();
  Q_INVOKABLE nosonapp::Future* tryPlayQueue(bool start);
  Q_INVOKABLE nosonapp::Future* trySeekTime(int timesec);
  Q_INVOKABLE nosonapp::Future* trySeekTrack(int position);
  Q_INVOKABLE nosonapp::Future* tryAddItemToQueue(const QVariant& payload, int position);
  Q_INVOKABLE nosonapp::Future* tryAddMultipleItemsToQueue(const QVariantList& payloads);
  Q_INVOKABLE nosonapp::Future* tryRemoveAllTracksFromQueue();
  Q_INVOKABLE nosonapp::Future* tryRemoveTrackFromQueue(const QString& id, int containerUpdateID);
  Q_INVOKABLE nosonapp::Future* tryReorderTrackInQueue(int trackNo, int newPosition, int containerUpdateID);
  Q_INVOKABLE nosonapp::Future* trySaveQueue(const QString& title);
  Q_INVOKABLE nosonapp::Future* tryCreateSavedQueue(const QString& title);
  Q_INVOKABLE nosonapp::Future* tryAddItemToSavedQueue(const QString& SQid, const QVariant& payload, int containerUpdateID);
  Q_INVOKABLE nosonapp::Future* tryAddMultipleItemsToSavedQueue(const QString& SQid, const QVariantList& payloads, int containerUpdateID);
  Q_INVOKABLE nosonapp::Future* tryRemoveTracksFromSavedQueue(const QString& SQid, const QVariantList& indexes, int containerUpdateID);
  Q_INVOKABLE nosonapp::Future* tryReorderTrackInSavedQueue(const QString& SQid, int index, int newIndex, int containerUpdateID);
  Q_INVOKABLE nosonapp::Future* tryPlaySource(const QVariant& payload);
  Q_INVOKABLE nosonapp::Future* tryPlayStream(const QString& url, const QString& title);
  Q_INVOKABLE nosonapp::Future* tryPlayFavorite(const QVariant& payload);
  Q_INVOKABLE nosonapp::Future* tryPlayPulse();
  Q_INVOKABLE nosonapp::Future* tryCurrentTrackPosition();

  Q_INVOKABLE nosonapp::Future* trySetTreble(double val);
  Q_INVOKABLE nosonapp::Future* trySetBass(double val);
  Q_INVOKABLE nosonapp::Future* trySetVolumeGroup(double volume);
  Q_INVOKABLE nosonapp::Future* trySetVolume(const QString& uuid, double volume);

  ///////////////////////////////////////////////////////////////////////////////
  ///
  /// Synchonous

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

  Q_INVOKABLE bool toggleNightmode();
  Q_INVOKABLE bool toggleNightmode(const QString& uuid);
  Q_INVOKABLE bool toggleLoudness();
  Q_INVOKABLE bool toggleLoudness(const QString& uuid);
  Q_INVOKABLE bool toggleOutputFixed(const QString& uuid);
  Q_INVOKABLE bool supportsOutputFixed(const QString& uuid);

  Q_INVOKABLE bool playStream(const QString& url, const QString& title);

  Q_INVOKABLE bool playPulse();
  Q_INVOKABLE bool isPulseStream(const QString& url); // return true if the given url is pulse stream from this player

  Q_INVOKABLE bool isMyStream(const QString& url);    // return true if the given url connects to this player

  Q_INVOKABLE QString makeFilePictureURL(const QString& filePath);
  Q_INVOKABLE QString makeFilePictureLocalURL(const QString& filePath);
  Q_INVOKABLE QVariant makeFileStreamItem(const QString& filePath,
                                          const QString& codec,
                                          const QString& title,
                                          const QString& album,
                                          const QString& author,
                                          const QString& duration,
                                          bool hasArt);

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
  Q_INVOKABLE int addMultipleItemsToSavedQueue(const QString& SQid, const QVariantList& payloads, int containerUpdateID);
  Q_INVOKABLE bool removeTracksFromSavedQueue(const QString& SQid, const QVariantList& indexes, int containerUpdateID);
  Q_INVOKABLE bool reorderTrackInSavedQueue(const QString& SQid, int index, int newIndex, int containerUpdateID);

  Q_INVOKABLE bool playFavorite(const QVariant& payload);

  int renderingCount() const { return m_RCTable.size(); }
  bool nightmode() const { return m_RCGroup.nightmode; }
  bool loudness() const { return m_RCGroup.loudness; }
  bool muteMaster() const { return m_RCGroup.mute; }
  bool outputFixed() const { return m_RCGroup.outputFixed; }
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
    bool loudness = false;
    bool outputFixed = false;
    int volume = 0;
    int treble = 0;
    int bass = 0;
    double volumeFake = 0.;
  };

  typedef std::vector<RCProperty> RCTable;

  const RCTable& renderingTable() const { return m_RCTable; }

  Q_INVOKABLE bool setTreble(double val);
  Q_INVOKABLE bool setBass(double val);

  Q_INVOKABLE bool setVolumeGroup(double volume, bool forFake = false);
  Q_INVOKABLE bool setVolume(const QString& uuid, double volume, bool forFake = false);

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
  bool currentInQueue() const { return m_currentProtocol == SONOS::Protocol_xRinconQueue; }
  int numberOfTracks() const { return m_AVTProperty.NumberOfTracks; }
  QString playbackState() const { return QString::fromUtf8(m_AVTProperty.TransportState.c_str()); }
  QString playMode() const { return QString::fromUtf8(m_AVTProperty.CurrentPlayMode.c_str()); }

  // Implements ContentProvider
  void beforeLoad();
  void afterLoad();
  void runContentLoader(ListModel<Player>* model);
  void loadContent(ListModel<Player>* model);
  void loadAllContent();
  void runContentLoaderForContext(ListModel<Player>* model, int id);
  void loadContentForContext(ListModel<Player>* model, int id);
  const char* getHost() const;
  unsigned getPort() const;
  QString getBaseUrl() const;
  void registerContent(ListModel<Player>* model, const QString& root);
  void unregisterContent(ListModel<Player>* model);

signals:
  void connectedChanged(int pid);
  void renderingChanged(int pid);
  void renderingGroupChanged(int pid);
  void renderingCountChanged(int pid);
  void sourceChanged(int pid);
  void playbackStateChanged(int pid);
  void playModeChanged(int pid);
  void sleepTimerChanged(int pid);

private slots:
    void handleTransportChange();
    void handleRenderingControlChange();

private:
  int m_pid;
  Sonos* m_sonos;
  SONOS::PlayerPtr m_player;
  SONOS::AVTProperty m_AVTProperty;

  RCTable m_RCTable; // Rendering control of subordinates
  RCProperty m_RCGroup; // Rendering control of the group

  bool m_connected;
  QString m_controllerURI; // Filled with the public URI of this controller when connected

  QString m_currentMetaAlbum;
  QString m_currentMetaArt;
  QString m_currentMetaArtist;
  QString m_currentMetaSource;
  QString m_currentMetaTitle;
  QString m_currentMetaURITitle;
  int m_currentIndex;
  int m_currentTrackDuration;
  int m_currentProtocol;

  typedef RegisteredContent<Player> ManagedQueue;
  typedef QList<ManagedQueue> ManagedQueueList ;
  Locked<ManagedQueueList> m_managedQueues; // list of managed queues

  bool m_shareIndexInProgress;

  static void playerEventCB(void* handle);

  static ManagedQueueList::iterator findManagedQueue(ManagedQueueList& list, const ListModel<Player>* model);
  static void unregisterContent(ManagedQueue& mq);
  static void unregisterAllContent(ManagedQueueList& list);

  Mpris2* m_mpris2;

  ///////////////////////////////////////////////////////////////////////////////
  ///
  /// About promises

  class PromisePing : public Promise
  {
  public:
    PromisePing(Player& player)
    : m_player(player) { }
    void run() override;
  private:
    Player& m_player;
  };

  class PromiseConfigureSleepTimer : public Promise
  {
  public:
    PromiseConfigureSleepTimer(Player& player, int seconds)
    : m_player(player), m_seconds(seconds) { }
    void run() override;
  private:
    Player& m_player;
    int m_seconds;
  };

  class PromiseRemainingSleepTimerDuration : public Promise
  {
  public:
    PromiseRemainingSleepTimerDuration(Player& player)
    : m_player(player) { }
    void run() override;
  private:
    Player& m_player;
  };

  class PromisePlay : public Promise
  {
  public:
    PromisePlay(Player& player)
    : m_player(player) { }
    void run() override;
  private:
    Player& m_player;
  };

  class PromiseStop : public Promise
  {
  public:
    PromiseStop(Player& player)
    : m_player(player) { }
    void run() override;
  private:
    Player& m_player;
  };

  class PromisePause : public Promise
  {
  public:
    PromisePause(Player& player)
    : m_player(player) { }
    void run() override;
  private:
    Player& m_player;
  };

  class PromisePrevious : public Promise
  {
  public:
    PromisePrevious(Player& player)
    : m_player(player) { }
    void run() override;
  private:
    Player& m_player;
  };

  class PromiseNext : public Promise
  {
  public:
    PromiseNext(Player& player)
    : m_player(player) { }
    void run() override;
  private:
    Player& m_player;
  };

  class PromiseToggleRepeat : public Promise
  {
  public:
    PromiseToggleRepeat(Player& player)
    : m_player(player) { }
    void run() override;
  private:
    Player& m_player;
  };

  class PromiseToggleShuffle : public Promise
  {
  public:
    PromiseToggleShuffle(Player& player)
    : m_player(player) { }
    void run() override;
  private:
    Player& m_player;
  };

  class PromiseToggleMute : public Promise
  {
  public:
    PromiseToggleMute(Player& player)
    : m_player(player) { }
    void run() override;
  private:
    Player& m_player;
  };

  class PromiseToggleMuteUUID : public Promise
  {
  public:
    PromiseToggleMuteUUID(Player& player, const QString& uuid)
    : m_player(player), m_uuid(uuid) { }
    void run() override;
  private:
    Player& m_player;
    const QString m_uuid;
  };

  class PromiseToggleNightmode : public Promise
  {
  public:
    PromiseToggleNightmode(Player& player)
    : m_player(player) { }
    void run() override;
  private:
    Player& m_player;
  };

  class PromiseToggleNightmodeUUID : public Promise
  {
  public:
    PromiseToggleNightmodeUUID(Player& player, const QString& uuid)
    : m_player(player), m_uuid(uuid) { }
    void run() override;
  private:
    Player& m_player;
    const QString m_uuid;
  };

  class PromiseToggleLoudness : public Promise
  {
  public:
    PromiseToggleLoudness(Player& player)
    : m_player(player) { }
    void run() override;
  private:
    Player& m_player;
    const QString m_uuid;
  };

  class PromiseToggleLoudnessUUID : public Promise
  {
  public:
    PromiseToggleLoudnessUUID(Player& player, const QString& uuid)
    : m_player(player), m_uuid(uuid) { }
    void run() override;
  private:
    Player& m_player;
    const QString m_uuid;
  };

  class PromiseToggleOutputFixed : public Promise
  {
  public:
    PromiseToggleOutputFixed(Player& player, const QString& uuid)
    : m_player(player), m_uuid(uuid) { }
    void run() override;
  private:
    Player& m_player;
    const QString m_uuid;
  };

  class PromisePlayLineIN : public Promise
  {
  public:
    PromisePlayLineIN(Player& player)
    : m_player(player) { }
    void run() override;
  private:
    Player& m_player;
  };

  class PromisePlayDigitalIN : public Promise
  {
  public:
    PromisePlayDigitalIN(Player& player)
    : m_player(player) { }
    void run() override;
  private:
    Player& m_player;
  };

  class PromisePlayQueue : public Promise
  {
  public:
    PromisePlayQueue(Player& player, bool start)
    : m_player(player), m_start(start) { }
    void run() override;
  private:
    Player& m_player;
    bool m_start;
  };

  class PromiseSeekTime : public Promise
  {
  public:
    PromiseSeekTime(Player& player, int timesec)
    : m_player(player), m_timesec(timesec) { }
    void run() override;
  private:
    Player& m_player;
    int m_timesec;
  };

  class PromiseSeekTrack : public Promise
  {
  public:
    PromiseSeekTrack(Player& player, int position)
    : m_player(player), m_position(position) { }
    void run() override;
  private:
    Player& m_player;
    int m_position;
  };

  class PromiseAddItemToQueue : public Promise
  {
  public:
    PromiseAddItemToQueue(Player& player, const QVariant& payload, int position)
    : m_player(player), m_payload(payload), m_position(position) { }
    void run() override;
  private:
    Player& m_player;
    const QVariant m_payload;
    int m_position;
  };

  class PromiseAddMultipleItemsToQueue : public Promise
  {
  public:
    PromiseAddMultipleItemsToQueue(Player& player, const QVariantList& payloads)
    : m_player(player), m_payloads(payloads) { }
    void run() override;
  private:
    Player& m_player;
    const QVariantList m_payloads;
  };

  class PromiseRemoveAllTracksFromQueue : public Promise
  {
  public:
    PromiseRemoveAllTracksFromQueue(Player& player)
    : m_player(player) { }
    void run() override;
  private:
    Player& m_player;
  };

  class PromiseRemoveTrackFromQueue : public Promise
  {
  public:
    PromiseRemoveTrackFromQueue(Player& player, const QString& id, int containerUpdateID)
    : m_player(player), m_id(id), m_containerUpdateID(containerUpdateID) { }
    void run() override;
  private:
    Player& m_player;
    const QString m_id;
    int m_containerUpdateID;
  };

  class PromiseReorderTrackInQueue : public Promise
  {
  public:
    PromiseReorderTrackInQueue(Player& player, int trackNo, int newPosition, int containerUpdateID)
    : m_player(player), m_trackNo(trackNo), m_newPosition(newPosition), m_containerUpdateID(containerUpdateID) { }
    void run() override;
  private:
    Player& m_player;
    int m_trackNo;
    int m_newPosition;
    int m_containerUpdateID;
  };

  class PromiseSaveQueue : public Promise
  {
  public:
    PromiseSaveQueue(Player& player, const QString& title)
    : m_player(player), m_title(title) { }
    void run() override;
  private:
    Player& m_player;
    const QString m_title;
  };

  class PromiseCreateSavedQueue : public Promise
  {
  public:
    PromiseCreateSavedQueue(Player& player, const QString& title)
    : m_player(player), m_title(title) { }
    void run() override;
  private:
    Player& m_player;
    const QString m_title;
  };

  class PromiseAddItemToSavedQueue : public Promise
  {
  public:
    PromiseAddItemToSavedQueue(Player& player, const QString& SQid, const QVariant& payload, int containerUpdateID)
    : m_player(player), m_SQid(SQid), m_payload(payload), m_containerUpdateID(containerUpdateID) { }
    void run() override;
  private:
    Player& m_player;
    const QString m_SQid;
    const QVariant m_payload;
    int m_containerUpdateID;
  };

  class PromiseAddMultipleItemsToSavedQueue : public Promise
  {
  public:
    PromiseAddMultipleItemsToSavedQueue(Player& player, const QString& SQid, const QVariantList& payloads, int containerUpdateID)
    : m_player(player), m_SQid(SQid), m_payloads(payloads), m_containerUpdateID(containerUpdateID) { }
    void run() override;
  private:
    Player& m_player;
    const QString m_SQid;
    const QVariantList m_payloads;
    int m_containerUpdateID;
  };

  class PromiseRemoveTracksFromSavedQueue : public Promise
  {
  public:
    PromiseRemoveTracksFromSavedQueue(Player& player, const QString& SQid, const QVariantList& indexes, int containerUpdateID)
    : m_player(player), m_SQid(SQid), m_indexes(indexes), m_containerUpdateID(containerUpdateID) { }
    void run() override;
  private:
    Player& m_player;
    const QString m_SQid;
    const QVariantList m_indexes;
    int m_containerUpdateID;
  };

  class PromiseReorderTrackInSavedQueue : public Promise
  {
  public:
    PromiseReorderTrackInSavedQueue(Player& player, const QString& SQid, int index, int newIndex, int containerUpdateID)
    : m_player(player), m_SQid(SQid), m_index(index), m_newIndex(newIndex), m_containerUpdateID(containerUpdateID) { }
    void run() override;
  private:
    Player& m_player;
    const QString m_SQid;
    int m_index;
    int m_newIndex;
    int m_containerUpdateID;
  };

  class PromisePlaySource : public Promise
  {
  public:
    PromisePlaySource(Player& player, const QVariant& payload)
    : m_player(player), m_payload(payload) { }
    void run() override;
  private:
    Player& m_player;
    QVariant m_payload;
  };

  class PromisePlayStream : public Promise
  {
  public:
    PromisePlayStream(Player& player, const QString& url, const QString& title)
    : m_player(player), m_url(url), m_title(title) { }
    void run() override;
  private:
    Player& m_player;
    const QString m_url;
    const QString m_title;
  };

  class PromisePlayFavorite : public Promise
  {
  public:
    PromisePlayFavorite(Player& player, const QVariant& payload)
    : m_player(player), m_payload(payload) { }
    void run() override;
  private:
    Player& m_player;
    QVariant m_payload;
  };

  class PromisePlayPulse : public Promise
  {
  public:
    PromisePlayPulse(Player& player)
    : m_player(player) { }
    void run() override;
  private:
    Player& m_player;
  };

  class PromiseSetTreble : public Promise
  {
  public:
    PromiseSetTreble(Player& player, double val)
    : m_player(player), m_val(val) { }
    void run() override;
  private:
    Player& m_player;
    double m_val;
  };

  class PromiseSetBass : public Promise
  {
  public:
    PromiseSetBass(Player& player, double val)
    : m_player(player), m_val(val) { }
    void run() override;
  private:
    Player& m_player;
    double m_val;
  };

  class PromiseSetVolumeGroup : public Promise
  {
  public:
    PromiseSetVolumeGroup(Player& player, double volume)
    : m_player(player), m_volume(volume) { }
    void run() override;
  private:
    Player& m_player;
    double m_volume;
  };

  class PromiseSetVolume : public Promise
  {
  public:
    PromiseSetVolume(Player& player, const QString& uuid, double volume)
    : m_player(player), m_uuid(uuid), m_volume(volume) { }
    void run() override;
  private:
    Player& m_player;
    const QString m_uuid;
    double m_volume;
  };

  class PromiseCurrentTrackPosition : public Promise
  {
  public:
    PromiseCurrentTrackPosition(Player& player)
    : m_player(player) { }
    void run() override;
  private:
    Player& m_player;
  };
};

}

#endif // NOSONAPPPLAYER_H
