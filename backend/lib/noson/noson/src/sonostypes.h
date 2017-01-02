/*
 *      Copyright (C) 2015 Jean-Luc Barriere
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

#ifndef SONOSTYPES_H
#define	SONOSTYPES_H

#include <local_config.h>
#include <stdint.h>
#include <vector>

#include "digitalitem.h"

namespace NSROOT
{
  typedef void (*EventCB)(void*);

  typedef enum
  {
    SVCEvent_TransportChanged        = 0x01,
    SVCEvent_RenderingControlChanged = 0x02,
    SVCEvent_ContentDirectoryChanged = 0x04,
  } SVCEventMask_t;

  typedef enum
  {
    PlayMode_NORMAL           = 0,
    PlayMode_REPEAT_ALL       = 1,
    PlayMode_SHUFFLE,
    PlayMode_SHUFFLE_NOREPEAT,
    PlayMode_REPEAT_ONE,
  } PlayMode_t;

  typedef enum
  {
    TransportState_STOPPED          = 0,
    TransportState_PLAYING          = 1,
    TransportState_PAUSED_PLAYBACK,
    TransportState_TRANSITIONING,
    TransportState_NO_MEDIA_PRESENT,
    TransportState_RECORDING,
    TransportState_PAUSED_RECORDING,
  } TransportState_t;

  typedef enum
  {
    Protocol_xRinconQueue    = 0,
    Protocol_xRinconStream   = 1,
    Protocol_xRinconMP3Radio,
    Protocol_xRinconPlaylist,
    Protocol_xSonosHtaStream,
    Protocol_xSonosHttp,
    Protocol_xSonosApiStream,
    Protocol_xFileCifs,
    Protocol_xRincon,
    Protocol_httpGet,
    Protocol_aac,
    Protocol_unknown,
  } Protocol_t;

  extern const char* ProtocolTable[Protocol_unknown + 1];

  typedef enum
  {
    ServiceDesc_default,
    ServiceDesc_unknown,
  } ServiceDesc_t;

  extern const char* ServiceDescTable[ServiceDesc_unknown + 1];

  class AVTProperty
  {
  public:
    AVTProperty()
    : NumberOfTracks(0)
    , CurrentTrack(0)
    , CurrentSection(0) { }

    virtual ~AVTProperty() { }

    std::string     TransportState;               // PLAYING, STOPPED
    std::string     CurrentPlayMode;              // NORMAL
    std::string     CurrentCrossfadeMode;         // 0
    unsigned        NumberOfTracks;               // 13
    unsigned        CurrentTrack;                 // 9
    unsigned        CurrentSection;               // 0
    std::string     CurrentTrackURI;              // x-file-cifs://server/share/music/track.flac
    std::string     CurrentTrackDuration;         // 0:03:14
    DigitalItemPtr  CurrentTrackMetaData;
    std::string     r_NextTrackURI;               // x-file-cifs://server/share/music/track.flac
    DigitalItemPtr  r_NextTrackMetaData;
    std::string     r_EnqueuedTransportURI;       // x-rincon-playlist:RINCON_000E58343F7201400#A:ALBUMARTIST/artist/album
    DigitalItemPtr  r_EnqueuedTransportURIMetaData;
    std::string     PlaybackStorageMedium;        // NETWORK
    std::string     AVTransportURI;               // x-rincon-queue:RINCON_000E583F284001400#0
    DigitalItemPtr  AVTransportURIMetaData;
    std::string     NextAVTransportURI;
    std::string     NextAVTransportURIMetaData;
    std::string     CurrentTransportActions;      // Set, Play, Stop, Pause, Seek, Next, Previous
    std::string     r_CurrentValidPlayModes;      // SHUFFLE,REPEAT,CROSSFADE
    std::string     r_MuseSessions;
    std::string     TransportStatus;              // OK
    std::string     r_SleepTimerGeneration;       // 0
    std::string     r_AlarmRunning;               // 0
    std::string     r_SnoozeRunning;              // 0
    std::string     r_RestartPending;             // 0
    std::string     PossiblePlaybackStorageMedia; // NONE, NETWORK

    // NOT IMPLEMENTED
    //std::string TransportPlaySpeed
    //std::string CurrentMediaDuration
    //std::string RecordStorageMedium
    //std::string PossibleRecordStorageMedia
    //std::string RecordMediumWriteStatus
    //std::string CurrentRecordQualityMode
    //std::string PossibleRecordQualityModes
  };

  class ContentProperty
  {
  public:
    ContentProperty()
    : ShareIndexInProgress(false) { }

    virtual ~ContentProperty() { }

    std::string SystemUpdateID;
    bool ShareIndexInProgress;
    std::string ShareIndexLastError;
    std::vector<std::pair<std::string, unsigned> > ContainerUpdateIDs;
    std::string UserRadioUpdateID;
    std::string SavedQueuesUpdateID;
    std::string ShareListUpdateID;
    std::string RecentlyPlayedUpdateID;
    std::string RadioFavoritesUpdateID;
    std::string RadioLocationUpdateID;
    std::string FavoritesUpdateID;
    std::string FavoritePresetsUpdateID;
  };

  class RCSProperty
  {
  public:
    RCSProperty()
    : VolumeMaster(0)
    , VolumeLF(0)
    , VolumeRF(0)
    , MuteMaster(0)
    , MuteLF(0)
    , MuteRF(0) { }

    virtual ~RCSProperty() { }

    int VolumeMaster;
    int VolumeLF;
    int VolumeRF;
    int MuteMaster;
    int MuteLF;
    int MuteRF;
  };

  class SRProperty
  {
  public:
    SRProperty() {}

    virtual ~SRProperty() { }

    std::string uuid;
    std::string subordinateName;
    RCSProperty property;
  };

}

#endif	/* SONOSTYPES_H */

