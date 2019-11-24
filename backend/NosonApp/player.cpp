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

#include "player.h"
#include "sonos.h"
#include "tools.h"
#include "private/debug.h"
#ifdef HAVE_DBUS
#include "dbus/mpris2.h"
#endif

#include <string>
#include <vector>

using namespace nosonapp;

namespace nosonapp
{

class playSourceWorker : public QRunnable
{
public:
  playSourceWorker(Player& player, const QVariant& payload)
  : m_player(player)
  , m_payload(payload)
  { }

  virtual void run() override
  {
    m_player.beginJob();
    if (!m_player.setSource(m_payload) || !m_player.play())
      emit m_player.jobFailed(m_player.pid());
    m_player.endJob();
  }
private:
  Player& m_player;
  QVariant m_payload;
};

class playStreamWorker : public QRunnable
{
public:
  playStreamWorker(Player& player, const QString& url, const QString& title)
  : m_player(player)
  , m_url(url)
  , m_title(title)
  { }

  virtual void run() override
  {
    m_player.beginJob();
    if (!m_player.playStream(m_url, m_title))
      emit m_player.jobFailed(m_player.pid());
    m_player.endJob();
  }
private:
  Player& m_player;
  const QString m_url;
  const QString m_title;
};

class playFavoriteWorker : public QRunnable
{
public:
  playFavoriteWorker(Player& player, const QVariant& payload)
  : m_player(player)
  , m_payload(payload)
  { }

  virtual void run() override
  {
    m_player.beginJob();
    if (!m_player.playFavorite(m_payload))
      emit m_player.jobFailed(m_player.pid());
    m_player.endJob();
  }
private:
  Player& m_player;
  QVariant m_payload;
};

}

Player::Player(QObject *parent)
: QObject(parent)
, m_pid(0)
, m_sonos(nullptr)
, m_player(nullptr)
, m_connected(false)
, m_currentIndex(-1)
, m_currentTrackDuration(0)
, m_currentProtocol(-1)
, m_queue(ManagedQueue(nullptr, ""))
, m_queueUpdateID(0)
, m_mpris2(nullptr)
{
}

Player::~Player()
{
  {
    Locked<ManagedQueue>::pointer queue = m_queue.Get();
    if (queue->model)
    {
      LockGuard g(queue->model->m_lock);
      unregisterContent(queue->model);
    }
  }
  disableMPRIS2();
  m_player.reset();
  m_sonos = nullptr;
}

bool Player::init(Sonos* sonos, const QString& zoneName)
{
  if (sonos)
  {
    SONOS::ZonePtr zone = sonos->findZone(zoneName);
    return init(sonos, zone);
  }
  return false;
}

bool Player::init(Sonos* sonos, const QVariant& zone)
{
  SONOS::ZonePtr _zone = zone.value<SONOS::ZonePtr>();
  return init(sonos, _zone);
}

bool Player::init(Sonos* sonos, const SONOS::ZonePtr& zone)
{
  // Clear the context
  m_connected = false;
  m_player.reset();
  m_RCTable.clear();

  if (sonos && zone)
  {
    m_sonos = sonos;
    m_player = sonos->getSystem().GetPlayer(zone, this, playerEventCB);
    if (m_player)
    {
      handleRenderingControlChange();
      handleTransportChange();
      m_connected = true;
      emit connectedChanged(m_pid);
      return true;
    }
  }

  emit connectedChanged(m_pid);
  return false;
}

void Player::beginJob()
{
  if (m_sonos)
    m_sonos->beginJob();
}

void Player::endJob()
{
  if (m_sonos)
    m_sonos->endJob();
}

SONOS::ZonePtr Player::zone() const
{
  SONOS::PlayerPtr p(m_player);
  if (p)
    return p->GetZone();
  return SONOS::ZonePtr();

}

void Player::enableMPRIS2()
{
#ifdef HAVE_DBUS
  if (!m_mpris2)
    m_mpris2 = new Mpris2(this);
#endif
}

void Player::disableMPRIS2()
{
#ifdef HAVE_DBUS
  if (m_mpris2)
  {
    delete m_mpris2;
    m_mpris2 = nullptr;
  }
#endif
}

bool Player::ping()
{
  SONOS::PlayerPtr p(m_player);
  if (p)
  {
    SONOS::ElementList vars;
    return p->GetMediaInfo(vars);
  }
  return false;
}

QString Player::zoneId() const
{
  SONOS::PlayerPtr p(m_player);
  if (p)
    return p->GetZone()->GetGroup().c_str();
  return QString();
}

QString Player::zoneName() const
{
  SONOS::PlayerPtr p(m_player);
  if (p)
    return QString::fromUtf8(p->GetZone()->GetZoneName().c_str());
  return QString();
}

QString Player::zoneShortName() const
{
  SONOS::PlayerPtr p(m_player);
  if (p)
    return QString::fromUtf8(p->GetZone()->GetZoneShortName().c_str());
  return QString();
}

QString Player::coordinatorName() const
{
  SONOS::PlayerPtr p(m_player);
  if (p)
    return QString::fromUtf8((p->GetZone()->GetCoordinator()->c_str()));
  return QString();
}

bool Player::configureSleepTimer(int seconds)
{
  SONOS::PlayerPtr p(m_player);
  return p ? p->ConfigureSleepTimer(seconds) : false;
}

int Player::remainingSleepTimerDuration()
{
  SONOS::PlayerPtr p(m_player);
  if (p)
  {
    SONOS::ElementList vars;
    p->GetRemainingSleepTimerDuration(vars);
    unsigned hh, hm, hs;
    if (sscanf(vars.GetValue("RemainingSleepTimerDuration").c_str(), "%u:%u:%u", &hh, &hm, &hs) == 3)
      return (int)(hh * 3600 + hm * 60 + hs);
  }
  return 0;
}

bool Player::startPlaySource(const QVariant& payload)
{
  return m_sonos && m_sonos->startJob(new playSourceWorker(*this, payload));
}

bool Player::play()
{
  SONOS::PlayerPtr p(m_player);
  return p ? p->Play() : false;
}

bool Player::stop()
{
  SONOS::PlayerPtr p(m_player);
  return p ? p->Stop() : false;
}

bool Player::pause()
{
  SONOS::PlayerPtr p(m_player);
  return p ? p->Pause() : false;
}

bool Player::previous()
{
  SONOS::PlayerPtr p(m_player);
  return p ? p->Previous() : false;
}

bool Player::next()
{
  SONOS::PlayerPtr p(m_player);
  return p ? p->Next() : false;
}

bool Player::toggleRepeat()
{
  SONOS::PlayerPtr p(m_player);
  if (p)
  {
    if (m_AVTProperty.CurrentPlayMode == "NORMAL")
      return p->SetPlayMode(SONOS::PlayMode_REPEAT_ALL);
    else if (m_AVTProperty.CurrentPlayMode == "REPEAT_ALL")
      return p->SetPlayMode(SONOS::PlayMode_NORMAL);
    else if (m_AVTProperty.CurrentPlayMode == "SHUFFLE")
      return p->SetPlayMode(SONOS::PlayMode_SHUFFLE_NOREPEAT);
    else if (m_AVTProperty.CurrentPlayMode == "SHUFFLE_NOREPEAT")
      return p->SetPlayMode(SONOS::PlayMode_SHUFFLE);
  }
  return false;
}

bool Player::toggleShuffle()
{
  SONOS::PlayerPtr p(m_player);
  if (p)
  {
    if (m_AVTProperty.CurrentPlayMode == "NORMAL")
      return p->SetPlayMode(SONOS::PlayMode_SHUFFLE_NOREPEAT);
    else if (m_AVTProperty.CurrentPlayMode == "REPEAT_ALL")
      return p->SetPlayMode(SONOS::PlayMode_SHUFFLE);
    else if (m_AVTProperty.CurrentPlayMode == "SHUFFLE")
      return p->SetPlayMode(SONOS::PlayMode_REPEAT_ALL);
    else if (m_AVTProperty.CurrentPlayMode == "SHUFFLE_NOREPEAT")
      return p->SetPlayMode(SONOS::PlayMode_NORMAL);
  }
  return false;
}

bool Player::setSource(const QVariant& payload)
{
  SONOS::PlayerPtr p(m_player);
  if (p)
    return p->SetCurrentURI(payload.value<SONOS::DigitalItemPtr>());
  return false;
}

bool Player::toggleMute()
{
  SONOS::PlayerPtr p(m_player);
  if (p)
  {
    bool ret = true;
    bool mute = !m_RCGroup.mute;
    for (RCTable::iterator it = m_RCTable.begin(); it != m_RCTable.end(); ++it)
    {
      if (p->SetMute(it->uuid, mute ? 1 : 0))
        it->mute = mute;
      else
        ret = false;
    }
    if (ret)
      m_RCGroup.mute = mute;
    return ret;
  }
  return false;
}

bool Player::toggleMute(const QString& uuid)
{
  SONOS::PlayerPtr p(m_player);
  if (p)
  {
    std::string _uuid = uuid.toUtf8().constData();
    for (RCTable::iterator it = m_RCTable.begin(); it != m_RCTable.end(); ++it)
    {
      if (it->uuid == _uuid)
      {
        if (p->SetMute(it->uuid, it->mute ? 0 : 1))
          return true;
        return false;
      }
    }
  }
  return false;
}

bool Player::toggleNightmode()
{
  SONOS::PlayerPtr p(m_player);
  if (p)
  {
    bool ret = true;
    bool nightmode = !m_RCGroup.nightmode;
    for (RCTable::iterator it = m_RCTable.begin(); it != m_RCTable.end(); ++it)
    {
      // it could fail when device doesn't support the setting. Anyway force the flag for the group
      if (p->SetNightmode(it->uuid, nightmode ? 1 : 0))
        m_RCGroup.nightmode = it->nightmode = nightmode;
      else
        ret = false;
    }
    return ret;
  }
  return false;
}

bool Player::toggleNightmode(const QString &uuid)
{
  SONOS::PlayerPtr p(m_player);
  if (p)
  {
    std::string _uuid = uuid.toUtf8().constData();
    for (RCTable::iterator it = m_RCTable.begin(); it != m_RCTable.end(); ++it)
    {
      if (it->uuid == _uuid)
      {
        if (p->SetNightmode(it->uuid, it->nightmode ? 0 : 1))
          return true;
        return false;
      }
    }
  }
  return false;
}

bool Player::toggleLoudness()
{
  SONOS::PlayerPtr p(m_player);
  if (p)
  {
    bool ret = true;
    bool loudness = !m_RCGroup.loudness;
    for (RCTable::iterator it = m_RCTable.begin(); it != m_RCTable.end(); ++it)
    {
      // it could fail when device doesn't support the setting. Anyway force the flag for the group
      if (p->SetLoudness(it->uuid, loudness ? 1 : 0))
        m_RCGroup.loudness = it->loudness = loudness;
      else
        ret = false;
    }
    return ret;
  }
  return false;
}

bool Player::toggleLoudness(const QString &uuid)
{
  SONOS::PlayerPtr p(m_player);
  if (p)
  {
    std::string _uuid = uuid.toUtf8().constData();
    for (RCTable::iterator it = m_RCTable.begin(); it != m_RCTable.end(); ++it)
    {
      if (it->uuid == _uuid)
      {
        if (p->SetLoudness(it->uuid, it->loudness ? 0 : 1))
          return true;
        return false;
      }
    }
  }
  return false;
}

bool Player::toggleOutputFixed(const QString &uuid)
{
  SONOS::PlayerPtr p(m_player);
  if (p)
  {
    std::string _uuid = uuid.toUtf8().constData();
    for (RCTable::iterator it = m_RCTable.begin(); it != m_RCTable.end(); ++it)
    {
      if (it->uuid == _uuid)
      {
        if (p->SetOutputFixed(it->uuid, it->outputFixed ? 0 : 1))
          return true;
        return false;
      }
    }
  }
  return false;
}

bool Player::supportsOutputFixed(const QString &uuid)
{
  SONOS::PlayerPtr p(m_player);
  if (p)
  {
    std::string _uuid = uuid.toUtf8().constData();
    for (RCTable::iterator it = m_RCTable.begin(); it != m_RCTable.end(); ++it)
    {
      if (it->uuid == _uuid)
      {
        uint8_t val = 0;
        if (p->GetSupportsOutputFixed(it->uuid, &val))
          return (val == 1);
        return false;
      }
    }
  }
  return false;
}

bool Player::startPlayStream(const QString& url, const QString& title)
{
  return m_sonos && m_sonos->startJob(new playStreamWorker(*this, url, title));
}

bool Player::playStream(const QString& url, const QString& title)
{
  SONOS::PlayerPtr p(m_player);
  if (p)
    return p->PlayStream(url.toUtf8().constData(), title.toUtf8().constData());
  return false;
}

bool Player::playPulse()
{
  SONOS::PlayerPtr p(m_player);
  return p && p->PlayPulse();
}

bool Player::isPulseStream(const QString &url)
{
  SONOS::PlayerPtr p(m_player);
  return p && p->IsPulseStream(url.toUtf8().constData());
}

bool Player::isMyStream(const QString &url)
{
  SONOS::PlayerPtr p(m_player);
  return p && p->IsMyStream(url.toUtf8().constData());
}

QString Player::makeFilePictureURL(const QString& filePath)
{
  SONOS::PlayerPtr p(m_player);
  if (p)
    return QString::fromUtf8(p->MakeFilePictureUrl(filePath.toUtf8().constData()).c_str());
  return QString("");
}

QString Player::makeFilePictureLocalURL(const QString& filePath)
{
  SONOS::PlayerPtr p(m_player);
  if (p)
    return QString::fromUtf8(p->MakeFilePictureLocalUrl(filePath.toUtf8().constData()).c_str());
  return QString("");
}

QVariant Player::makeFileStreamItem(const QString& filePath,
                                    const QString& codec,
                                    const QString& title,
                                    const QString& album,
                                    const QString& author,
                                    const QString& duration,
                                    bool hasArt)
{
  QVariant var;
  SONOS::PlayerPtr p(m_player);
  if (p)
  {
    var.setValue<SONOS::DigitalItemPtr>(p->MakeFileStreamItem(filePath.toUtf8().constData(), codec.toUtf8().constData(),
            title.toUtf8().constData(), album.toUtf8().constData(), author.toUtf8().constData(), duration.toUtf8().constData(),
            hasArt));
  }
  else
    var.setValue<SONOS::DigitalItemPtr>(SONOS::DigitalItemPtr(nullptr));
  return var;
}

bool Player::playLineIN()
{
  SONOS::PlayerPtr p(m_player);
  return p ? p->PlayLineIN() : false;
}

bool Player::playDigitalIN()
{
  SONOS::PlayerPtr p(m_player);
  return p ? p->PlayDigitalIN() : false;
}

bool Player::playQueue(bool start)
{
  SONOS::PlayerPtr p(m_player);
  return p ? p->PlayQueue(start) : false;
}

bool Player::seekTime(int timesec)
{
  SONOS::PlayerPtr p(m_player);
  timesec = timesec < 0 ? 0 : (timesec > 0xffff ? 0xffff : timesec);
  return p ? p->SeekTime(timesec) : false;
}

bool Player::seekTrack(int position)
{
  SONOS::PlayerPtr p(m_player);
  return p ? p->SeekTrack(position) : false;
}

int Player::addItemToQueue(const QVariant& payload, int position)
{
  SONOS::PlayerPtr p(m_player);
  if (p)
    return p->AddURIToQueue(payload.value<SONOS::DigitalItemPtr>(), position);
  return 0;
}

int Player::addMultipleItemsToQueue(const QVariantList& payloads)
{
  SONOS::PlayerPtr p(m_player);
  if (p)
  {
    std::vector<SONOS::DigitalItemPtr> items;
    for (QVariantList::const_iterator it = payloads.begin(); it != payloads.end(); ++it)
      items.push_back(it->value<SONOS::DigitalItemPtr>());
    return p->AddMultipleURIsToQueue(items);
  }
  return 0;
}

bool Player::removeAllTracksFromQueue()
{
  SONOS::PlayerPtr p(m_player);
  if (p && p->RemoveAllTracksFromQueue()) {
    m_currentIndex = -1;
    return true;
  }
  return false;
}

bool Player::removeTrackFromQueue(const QString& id, int containerUpdateID)
{
  SONOS::PlayerPtr p(m_player);
  return p ? p->RemoveTrackFromQueue(id.toUtf8().constData(), containerUpdateID) : false;
}

bool Player::reorderTrackInQueue(int trackNo, int newPosition, int containerUpdateID)
{
  SONOS::PlayerPtr p(m_player);
  return p ? p->ReorderTracksInQueue(trackNo, 1, newPosition, containerUpdateID) : false;
}

bool Player::saveQueue(const QString& title)
{
  SONOS::PlayerPtr p(m_player);
  return p ? p->SaveQueue(title.toUtf8().constData()) : false;
}

bool Player::createSavedQueue(const QString& title)
{
  SONOS::PlayerPtr p(m_player);
  return p ? p->CreateSavedQueue(title.toUtf8().constData()) : false;
}

int Player::addItemToSavedQueue(const QString& SQid, const QVariant& payload, int containerUpdateID)
{
  SONOS::PlayerPtr p(m_player);
  return p ? p->AddURIToSavedQueue(SQid.toUtf8().constData(), payload.value<SONOS::DigitalItemPtr>(), containerUpdateID) : 0;
}

bool Player::removeTracksFromSavedQueue(const QString& SQid, const QVariantList& indexes, int containerUpdateID)
{
  SONOS::PlayerPtr p(m_player);
  if (p)
  {
    QString trackList;
    for (QVariantList::const_iterator it = indexes.begin(); it != indexes.end(); ++it)
    {
      if (it != indexes.begin())
        trackList.append(",");
      trackList.append(QString::number(it->value<int>()));
    }
    return p->ReorderTracksInSavedQueue(SQid.toUtf8().constData(), trackList.toUtf8().constData(), "", containerUpdateID);
  }
  return false;
}

bool Player::reorderTrackInSavedQueue(const QString& SQid, int index, int newIndex, int containerUpdateID)
{
  SONOS::PlayerPtr p(m_player);
  if (p)
  {
    QString trackList = QString::number(index);
    QString newPositionList = QString::number(newIndex);
    return p->ReorderTracksInSavedQueue(SQid.toUtf8().constData(), trackList.toUtf8().constData(), newPositionList.toUtf8().constData(), containerUpdateID);
  }
  return false;
}

bool Player::startPlayFavorite(const QVariant& payload)
{
  return m_sonos && m_sonos->startJob(new playFavoriteWorker(*this, payload));
}

bool Player::playFavorite(const QVariant& payload)
{
  SONOS::DigitalItemPtr favorite(payload.value<SONOS::DigitalItemPtr>());
  SONOS::PlayerPtr p(m_player);
  if (favorite && p)
  {
    SONOS::DigitalItemPtr item;
    if (SONOS::System::ExtractObjectFromFavorite(favorite, item))
    {
      if (SONOS::System::CanQueueItem(item))
      {
        int pos = (m_currentIndex < 0 ? 1 : m_currentIndex + 2);
        return p->PlayQueue(false) && p->AddURIToQueue(item, pos) && p->SeekTrack(pos) && p->Play();
      }
      return p->SetCurrentURI(item) && p->Play();
    }
  }
  return false;
}

bool Player::setTreble(double val)
{
  SONOS::PlayerPtr p(m_player);
  if (p)
  {
    bool ret = true;
    for (RCTable::iterator it = m_RCTable.begin(); it != m_RCTable.end(); ++it)
    {
      if (p->SetTreble(it->uuid, val))
        m_RCGroup.treble = it->treble = val;
      else
        ret = false;
    }
    return ret;
  }
  return false;
}

bool Player::setBass(double val)
{
  SONOS::PlayerPtr p(m_player);
  if (p)
  {
    bool ret = true;
    for (RCTable::iterator it = m_RCTable.begin(); it != m_RCTable.end(); ++it)
    {
      if (p->SetBass(it->uuid, val))
        m_RCGroup.bass = it->bass = val;
      else
        ret = false;
    }
    return ret;
  }
  return false;

}

bool Player::setVolumeGroup(double volume)
{
  SONOS::PlayerPtr p(m_player);
  if (p)
  {
    bool ret = true;
    if (roundDouble(volume) == m_RCGroup.volume)
      return true;
    double r = (volume > 0 ? volume : 1.0);
    if (m_RCGroup.volumeFake > 0.0)
       r /= m_RCGroup.volumeFake;
    for (RCTable::iterator it = m_RCTable.begin(); it != m_RCTable.end(); ++it)
    {
      if (it->outputFixed)
        continue; // output is fixed
      double fake = it->volumeFake * r;
      int v = roundDouble(fake < 1.0 ? 0.0 : fake < 100.0 ? fake : 100.0);
      SONOS::DBG(DBG_DEBUG, "%s: req=%3.3f ratio=%3.3f fake=%3.3f vol=%d\n", __FUNCTION__, volume, r, fake, v);
      if (p->SetVolume(it->uuid, v))
        it->volumeFake = fake;
      else
        ret = false;
    }
    if (ret)
      m_RCGroup.volume = roundDouble(m_RCGroup.volumeFake = volume);
    emit renderingChanged(m_pid);
    return ret;
  }
  return false;
}

bool Player::setVolume(const QString& uuid, double volume)
{
  SONOS::PlayerPtr p(m_player);
  if (p)
  {
    double fake = 0.0;
    std::string _uuid = uuid.toUtf8().constData();
    size_t count = m_RCTable.size();
    for (RCTable::iterator it = m_RCTable.begin(); it != m_RCTable.end(); ++it)
    {
      if (it->outputFixed)
        --count; // remove it from the count
      else
      {
        if (it->uuid == _uuid)
        {
          int v = roundDouble(volume);
          if (!p->SetVolume(it->uuid, v))
            return false;
          it->volumeFake = (v == 0 ? 100.0 / 101.0 : volume);
          it->volume = v;
        }
        fake += it->volumeFake;
      }
    }
    if (count == 0)
      fake = 100.0; // all output fixed
    else
      fake /= count;
    m_RCGroup.volume = roundDouble(m_RCGroup.volumeFake = fake);
    emit renderingGroupChanged(m_pid);
    return true;
  }
  return false;
}

int Player::currentTrackPosition()
{
  SONOS::PlayerPtr p(m_player);
  if (p)
  {
    SONOS::ElementList vars;
    p->GetPositionInfo(vars);
    unsigned hh, hm, hs;
    if (sscanf(vars.GetValue("RelTime").c_str(), "%u:%u:%u", &hh, &hm, &hs) == 3)
      return (int)(hh * 3600 + hm * 60 + hs);
  }
  return 0;
}

void Player::setCurrentMeta(const SONOS::AVTProperty& prop)
{
  // reset current
  m_currentMetaAlbum = "";
  m_currentMetaArt = "";
  m_currentMetaArtist = "";
  m_currentMetaSource = "";
  m_currentMetaTitle = "";
  m_currentMetaURITitle = "";
  m_currentIndex = -1;
  m_currentTrackDuration = 0;
  m_currentProtocol = SONOS::Protocol_unknown;

  SONOS::PlayerPtr p(m_player);
  if (p)
  {
    QString port;
    port.setNum(p->GetPort());
    QString playerUrl = "http://";
    playerUrl.append(p->GetHost().c_str()).append(":").append(port);

    // Set the protocol
    m_currentProtocol = p->GetURIProtocol(prop.CurrentTrackURI);

    // Set the source (URI)
    m_currentMetaSource = QString::fromUtf8(prop.CurrentTrackURI.c_str());

    // Set the URI title from 'r_EnqueuedTransportURIMetaData'. Also it is the default title.
    if (prop.r_EnqueuedTransportURIMetaData)
      m_currentMetaURITitle = QString::fromUtf8(prop.r_EnqueuedTransportURIMetaData->GetValue("dc:title").c_str());

    // Set the duration for a track. It is zero for a stream.
    unsigned hh, hm, hs;
    if (sscanf(prop.CurrentTrackDuration.c_str(), "%u:%u:%u", &hh, &hm, &hs) == 3)
      m_currentTrackDuration = static_cast<int>(hh * 3600 + hm * 60 + hs);

    QString artUri;
    // Handle stream. Postulate: duration is zero
    if (m_currentTrackDuration == 0)
    {
      if (prop.CurrentTrackMetaData)
      {
        artUri = QString::fromUtf8(prop.CurrentTrackMetaData->GetValue("upnp:albumArtURI").c_str());
        // On the transitionning state, use the title from the metadata of transport
        if (prop.TransportState.compare("TRANSITIONING") == 0)
        {
          m_currentMetaTitle = m_currentMetaURITitle;
        }
        else
        {
          m_currentMetaTitle = m_currentMetaURITitle;
          std::string content = prop.CurrentTrackMetaData->GetValue("r:streamContent");
          if (content.empty())
          {
            const std::string& radioShowMd = prop.CurrentTrackMetaData->GetValue("r:radioShowMd");
            content = radioShowMd.substr(0, radioShowMd.find_last_of(","));
          }
          m_currentMetaArtist = QString::fromUtf8(content.c_str());
        }
      }
    }
    // Handle track
    else if (prop.CurrentTrackMetaData)
    {
      // file
      artUri = QString::fromUtf8(prop.CurrentTrackMetaData->GetValue("upnp:albumArtURI").c_str());
      m_currentMetaTitle = QString::fromUtf8(prop.CurrentTrackMetaData->GetValue("dc:title").c_str());
      m_currentMetaAlbum = QString::fromUtf8(prop.CurrentTrackMetaData->GetValue("upnp:album").c_str());
      m_currentMetaArtist = QString::fromUtf8(prop.CurrentTrackMetaData->GetValue("dc:creator").c_str());
      m_currentIndex = static_cast<int>(prop.CurrentTrack) - 1; // playing queue
    }

    if (artUri.startsWith("/"))
      m_currentMetaArt = playerUrl + artUri;
    else
      m_currentMetaArt = artUri;
  }
}

void Player::beforeLoad()
{
  if (m_sonos)
    m_sonos->beginJob();
}

void Player::afterLoad()
{
  if (m_sonos)
    m_sonos->endJob();
}

void Player::runContentLoader(ListModel<Player>* model)
{
  if (model && !model->m_pending && m_sonos)
  {
    model->m_pending = true; // decline next request
    m_sonos->startJob(new ContentLoader<Player>(*this, model));
  }
  else
    SONOS::DBG(DBG_ERROR, "%s: request has been declined (%p)\n", __FUNCTION__, model);
}

void Player::loadContent(ListModel<Player>* model)
{
  SONOS::DBG(DBG_DEBUG, "%s: %p (%s)\n", __FUNCTION__, model, model->m_root.toUtf8().constData());
  //emit loadingStarted();
  model->m_pending = false; // accept add next request in queue
  model->loadData();
  //emit loadingFinished();
}

void Player::loadAllContent()
{
  ListModel<Player>* model = m_queue.Get()->model;
  if (model && model->m_dataState == DataStatus::DataNotFound)
  {
    //emit loadingStarted();
    model->loadData();
    //emit loadingFinished();
  }
}

void Player::runContentLoaderForContext(ListModel<Player>* model, int id)
{
  if (model && !model->m_pending && m_sonos)
  {
    model->m_pending = true; // decline next request
    m_sonos->startJob(new ContentForContextLoader<Player>(*this, model, id));
  }
  else
    SONOS::DBG(DBG_ERROR, "%s: request id %d has been declined (%p)\n", __FUNCTION__, id, model);
}

void Player::loadContentForContext(ListModel<Player>* model, int id)
{
  model->m_pending = false; // accept add next request in queue
  model->loadDataForContext(id);
}

const char* Player::getHost() const
{
  SONOS::PlayerPtr p(m_player);
  return p->GetHost().c_str();
}

unsigned Player::getPort() const
{
  SONOS::PlayerPtr p(m_player);
  return p->GetPort();
}

QString Player::getBaseUrl() const
{
  SONOS::PlayerPtr p(m_player);
  QString port;
  port.setNum(p->GetPort());
  QString url = "http://";
  url.append(p->GetHost().c_str()).append(":").append(port);
  return url;
}

void Player::registerContent(ListModel<Player>* model, const QString& root)
{
  if (model)
  {
    SONOS::DBG(DBG_DEBUG, "%s: %p (%s)\n", __FUNCTION__, model, model->m_root.toUtf8().constData());
    Locked<ManagedQueue>::pointer rc = m_queue.Get();
    if (rc->model == model)
    {
      rc->root = root;
      return;
    }
    if (rc->model)
      rc->model->m_provider = nullptr;
    rc->model = model;
    rc->root = root;
  }
}

void Player::unregisterContent(ListModel<Player>* model)
{
  if (model)
  {
    SONOS::DBG(DBG_DEBUG, "%s: %p (%s)\n", __FUNCTION__, model, model->m_root.toUtf8().constData());
    Locked<ManagedQueue>::pointer rc = m_queue.Get();
    if (rc->model == model)
    {
      rc->model->m_provider = nullptr;
      rc->model = nullptr;
      rc->root.clear();
    }
  }
}

void Player::handleTransportChange()
{
  SONOS::PlayerPtr p(m_player);
  if (p)
  {
#define AVTRANSPORT_UNCHANGED           0
#define AVTRANSPORT_STATE_CHANGED       1
#define AVTRANSPORT_PLAYMODE_CHANGED    2
#define AVTRANSPORT_SLEEPTIMER_CHANGED  4
    unsigned signalMask = AVTRANSPORT_UNCHANGED;

    SONOS::AVTProperty prop = p->GetTransportProperty();

    setCurrentMeta(prop);
    emit sourceChanged(m_pid);

    if (prop.TransportState != m_AVTProperty.TransportState)
      signalMask |= AVTRANSPORT_STATE_CHANGED;

    if (prop.CurrentPlayMode != m_AVTProperty.CurrentPlayMode)
      signalMask |= AVTRANSPORT_PLAYMODE_CHANGED;

    if (prop.r_SleepTimerGeneration != m_AVTProperty.r_SleepTimerGeneration)
      signalMask |= AVTRANSPORT_SLEEPTIMER_CHANGED;

    // Set property before emit
    m_AVTProperty = prop;
    if (signalMask & AVTRANSPORT_STATE_CHANGED)
      emit playbackStateChanged(m_pid);
    if (signalMask & AVTRANSPORT_PLAYMODE_CHANGED)
      emit playModeChanged(m_pid);
    if (signalMask & AVTRANSPORT_SLEEPTIMER_CHANGED)
      emit sleepTimerChanged(m_pid);
  }
}

void Player::handleRenderingControlChange()
{
  SONOS::PlayerPtr p(m_player);
  if (p)
  {
#define RENDERING_UNCHANGED     0
#define RENDERING_GROUP_CHANGED 1
#define RENDERING_CHANGED       2
    unsigned signalMask = RENDERING_UNCHANGED;

    SONOS::SRPList props = p->GetRenderingProperty();
    size_t count = props.size(); // the count of renderer
    // At initial callback the table of properties for the connected zone is empty
    // So fill it for each subordinate attached to the zone
    if (m_RCTable.empty())
    {
      double volume = 0.0;
      bool mute = true;
      bool outputFixed = true;
      SONOS::SRPList::const_iterator it = props.begin();
      while (it != props.end())
      {
        RCProperty item;
        item.uuid = it->uuid;
        item.name = it->subordinateName;
        item.mute = it->property.MuteMaster != 0 ? true : false;
        item.nightmode = it->property.NightMode != 0 ? true : false;
        item.loudness = it->property.LoudnessMaster != 0 ? true : false;
        item.outputFixed = it->property.OutputFixed != 0 ? true : false;
        item.volume = it->property.VolumeMaster;
        item.volumeFake = it->property.VolumeMaster > 0 ? (double)it->property.VolumeMaster : 100.0 / 101.0;
        item.treble = it->property.Treble;
        item.bass = it->property.Bass;
        m_RCTable.push_back(item);
        if (!item.mute)
          mute = false; // exists active audio in group
        if (item.outputFixed)
          --count; // remove it from the count
        else
        {
          outputFixed = false; // exists ajustable output in group
          volume += item.volumeFake;
        }
        // As the sound settings relate the connected group and doesn't handle separately each subordinate,
        // i gatherer data only from the master zone that is the first on the array.
        if (it == props.begin())
        {
          m_RCGroup.nightmode = item.nightmode;
          m_RCGroup.loudness = item.loudness;
          m_RCGroup.treble = item.treble;
          m_RCGroup.bass = item.bass;
        }

        ++it;
      }

      if (count == 0)
        volume = 100.0;
      else
        volume /= (double)count;

      m_RCGroup.volumeFake = volume;
      m_RCGroup.volume = roundDouble(volume);
      m_RCGroup.mute = mute;
      m_RCGroup.outputFixed = outputFixed;
      signalMask |= RENDERING_GROUP_CHANGED | RENDERING_CHANGED; // handles group & subordinate update
    }
    else
    {
      double volume = 0.0;
      bool mute = true;
      bool outputFixed = true;
      bool nightmode = false;
      bool loudness = false;
      int treble = 0, bass = 0;
      SONOS::SRPList::const_iterator it = props.begin();
      std::vector<RCProperty>::iterator itz = m_RCTable.begin();
      while (it != props.end())
      {
        // cast to bool before comparing
        bool MuteMasterAsBool = it->property.MuteMaster != 0 ? true : false;
        bool NightModeAsBool = it->property.NightMode != 0 ? true : false;
        bool LoudnessAsBool = it->property.LoudnessMaster != 0 ? true : false;
        bool OutputFixedAsBool = it->property.OutputFixed != 0 ? true : false;

        if (MuteMasterAsBool != itz->mute)
        {
          itz->mute = MuteMasterAsBool;
          signalMask |= RENDERING_CHANGED;
        }
        if (NightModeAsBool != itz->nightmode)
        {
          itz->nightmode = NightModeAsBool;
          signalMask |= RENDERING_CHANGED;
        }
        if (LoudnessAsBool != itz->loudness)
        {
          itz->loudness = LoudnessAsBool;
          signalMask |= RENDERING_CHANGED;
        }
        if (OutputFixedAsBool != itz->outputFixed)
        {
          itz->outputFixed = OutputFixedAsBool;
          signalMask |= RENDERING_CHANGED;
        }
        if (it->property.Treble != itz->treble)
        {
          itz->treble = it->property.Treble;
          signalMask |= RENDERING_CHANGED;
        }
        if (it->property.Bass != itz->bass)
        {
          itz->bass = it->property.Bass;
          signalMask |= RENDERING_CHANGED;
        }

        // As the sound settings relate the connected group and doesn't handle separately each subordinate,
        // i gatherer data from the master zone that is the first on the array.
        if (it == props.begin())
        {
          nightmode = NightModeAsBool;
          loudness = LoudnessAsBool;
          treble = it->property.Treble;
          bass = it->property.Bass;
        }
        // And override data from the first subordinate updated accordinaly with sended values
        else
        {
          if (NightModeAsBool == m_RCGroup.nightmode)
            nightmode = NightModeAsBool;
          if (LoudnessAsBool == m_RCGroup.loudness)
            loudness = LoudnessAsBool;
          if (it->property.Treble == m_RCGroup.treble)
            treble = it->property.Treble;
          if (it->property.Bass == m_RCGroup.bass)
            bass = it->property.Bass;
        }

        if (it->property.VolumeMaster != itz->volume)
        {
          itz->volume = it->property.VolumeMaster;

          if (it->property.VolumeMaster > 0 && it->property.VolumeMaster < 100 &&
                  it->property.VolumeMaster != roundDouble(itz->volumeFake))
          {
            itz->volumeFake = (double)it->property.VolumeMaster;
            signalMask |= RENDERING_CHANGED;
          }
          else if (it->property.VolumeMaster == 100 && itz->volumeFake < 100.0)
          {
            itz->volumeFake = (double)it->property.VolumeMaster;
            signalMask |= RENDERING_CHANGED;
          }
          else if (it->property.VolumeMaster == 0 && itz->volumeFake >= 1.0)
          {
            itz->volumeFake = 100.0 / 101.0;
            signalMask |= RENDERING_CHANGED;
          }
        }
        SONOS::DBG(DBG_DEBUG, "%s: [%s] sig=%d volume: %3.3f [%d]\n", __FUNCTION__, it->uuid.c_str(), signalMask, itz->volumeFake, itz->volume);
        if (!itz->mute)
          mute = false; // exists active audio in group
        if (!itz->outputFixed)
          outputFixed = false; // exists ajustable output in group

        if (itz->outputFixed)
          --count; // remove it from the count
        else
          volume += itz->volumeFake;

        ++it;
        ++itz;
      }

      if (count == 0)
        volume = 100.0;
      else
        volume /= (double)count;

      m_RCGroup.volumeFake = volume;
      m_RCGroup.volume = roundDouble(volume);
      m_RCGroup.mute = mute;
      m_RCGroup.nightmode = nightmode;
      m_RCGroup.loudness = loudness;
      m_RCGroup.treble = treble;
      m_RCGroup.bass = bass;
      m_RCGroup.outputFixed = outputFixed;
      signalMask |= RENDERING_GROUP_CHANGED; // handles group update
    }

    SONOS::DBG(DBG_DEBUG, "%s: sig=%d volume: %3.3f [%d]\n", __FUNCTION__, signalMask, m_RCGroup.volumeFake, m_RCGroup.volume);

    // emit signal about changes
    if (signalMask & RENDERING_GROUP_CHANGED)
      emit renderingGroupChanged(m_pid);
    if (signalMask & RENDERING_CHANGED)
      emit renderingChanged(m_pid);
  }
}

void Player::playerEventCB(void* handle)
{
  Player* player = static_cast<Player*>(handle);
  Q_ASSERT(player);
  SONOS::PlayerPtr p(player->m_player);
  if (p)
  {
    // Read last event flags
    unsigned char events = p->LastEvents();

    if ((events & SONOS::SVCEvent_TransportChanged))
      player->handleTransportChange();
    if ((events & SONOS::SVCEvent_RenderingControlChanged))
      player->handleRenderingControlChange();

    if ((events & SONOS::SVCEvent_ContentDirectoryChanged))
    {
      Locked<ManagedQueue>::pointer cl = player->m_queue.Get();
      // find the base of the model from its root
      if (cl->model)
      {
        QString _base;
        int slash = cl->model->m_root.indexOf("/");
        if (slash < 0)
          _base.append(cl->model->m_root);
        else
          _base.append(cl->model->m_root.left(slash));

        SONOS::ContentProperty prop = p->GetContentProperty();
        for (std::vector<std::pair<std::string, unsigned> >::const_iterator uit = prop.ContainerUpdateIDs.begin(); uit != prop.ContainerUpdateIDs.end(); ++uit)
        {
          SONOS::DBG(DBG_DEBUG, "%s: container [%s] has being updated to %u\n", __FUNCTION__, uit->first.c_str(), uit->second);
          // same base
          if (cl->model->m_updateID != uit->second && _base == uit->first.c_str())
            cl->model->handleDataUpdate();
        }
      }
    }
  }
}
