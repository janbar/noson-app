/*
 *      Copyright (C) 2015-2016 Jean-Luc Barriere
 *
 *  This file is part of Noson-App
 *
 *  Noson-App is free software: you can redistribute it and/or modify
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
 *  along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "player.h"
#include "sonos.h"
#include "tools.h"
#include "private/debug.h"

#include <cstdio> // for sscanf
#include <string>
#include <vector>

Player::Player(QObject *parent)
: QObject(parent)
, m_sonos(0)
, m_player(0)
, m_connected(false)
, m_currentIndex(-1)
, m_currentTrackDuration(0)
, m_currentProtocol(-1)
{
}

Player::~Player()
{
  disconnectSonos(m_sonos);
  m_sonos = 0;
}

bool Player::init(QObject* sonos)
{
  Sonos* _sonos = reinterpret_cast<Sonos*>(sonos);
  if (_sonos)
  {
    disconnectSonos(m_sonos);
    m_sonos = _sonos;
    m_player = _sonos->getPlayer();
    if (m_player)
    {
      connectSonos(m_sonos);

      // Clear context from the new player
      m_RCTable.clear();
      if (!m_player->RenderingPropertyEmpty())
        handleRenderingControlChange();
      if (!m_player->TransportPropertyEmpty())
        handleTransportChange();

      m_connected = true;
      emit connectedChanged();
      return true;
    }
  }
  m_connected = false;
  emit connectedChanged();
  return false;
}

void Player::beginJob()
{
  m_sonos->beginJob();
}

void Player::endJob()
{
  m_sonos->endJob();
}

void Player::renewSubscriptions()
{
  if (m_player)
    m_player->RenewSubscriptions();
}

bool Player::ping()
{
  if (m_player)
  {
    SONOS::ElementList vars;
    return m_player->GetMediaInfo(vars);
  }
  return false;
}

bool Player::refreshShareIndex()
{
  return m_player ? m_player->RefreshShareIndex() : false;
}

bool Player::configureSleepTimer(int seconds)
{
  return m_player ? m_player->ConfigureSleepTimer(seconds) : false;
}

int Player::remainingSleepTimerDuration()
{
  if (m_player)
  {
    SONOS::ElementList vars;
    m_player->GetRemainingSleepTimerDuration(vars);
    unsigned hh, hm, hs;
    if (sscanf(vars.GetValue("RemainingSleepTimerDuration").c_str(), "%u:%u:%u", &hh, &hm, &hs) == 3)
      return (int)(hh * 3600 + hm * 60 + hs);
  }
  return 0;
}

class playSourceWorker : public SONOS::OS::CWorker
{
public:
  playSourceWorker(Player& player, const QVariant& payload)
  : m_player(player)
  , m_payload(payload)
  { }

  virtual void Process()
  {
    m_player.beginJob();
    if (!m_player.setSource(m_payload) || !m_player.play())
      emit m_player.jobFailed();
    m_player.endJob();
  }
private:
  Player& m_player;
  QVariant m_payload;
};

bool Player::startPlaySource(const QVariant& payload)
{
  return m_sonos->startJob(new playSourceWorker(*this, payload));
}

bool Player::play()
{
  return m_player ? m_player->Play() : false;
}

bool Player::stop()
{
  return m_player ? m_player->Stop() : false;
}

bool Player::pause()
{
  return m_player ? m_player->Pause() : false;
}

bool Player::previous()
{
  return m_player ? m_player->Previous() : false;
}

bool Player::next()
{
  return m_player ? m_player->Next() : false;
}

bool Player::toggleRepeat()
{
  if (m_player)
  {
    if (m_AVTProperty.CurrentPlayMode == "NORMAL")
      return m_player->SetPlayMode(SONOS::PlayMode_REPEAT_ALL);
    else if (m_AVTProperty.CurrentPlayMode == "REPEAT_ALL")
      return m_player->SetPlayMode(SONOS::PlayMode_NORMAL);
    else if (m_AVTProperty.CurrentPlayMode == "SHUFFLE")
      return m_player->SetPlayMode(SONOS::PlayMode_SHUFFLE_NOREPEAT);
    else if (m_AVTProperty.CurrentPlayMode == "SHUFFLE_NOREPEAT")
      return m_player->SetPlayMode(SONOS::PlayMode_SHUFFLE);
  }
  return false;
}

bool Player::toggleShuffle()
{
  if (m_player)
  {
    if (m_AVTProperty.CurrentPlayMode == "NORMAL")
      return m_player->SetPlayMode(SONOS::PlayMode_SHUFFLE_NOREPEAT);
    else if (m_AVTProperty.CurrentPlayMode == "REPEAT_ALL")
      return m_player->SetPlayMode(SONOS::PlayMode_SHUFFLE);
    else if (m_AVTProperty.CurrentPlayMode == "SHUFFLE")
      return m_player->SetPlayMode(SONOS::PlayMode_REPEAT_ALL);
    else if (m_AVTProperty.CurrentPlayMode == "SHUFFLE_NOREPEAT")
      return m_player->SetPlayMode(SONOS::PlayMode_NORMAL);
  }
  return false;
}

bool Player::setSource(const QVariant& payload)
{
  if (m_player)
    return m_player->SetCurrentURI(payload.value<SONOS::DigitalItemPtr>());
  return false;
}

bool Player::toggleMute()
{
  if (m_player)
  {
    bool ret = true;
    bool mute = !m_RCGroup.mute;
    for (RCTable::iterator it = m_RCTable.begin(); it != m_RCTable.end(); ++it)
    {
      if (m_player->SetMute(it->uuid, mute ? 1 : 0))
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
  if (m_player)
  {
    std::string _uuid = uuid.toUtf8().constData();
    for (RCTable::iterator it = m_RCTable.begin(); it != m_RCTable.end(); ++it)
    {
      if (it->uuid == _uuid)
      {
        bool mute = !it->mute;
        if (m_player->SetMute(it->uuid, mute ? 1 : 0))
        {
          it->mute = !mute;
          return true;
        }
        return false;
      }
    }
  }
  return false;
}

bool Player::toggleNightmode()
{
  if (m_player)
  {
    bool ret = true;
    bool nightmode = !m_RCGroup.nightmode;
    for (RCTable::iterator it = m_RCTable.begin(); it != m_RCTable.end(); ++it)
    {
      if (m_player->SetNightmode(it->uuid, nightmode ? 1 : 0))
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
  if (m_player)
  {
    std::string _uuid = uuid.toUtf8().constData();
    for (RCTable::iterator it = m_RCTable.begin(); it != m_RCTable.end(); ++it)
    {
      if (it->uuid == _uuid)
      {
        bool nightmode = !it->nightmode;
        if (m_player->SetNightmode(it->uuid, nightmode ? 1 : 0))
        {
          it->nightmode = !nightmode;
          return true;
        }
        return false;
      }
    }
  }
  return false;
}

class playStreamWorker : public SONOS::OS::CWorker
{
public:
  playStreamWorker(Player& player, const QString& url, const QString& title)
  : m_player(player)
  , m_url(url)
  , m_title(title)
  { }

  virtual void Process()
  {
    m_player.beginJob();
    if (!m_player.playStream(m_url, m_title))
      emit m_player.jobFailed();
    m_player.endJob();
  }
private:
  Player& m_player;
  const QString m_url;
  const QString m_title;
};

bool Player::startPlayStream(const QString& url, const QString& title)
{
  return m_sonos->startJob(new playStreamWorker(*this, url, title));
}

bool Player::playStream(const QString& url, const QString& title)
{
  if (m_player)
    return m_player->PlayStream(url.toUtf8().constData(), title.toUtf8().constData());
  return false;
}

bool Player::playLineIN()
{
  return m_player ? m_player->PlayLineIN() : false;
}

bool Player::playDigitalIN()
{
  return m_player ? m_player->PlayDigitalIN() : false;
}

bool Player::playQueue(bool start)
{
  return m_player ? m_player->PlayQueue(start) : false;
}

bool Player::seekTime(int timesec)
{
  timesec = timesec < 0 ? 0 : (timesec > 0xffff ? 0xffff : timesec);
  return m_player ? m_player->SeekTime(timesec) : false;
}

bool Player::seekTrack(int position)
{
  return m_player ? m_player->SeekTrack(position) : false;
}

int Player::addItemToQueue(const QVariant& payload, int position)
{
  if (m_player)
    return m_player->AddURIToQueue(payload.value<SONOS::DigitalItemPtr>(), position);
  return 0;
}

int Player::addMultipleItemsToQueue(const QVariantList& payloads)
{
  if (m_player)
  {
    std::vector<SONOS::DigitalItemPtr> items;
    for (QVariantList::const_iterator it = payloads.begin(); it != payloads.end(); ++it)
      items.push_back(it->value<SONOS::DigitalItemPtr>());
    return m_player->AddMultipleURIsToQueue(items);
  }
  return 0;
}

bool Player::removeAllTracksFromQueue()
{
  if (m_player && m_player->RemoveAllTracksFromQueue()) {
    m_currentIndex = -1;
    return true;
  }
  return false;
}

bool Player::removeTrackFromQueue(const QString& id, int containerUpdateID)
{
  return m_player ? m_player->RemoveTrackFromQueue(id.toUtf8().constData(), containerUpdateID) : false;
}

bool Player::reorderTrackInQueue(int trackNo, int newPosition, int containerUpdateID)
{
  return m_player ? m_player->ReorderTracksInQueue(trackNo, 1, newPosition, containerUpdateID) : false;
}

bool Player::saveQueue(const QString& title)
{
  return m_player ? m_player->SaveQueue(title.toUtf8().constData()) : false;
}

bool Player::createSavedQueue(const QString& title)
{
  return m_player ? m_player->CreateSavedQueue(title.toUtf8().constData()) : false;
}

int Player::addItemToSavedQueue(const QString& SQid, const QVariant& payload, int containerUpdateID)
{
  return m_player ? m_player->AddURIToSavedQueue(SQid.toUtf8().constData(), payload.value<SONOS::DigitalItemPtr>(), containerUpdateID) : 0;
}

bool Player::removeTracksFromSavedQueue(const QString& SQid, const QVariantList& indexes, int containerUpdateID)
{
  if (m_player)
  {
    QString trackList;
    for (QVariantList::const_iterator it = indexes.begin(); it != indexes.end(); ++it)
    {
      if (it != indexes.begin())
        trackList.append(",");
      trackList.append(QString::number(it->value<int>()));
    }
    return m_player->ReorderTracksInSavedQueue(SQid.toUtf8().constData(), trackList.toUtf8().constData(), "", containerUpdateID);
  }
  return false;
}

bool Player::reorderTrackInSavedQueue(const QString& SQid, int index, int newIndex, int containerUpdateID)
{
  if (m_player)
  {
    QString trackList = QString::number(index);
    QString newPositionList = QString::number(newIndex);
    return m_player->ReorderTracksInSavedQueue(SQid.toUtf8().constData(), trackList.toUtf8().constData(), newPositionList.toUtf8().constData(), containerUpdateID);
  }
  return false;
}

bool Player::destroySavedQueue(const QString& SQid)
{
  return m_player ? m_player->DestroySavedQueue(SQid.toUtf8().constData()) : false;
}

bool Player::addItemToFavorites(const QVariant& payload, const QString& description, const QString& artURI)
{
  return m_player ? m_player->AddURIToFavorites(payload.value<SONOS::DigitalItemPtr>(), description.toUtf8().constData(), artURI.toUtf8().constData()) : false;
}

bool Player::destroyFavorite(const QString& FVid)
{
  return m_player ? m_player->DestroyFavorite(FVid.toUtf8().constData()) : false;
}

class playFavoriteWorker : public SONOS::OS::CWorker
{
public:
  playFavoriteWorker(Player& player, const QVariant& payload)
  : m_player(player)
  , m_payload(payload)
  { }

  virtual void Process()
  {
    m_player.beginJob();
    if (!m_player.playFavorite(m_payload))
      emit m_player.jobFailed();
    m_player.endJob();
  }
private:
  Player& m_player;
  QVariant m_payload;
};

bool Player::startPlayFavorite(const QVariant& payload)
{
  return m_sonos->startJob(new playFavoriteWorker(*this, payload));
}

bool Player::playFavorite(const QVariant& payload)
{
  SONOS::DigitalItemPtr favorite(payload.value<SONOS::DigitalItemPtr>());
  SONOS::PlayerPtr player(m_player);
  if (favorite && player)
  {
    SONOS::DigitalItemPtr item;
    if (SONOS::System::ExtractObjectFromFavorite(favorite, item))
    {
      if (SONOS::System::CanQueueItem(item))
      {
        int pos = (m_currentIndex < 0 ? 1 : m_currentIndex + 2);
        return m_player->PlayQueue(false) && m_player->AddURIToQueue(item, pos) && m_player->SeekTrack(pos) && m_player->Play();
      }
      return m_player->SetCurrentURI(item) && m_player->Play();
    }
  }
  return false;
}

bool Player::setTreble(double val)
{
  if (m_player)
  {
    bool ret = true;
    for (RCTable::iterator it = m_RCTable.begin(); it != m_RCTable.end(); ++it)
    {
      if (m_player->SetTreble(it->uuid, val))
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
  if (m_player)
  {
    bool ret = true;
    for (RCTable::iterator it = m_RCTable.begin(); it != m_RCTable.end(); ++it)
    {
      if (m_player->SetBass(it->uuid, val))
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
  if (m_player)
  {
    bool ret = true;
    if (roundDouble(volume) == m_RCGroup.volume)
      return true;
    double r = (volume > 0 ? volume : 1.0);
    if (m_RCGroup.volumeFake > 0.0)
       r /= m_RCGroup.volumeFake;
    for (RCTable::iterator it = m_RCTable.begin(); it != m_RCTable.end(); ++it)
    {
      double fake = it->volumeFake * r;
      int v = roundDouble(fake < 1.0 ? 0.0 : fake < 100.0 ? fake : 100.0);
      SONOS::DBG(DBG_DEBUG, "%s: req=%3.3f ratio=%3.3f fake=%3.3f vol=%d\n", __FUNCTION__, volume, r, fake, v);
      if (m_player->SetVolume(it->uuid, v))
        it->volumeFake = fake;
      else
        ret = false;
    }
    if (ret)
      m_RCGroup.volume = roundDouble(m_RCGroup.volumeFake = volume);
    emit renderingChanged();
    return ret;
  }
  return false;
}

bool Player::setVolume(const QString& uuid, double volume)
{
  if (m_player)
  {
    double fake = 0.0;
    std::string _uuid = uuid.toUtf8().constData();
    for (RCTable::iterator it = m_RCTable.begin(); it != m_RCTable.end(); ++it)
    {
      if (it->uuid == _uuid)
      {
        int v = roundDouble(volume);
        if (!m_player->SetVolume(it->uuid, v))
          return false;
        it->volumeFake = (v == 0 ? 100.0 / 101.0 : volume);
        it->volume = v;
      }
      fake += it->volumeFake;
    }
    fake /= m_RCTable.size();
    m_RCGroup.volume = roundDouble(m_RCGroup.volumeFake = fake);
    emit renderingGroupChanged();
    return true;
  }
  return false;
}

int Player::currentTrackPosition()
{
  if (m_player)
  {
    SONOS::ElementList vars;
    m_player->GetPositionInfo(vars);
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


  if (m_player)
  {
    QString port;
    port.setNum(m_player->GetPort());
    QString url = "http://";
    url.append(m_player->GetHost().c_str()).append(":").append(port);

    m_currentProtocol = m_player->GetURIProtocol(prop.CurrentTrackURI);
    m_currentMetaSource = QString::fromUtf8(prop.CurrentTrackURI.c_str());
    if (prop.r_EnqueuedTransportURIMetaData)
      m_currentMetaURITitle = QString::fromUtf8(prop.r_EnqueuedTransportURIMetaData->GetValue("dc:title").c_str());

    unsigned hh, hm, hs;
    if (sscanf(prop.CurrentTrackDuration.c_str(), "%u:%u:%u", &hh, &hm, &hs) == 3)
      m_currentTrackDuration = hh * 3600 + hm * 60 + hs;

    if (prop.CurrentTrackMetaData)
    {
      QString uri;
      // Postulate: stream has 0 duration
      if (m_currentTrackDuration == 0)
      {
        // stream
        uri = QString::fromUtf8(prop.CurrentTrackMetaData->GetValue("upnp:albumArtURI").c_str());
        if (prop.TransportState.compare("TRANSITIONING") == 0)
          m_currentMetaTitle = m_currentMetaURITitle;
        else
        {
          m_currentMetaTitle = QString::fromUtf8(prop.CurrentTrackMetaData->GetValue("r:streamContent").c_str());
          // fallback to uri title then title
          if (m_currentMetaTitle.isEmpty())
            m_currentMetaTitle = !m_currentMetaURITitle.isEmpty() ? m_currentMetaURITitle : QString::fromUtf8(prop.CurrentTrackMetaData->GetValue("dc:title").c_str());
        }
        m_currentMetaAlbum = QString::fromUtf8(prop.CurrentTrackMetaData->GetValue("upnp:album").c_str());
        m_currentMetaArtist = "";
      }
      else
      {
        // file
        uri = QString::fromUtf8(prop.CurrentTrackMetaData->GetValue("upnp:albumArtURI").c_str());
        m_currentMetaTitle = QString::fromUtf8(prop.CurrentTrackMetaData->GetValue("dc:title").c_str());
        m_currentMetaAlbum = QString::fromUtf8(prop.CurrentTrackMetaData->GetValue("upnp:album").c_str());
        m_currentMetaArtist = QString::fromUtf8(prop.CurrentTrackMetaData->GetValue("dc:creator").c_str());
        m_currentIndex = prop.CurrentTrack - 1; // playing queue
      }
      if (!uri.isEmpty())
        m_currentMetaArt = url + uri;
    }
  }
}

void Player::handleTransportChange()
{
  if (m_player)
  {
#define AVTRANSPORT_UNCHANGED           0
#define AVTRANSPORT_STATE_CHANGED       1
#define AVTRANSPORT_PLAYMODE_CHANGED    2
#define AVTRANSPORT_SLEEPTIMER_CHANGED  4
    unsigned signalMask = AVTRANSPORT_UNCHANGED;

    SONOS::AVTProperty prop = m_player->GetTransportProperty();

    setCurrentMeta(prop);
    emit sourceChanged();

    if (prop.TransportState != m_AVTProperty.TransportState)
      signalMask |= AVTRANSPORT_STATE_CHANGED;

    if (prop.CurrentPlayMode != m_AVTProperty.CurrentPlayMode)
      signalMask |= AVTRANSPORT_PLAYMODE_CHANGED;

    if (prop.r_SleepTimerGeneration != m_AVTProperty.r_SleepTimerGeneration)
      signalMask |= AVTRANSPORT_SLEEPTIMER_CHANGED;

    // Set property before emit
    m_AVTProperty = prop;
    if (signalMask & AVTRANSPORT_STATE_CHANGED)
      emit playbackStateChanged();
    if (signalMask & AVTRANSPORT_PLAYMODE_CHANGED)
      emit playModeChanged();
    if (signalMask & AVTRANSPORT_SLEEPTIMER_CHANGED)
      emit sleepTimerChanged();
  }
}

void Player::handleRenderingControlChange()
{
  if (m_player)
  {
#define RENDERING_UNCHANGED     0
#define RENDERING_GROUP_CHANGED 1
#define RENDERING_CHANGED       2
    unsigned signalMask = RENDERING_UNCHANGED;

    SONOS::SRPList props = m_player->GetRenderingProperty();
    // At initial callback the table of properties for the connected zone is empty
    // So fill it for each subordinate attached to the zone
    if (m_RCTable.empty())
    {
      double volume = 0.0;
      bool mute = true;
      SONOS::SRPList::const_iterator it = props.begin();
      while (it != props.end())
      {
        RCProperty item;
        item.uuid = it->uuid;
        item.name = it->subordinateName;
        item.mute = it->property.MuteMaster ? true : false;
        item.nightmode = it->property.NightMode ? true : false;
        item.volume = it->property.VolumeMaster;
        item.volumeFake = it->property.VolumeMaster > 0 ? (double)it->property.VolumeMaster : 100.0 / 101.0;
        item.treble = it->property.Treble;
        item.bass = it->property.Bass;
        m_RCTable.push_back(item);
        if (!item.mute)
          mute = false; // exists active audio in group
        volume += item.volumeFake;

        // As the sound settings relate the connected group and doesn't handle separately each subordinate,
        // i gatherer data only from the master zone that is the first on the array.
        if (it == props.begin())
        {
          m_RCGroup.nightmode = item.nightmode;
          m_RCGroup.treble = item.treble;
          m_RCGroup.bass = item.bass;
        }

        ++it;
      }
      volume /= (double)props.size();
      m_RCGroup.volumeFake = volume;
      m_RCGroup.volume = roundDouble(volume);
      m_RCGroup.mute = mute;
      signalMask |= RENDERING_GROUP_CHANGED | RENDERING_CHANGED; // handles group & subordinate update
    }
    else
    {
      double volume = 0.0;
      bool mute = true;
      bool nightmode = false;
      int treble = 0, bass = 0;
      SONOS::SRPList::const_iterator it = props.begin();
      std::vector<RCProperty>::iterator itz = m_RCTable.begin();
      while (it != props.end())
      {
        if (it->property.MuteMaster != itz->mute)
        {
          itz->mute = it->property.MuteMaster;
          signalMask |= RENDERING_CHANGED;
        }
        if (it->property.NightMode != itz->nightmode)
        {
          itz->nightmode = it->property.NightMode;
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
          nightmode = it->property.NightMode;
          treble = it->property.Treble;
          bass = it->property.Bass;
        }
        // And override data from the first subordinate updated accordinaly with sended values
        else
        {
          if (it->property.NightMode == m_RCGroup.nightmode)
            nightmode = it->property.NightMode;
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
        volume += itz->volumeFake;
        ++it;
        ++itz;
      }
      volume /= (double)props.size();
      m_RCGroup.volumeFake = volume;
      m_RCGroup.volume = roundDouble(volume);
      m_RCGroup.mute = mute;
      m_RCGroup.nightmode = nightmode;
      m_RCGroup.treble = treble;
      m_RCGroup.bass = bass;
      signalMask |= RENDERING_GROUP_CHANGED; // handles group update
    }

    SONOS::DBG(DBG_DEBUG, "%s: sig=%d volume: %3.3f [%d]\n", __FUNCTION__, signalMask, m_RCGroup.volumeFake, m_RCGroup.volume);

    // emit signal about changes
    if (signalMask & RENDERING_GROUP_CHANGED)
      emit renderingGroupChanged();
    if (signalMask & RENDERING_CHANGED)
      emit renderingChanged();
  }
}

void Player::connectSonos(Sonos* sonos)
{
  if (sonos)
  {
    QObject::connect(sonos, SIGNAL(transportChanged()), this, SLOT(handleTransportChange()));
    QObject::connect(sonos, SIGNAL(renderingControlChanged()), this, SLOT(handleRenderingControlChange()));
  }
}

void Player::disconnectSonos(Sonos* sonos)
{
  if (sonos)
  {
    QObject::disconnect(sonos, SIGNAL(transportChanged()), this, SLOT(handleTransportChange()));
    QObject::disconnect(sonos, SIGNAL(renderingControlChanged()), this, SLOT(handleRenderingControlChange()));
  }
}
