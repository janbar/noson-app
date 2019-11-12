/*
 *      Copyright (C) 2018 Jean-Luc Barriere
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

#if (defined(_WIN32) || defined(_WIN64))
#define __WINDOWS__
#endif

#ifdef __WINDOWS__
#include <winsock2.h>
#include <Windows.h>
#include <time.h>
#define usleep(t) Sleep((DWORD)(t)/1000)
#define sleep(t)  Sleep((DWORD)(t)*1000)
#else
#include <unistd.h>
#include <sys/time.h>
#endif

#include <noson/sonossystem.h>
#include <noson/sonosplayer.h>
#include <noson/contentdirectory.h>
#include <noson/didlparser.h>
#include <noson/imageservice.h>
#include <noson/filestreamer.h>
#ifdef HAVE_PULSEAUDIO
#include <noson/pulsestreamer.h>
#endif

#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <string>
#include <algorithm> // std::find

#include "private/debug.h"
#include "private/tokenizer.h"
#include "private/builtin.h"

#ifdef __WINDOWS__
#define PRINT(a) fprintf(stderr, a)
#define PRINT1(a,b) fprintf(stderr, a, b)
#define PRINT2(a,b,c) fprintf(stderr, a, b, c)
#define PRINT3(a,b,c,d) fprintf(stderr, a, b, c, d)
#define PRINT4(a,b,c,d,e) fprintf(stderr, a, b, c, d, e)
#else
#define PRINT(a) fprintf(stdout, a)
#define PRINT1(a,b) fprintf(stdout, a, b)
#define PRINT2(a,b,c) fprintf(stdout, a, b, c)
#define PRINT3(a,b,c,d) fprintf(stdout, a, b, c, d)
#define PRINT4(a,b,c,d,e) fprintf(stdout, a, b, c, d, e)
#endif
#define PERROR(a) fprintf(stderr, a)
#define PERROR1(a,b) fprintf(stderr, a, b)
#define PERROR2(a,b,c) fprintf(stderr, a, b, c)
#define PERROR3(a,b,c,d) fprintf(stderr, a, b, c, d)

static const char * getCmd(char **begin, char **end, const std::string& option);
static const char * getCmdOption(char **begin, char **end, const std::string& option);
static void readStream(std::istream*);

SONOS::System * gSonos = 0;
SONOS::PlayerPtr gPlayer;

void handleEventCB(void* handle)
{
  unsigned char mask = gSonos->LastEvents();
  if ((mask & SONOS::SVCEvent_AlarmClockChanged))
    SONOS::DBG(DBG_DEBUG, "AlarmClockChanged event triggered\n");
  if ((mask & SONOS::SVCEvent_ZGTopologyChanged))
    SONOS::DBG(DBG_DEBUG, "ZGTopologyChanged event triggered\n");
}

/*
 * the main function
 */
int main(int argc, char** argv)
{
  int ret = 0;
  SONOS::DBGLevel(DBG_ERROR);

  if (getCmd(argv, argv + argc, "--help") || getCmd(argv, argv + argc, "-h"))
  {
    PRINT("\n  --deviceurl <URL>\n\n");
    PRINT("  Bypass the SSDP discovery by connecting to an endpoint. The typical URLs are:\n");
    PRINT("  http://{IPADDRESS}:1400 or http://{IPADDRESS}:3400\n");
    PRINT("\n  --debug\n\n");
    PRINT("  Enable the debug output.\n");
    PRINT("\n  --help | -h\n\n");
    PRINT("  Print the command usage.\n\n");
    return EXIT_SUCCESS;
  }

  if (getCmd(argv, argv + argc, "--debug"))
    SONOS::DBGLevel(DBG_PROTO);

  const char* deviceUrl = getCmdOption(argv, argv + argc, "--deviceurl");

#ifdef __WINDOWS__
  //Initialize Winsock
  WSADATA wsaData;
  if ((ret = WSAStartup(MAKEWORD(2, 2), &wsaData)))
    return ret;
#endif /* __WINDOWS__ */

  PRINT1("Noson CLI using libnoson %s, Copyright (C) 2018 Jean-Luc Barriere\n", SONOS::libVersionString());
  gSonos = new SONOS::System(0, handleEventCB);
  if (!deviceUrl)
  {
    PERROR("Searching... ");
    if (!gSonos->Discover())
    {
      PERROR("No SONOS zone found.\n");
      return EXIT_FAILURE;
    }
    else
      PERROR("Succeeded\n");
  }
  else
  {
    PERROR1("Connecting to %s... ", deviceUrl);
    if (!gSonos->Discover(deviceUrl))
    {
      PERROR("The SONOS device is unreachable.\n");
      return EXIT_FAILURE;
    }
    else
      PERROR("Succeeded\n");
  }

  /*
   * Register handlers to process remote request
   */
  {
    SONOS::RequestBrokerPtr imageService(new SONOS::ImageService());
    gSonos->RegisterRequestBroker(imageService);
#ifdef HAVE_PULSEAUDIO
    gSonos->RegisterRequestBroker(SONOS::RequestBrokerPtr(new SONOS::PulseStreamer(imageService.get())));
#endif
    gSonos->RegisterRequestBroker(SONOS::RequestBrokerPtr(new SONOS::FileStreamer()));
  }
  /*
   * Print Players list
   */
  SONOS::ZonePlayerList players = gSonos->GetZonePlayerList();
  for (SONOS::ZonePlayerList::const_iterator it = players.begin(); it != players.end(); ++it)
    PRINT2("Found player '%s' with UUID '%s'\n", it->first.c_str(), it->second->GetUUID().c_str());
  /*
   * Print Zones list and connect to
   */
  SONOS::ZoneList zones = gSonos->GetZoneList();
  for (SONOS::ZoneList::const_iterator it = zones.begin(); it != zones.end(); ++it)
    PRINT2("Found zone '%s' with coordinator '%s'\n", it->second->GetZoneName().c_str(), it->second->GetCoordinator()->c_str());

  readStream(&std::cin);

  if (gPlayer)
    gPlayer.reset();
  delete gSonos;
  gSonos = 0;

#ifdef __WINDOWS__
  WSACleanup();
#endif /* __WINDOWS__ */
  return ret;
}

static const char * getCmd(char **begin, char **end, const std::string& option)
{
  char **itr = std::find(begin, end, option);
  if (itr != end)
  {
    return *itr;
  }
  return NULL;
}

static const char * getCmdOption(char **begin, char **end, const std::string& option)
{
  for (char** it = begin; it != end; ++it)
  {
    if (strncmp(*it, option.c_str(), option.length()) == 0 && (*it)[option.length()] == '=')
      return &((*it)[option.length() + 1]);
  }
  return NULL;
}

std::string& upstr(std::string& str)
{
  std::string::iterator c = str.begin();
  while (c != str.end())
  {
    *c = toupper(*c);
    ++c;
  }
  return str;
}

static bool parseCommand(const std::string& line)
{
  std::vector<std::string> tokens;
  tokenize(line, " ", tokens, true);
  std::vector<std::string>::const_iterator it = tokens.begin();
  if (it != tokens.end())
  {
    std::string token(*it);
    upstr(token);

    if (token == "EXIT")
      return false;
    else if (token == "")
    {}
    else if (token == "HELP")
    {
      PRINT("EXIT                          Exit from CLI\n");
      PRINT("CONNECT {zone name}           Connect to a zone for control\n");
      PRINT("STATUS                        Show the playing status\n");
#ifdef HAVE_PULSEAUDIO
      PRINT("PLAYPULSE                     Play stream from Pulse\n");
#endif
      PRINT("PLAYURL {stream URL}          Play stream from URL\n");
      PRINT("PLAYFV {URI}                  Play the given favorite\n");
      PRINT("PLAYSQ {URI}                  Play the given playlist\n");
      PRINT("PLAYQUEUE                     Play queue\n");
      PRINT("PLAYLINEIN                    Play line-IN\n");
      PRINT("PLAYDIGITALIN                 Play digital-IN/TV\n");
      PRINT("PLAY                          Press play\n");
      PRINT("PAUSE                         Press pause\n");
      PRINT("STOP                          Press stop\n");
      PRINT("PREVIOUS                      Press skip previous\n");
      PRINT("NEXT                          Press skip next\n");
      PRINT("SEEK 1..                      Seek to track number\n");
      PRINT("VOLUME 0..100                 Set volume master\n");
      PRINT("VOLUME {player} 0..100        Set volume\n");
      PRINT("SLEEPTIMER 0..65535           Set sleep timer\n");
      PRINT("SHOWQUEUE                     Show queue content\n");
      PRINT("SHOWFV                        Show favorites\n");
      PRINT("SHOWSQ                        Show playlists\n");
      PRINT("SHOWAC                        Show alarms clock\n");
      PRINT("CREATEAC {1} {2} {3} {4} {5}  Create alarm clock using arguments:\n");
      PRINT("  1:ROOM       The room UUID\n");
      PRINT("  2:STARTTIME  The time using format HH:MM:SS\n");
      PRINT("  3:RECURRENCE The comma separated values of day: SUN,MON,..,SAT\n");
      PRINT("  4:DURATION   The duration using format HH:MM:SS\n");
      PRINT("  5:VOLUME     0..100\n");
      PRINT("ENABLEAC {id}                 Enable alarm clock\n");
      PRINT("DISABLEAC {id}                Disable alarm clock\n");
      PRINT("DESTROYAC {id}                Destroy alarm clock\n");
      PRINT("UPDATEAC {id} {2} {3}         Update alarm clock using arguments:\n");
      PRINT("  2:Type ROOM,STARTTIME,RECURRENCE,DURATION,VOLUME,PROGRAM\n");
      PRINT("  3:New value\n");
      PRINT("  Program 0 for Buzzer or the index of favorite (see SHOWFV)\n");
      PRINT("HELP                          Print this help\n");
      PRINT("\n");
    }
    else if (token == "CONNECT")
    {
      if (++it != tokens.end())
      {
        std::string param(*it);
        while(++it != tokens.end())
          param.append(" ").append(*it);
        SONOS::ZoneList zones = gSonos->GetZoneList();
        bool found = false;
        for (SONOS::ZoneList::const_iterator iz = zones.begin(); iz != zones.end(); ++iz)
        {
          if (iz->second->GetZoneName() == param)
          {
            found = true;
            if ((gPlayer = gSonos->GetPlayer(iz->second, 0, 0)))
              PERROR1("Connected to zone %s\n", gPlayer->GetZone()->GetZoneName().c_str());
            else
              PERROR("Failed\n");
            break;
          }
        }
        if (!found)
          PERROR("Not found\n");
      }
      else
        PERROR("Error: Missing arguments.\n");
    }
    else if (!gSonos->IsConnected() || !gPlayer)
    {
      PERROR("Error: Not connected.\n");
    }
    else if (token == "STATUS")
    {
      while (gPlayer->TransportPropertyEmpty())
        sleep(1);
      SONOS::AVTProperty props = gPlayer->GetTransportProperty();
      PRINT1("TransportStatus = %s\n", props.TransportStatus.c_str());
      PRINT1("TransportState = %s\n", props.TransportState.c_str());
      PRINT1("AVTransportURI = [%s]\n", props.AVTransportURI.c_str());
      PRINT1("AVTransportTitle = [%s]\n", props.AVTransportURIMetaData ? props.AVTransportURIMetaData->GetValue("dc:title").c_str() : "null");
      PRINT1("CurrentTrack = %d\n", props.CurrentTrack);
      PRINT1("CurrentTrackDuration = %s\n", props.CurrentTrackDuration.c_str());
      PRINT1("CurrentTrackURI = [%s]\n", props.CurrentTrackURI.c_str());
      PRINT1("CurrentTrackTitle = [%s]\n", props.CurrentTrackMetaData ? props.CurrentTrackMetaData->GetValue("dc:title").c_str() : "null");
      PRINT1("CurrentTrackAlbum = [%s]\n", props.CurrentTrackMetaData ? props.CurrentTrackMetaData->GetValue("upnp:album").c_str() : "null");
      PRINT1("CurrentTrackArtist = [%s]\n", props.CurrentTrackMetaData ? props.CurrentTrackMetaData->GetValue("dc:creator").c_str() : "null");
      PRINT1("CurrentCrossfadeMode = %s\n", props.CurrentCrossfadeMode.c_str());
      PRINT1("CurrentPlayMode = %s\n", props.CurrentPlayMode.c_str());
      PRINT1("CurrentTransportActions = %s\n", props.CurrentTransportActions.c_str());
      PRINT1("NumberOfTracks = %d\n", props.NumberOfTracks);
      PRINT1("AlarmRunning = %s\n", props.r_AlarmRunning.c_str());
      PRINT1("AlarmIDRunning = %s\n", props.r_AlarmIDRunning.c_str());
      PRINT1("AlarmLoggedStartTime = %s\n", props.r_AlarmLoggedStartTime.c_str());
      PRINT1("AlarmState = %s\n", props.r_AlarmState.c_str());

      SONOS::ElementList vars;
      if (gPlayer->GetRemainingSleepTimerDuration(vars))
      {
        PRINT1("RemainingSleepTimerDuration = %s\n", vars.GetValue("RemainingSleepTimerDuration").c_str());
      }
    }
    else if (token == "SLEEPTIMER")
    {
      if (++it != tokens.end())
      {
        std::string param(*it);
        uint16_t value = 0;
        string_to_uint16(param.c_str(), &value);
        if (gPlayer->ConfigureSleepTimer((unsigned)value))
          PERROR("Succeeded\n");
        else
          PERROR("Failed\n");
      }
      else
        PERROR("Error: Missing arguments.\n");
    }
    else if (token == "SHOWAC")
    {
      SONOS::AlarmList alarms = gSonos->GetAlarmList();
      for (SONOS::AlarmList::const_iterator il = alarms.begin(); il != alarms.end(); ++il)
      {
	PRINT("\n");
	PRINT2("%s: Enabled = %s\n", (*il)->GetId().c_str(), (*il)->GetEnabled() ? "true" : "false");
	PRINT2("%s: StartTime = %s\n", (*il)->GetId().c_str(), (*il)->GetStartLocalTime().c_str());
	PRINT2("%s: Recurrence = %s\n", (*il)->GetId().c_str(), (*il)->GetRecurrence().c_str());
	PRINT2("%s: RoomUUID = %s\n", (*il)->GetId().c_str(), (*il)->GetRoomUUID().c_str());
	PRINT2("%s: IncludeLinkedZones = %s\n", (*il)->GetId().c_str(), (*il)->GetIncludeLinkedZones() ? "true" : "false");
	PRINT2("%s: ProgramURI = %s\n", (*il)->GetId().c_str(), (*il)->GetProgramURI().c_str());
	const SONOS::DigitalItemPtr didl = (*il)->GetProgramMetadata();
	PRINT2("%s: ProgramTitle = %s\n", (*il)->GetId().c_str(), didl ? didl->GetValue("dc:title").c_str() : "");
	PRINT2("%s: PlayMode = %s\n", (*il)->GetId().c_str(), (*il)->GetPlayMode().c_str());
	PRINT2("%s: Volume = %d\n", (*il)->GetId().c_str(), (*il)->GetVolume());
	PRINT2("%s: Duration = %s\n", (*il)->GetId().c_str(), (*il)->GetDuration().c_str());
      }
    }
    else if (token == "CREATEAC")
    {
      std::string roomUUID;
      std::string start;
      std::string recurrence("MON,TUE,WED,THU,FRI");
      std::string duration("01:00:00");
      uint8_t volume = 20;
      if (++it != tokens.end())
        roomUUID.assign(*it);
      if (++it != tokens.end())
        start.assign(*it);
      if (it != tokens.end() && ++it != tokens.end())
        recurrence.assign(*it);
      if (it != tokens.end() && ++it != tokens.end())
        duration.assign(*it);
      if (it != tokens.end() && ++it != tokens.end())
        string_to_uint8(it->c_str(), &volume);
      if (it != tokens.end() && ++it == tokens.end())
      {
        SONOS::Alarm alarm;
        alarm.SetRoomUUID(roomUUID);
        alarm.SetStartLocalTime(start);
        alarm.SetRecurrence(recurrence);
        alarm.SetDuration(duration);
        alarm.SetVolume(volume);
        if (gSonos->CreateAlarm(alarm))
          PERROR("Succeeded\n");
        else
          PERROR("Failed\n");
      }
      else
        PERROR("Error: Missing arguments.\n");
    }
    else if (token == "ENABLEAC")
    {
      if (++it != tokens.end())
      {
        SONOS::AlarmList alarms = gSonos->GetAlarmList();
        SONOS::AlarmPtr ptr;
        for (SONOS::AlarmList::iterator il = alarms.begin(); il != alarms.end(); ++il)
        {
          if ((*il)->GetId() == *it)
          {
            ptr = *il;
            break;
          }
        }
        if (ptr)
        {
          ptr->SetEnabled(true);
          if (gSonos->UpdateAlarm(*ptr))
            PERROR("Succeeded\n");
          else
            PERROR("Failed\n");
        }
        else
          PERROR("Error: Invalid alarm ID.\n");
      }
      else
        PERROR("Error: Missing arguments.\n");
    }
    else if (token == "DISABLEAC")
    {
      if (++it != tokens.end())
      {
        SONOS::AlarmList alarms = gSonos->GetAlarmList();
        SONOS::AlarmPtr ptr;
        for (SONOS::AlarmList::iterator il = alarms.begin(); il != alarms.end(); ++il)
        {
          if ((*il)->GetId() == *it)
          {
            ptr = *il;
            break;
          }
        }
        if (ptr)
        {
          ptr->SetEnabled(false);
          if (gSonos->UpdateAlarm(*ptr))
            PERROR("Succeeded\n");
          else
            PERROR("Failed\n");
        }
        else
          PERROR("Error: Invalid alarm ID.\n");
      }
      else
        PERROR("Error: Missing arguments.\n");
    }
    else if (token == "DESTROYAC")
    {
      if (++it != tokens.end())
      {
        if (gSonos->DestroyAlarm(*it))
          PERROR("Succeeded\n");
        else
          PERROR("Failed\n");
      }
      else
        PERROR("Error: Missing arguments.\n");
    }
    else if (token == "UPDATEAC")
    {
      std::string alarmId;
      std::string type;
      if (++it != tokens.end())
        alarmId.assign(*it);
      if (it != tokens.end() && ++it != tokens.end())
      {
        type.assign(*it);
        upstr(type);
      }
      if (it != tokens.end() && ++it != tokens.end())
      {
        SONOS::AlarmList alarms = gSonos->GetAlarmList();
        SONOS::AlarmPtr ptr;
        for (SONOS::AlarmList::iterator il = alarms.begin(); il != alarms.end(); ++il)
        {
          if ((*il)->GetId() == alarmId)
          {
            ptr = *il;
            break;
          }
        }
        if (ptr)
        {
          //ROOM,TIME,RECURRENCE,DURATION,VOLUME
          if (type == "ROOM")
            ptr->SetRoomUUID(*it);
          else if (type == "STARTTIME")
            ptr->SetStartLocalTime(*it);
          else if (type == "RECURRENCE")
            ptr->SetRecurrence(*it);
          else if (type == "DURATION")
            ptr->SetDuration(*it);
          else if (type == "VOLUME")
          {
            uint8_t value;
            string_to_uint8(it->c_str(), &value);
            ptr->SetVolume(value);
          }
          else if (type == "PROGRAM")
          {
            if (*it == "0")
            {
              ptr->SetProgramURI(ALARM_BUZZER_URI);
              ptr->SetProgramMetadata(SONOS::DigitalItemPtr());
            }
            else
            {
              uint16_t value;
              string_to_uint16(it->c_str(), &value);
              SONOS::ContentDirectory mycontent(gSonos->GetHost(), gSonos->GetPort());
              SONOS::ContentList bdir(mycontent, "FV:2");
              SONOS::ContentList::iterator ic = bdir.begin();
              uint16_t i = 0;
              while (ic != bdir.end())
              {
                if (++i == value)
                {
                  SONOS::DigitalItemPtr metaPtr;
                  if (SONOS::System::ExtractObjectFromFavorite(*ic, metaPtr))
                  {
                    ptr->SetProgramURI(metaPtr->GetValue("res"));
                    ptr->SetProgramMetadata(metaPtr);
                  }
                  break;
                }
                ++ic;
              }
            }
          }
          if (gSonos->UpdateAlarm(*ptr))
            PERROR("Succeeded\n");
          else
            PERROR("Failed\n");
        }
        else
          PERROR("Error: Invalid alarm ID.\n");
      }
      else
        PERROR("Error: Missing arguments.\n");
    }
    else if (token == "PLAY")
    {
      if (gPlayer->Play())
        PERROR("Succeeded\n");
      else
        PERROR("Failed\n");
    }
    else if (token == "STOP")
    {
      if (gPlayer->Stop())
        PERROR("Succeeded\n");
      else
        PERROR("Failed\n");
    }
    else if (token == "PAUSE")
    {
      if (gPlayer->Pause())
        PERROR("Succeeded\n");
      else
        PERROR("Failed\n");
    }
    else if (token == "PREVIOUS")
    {
      if (gPlayer->Previous())
        PERROR("Succeeded\n");
      else
        PERROR("Failed\n");
    }
    else if (token == "NEXT")
    {
      if (gPlayer->Next())
        PERROR("Succeeded\n");
      else
        PERROR("Failed\n");
    }
    else if (token == "SEEK")
    {
      if (++it != tokens.end())
      {
        uint32_t value;
        string_to_uint32(it->c_str(), &value);
        if (gPlayer->SeekTrack(value))
          PERROR("Succeeded\n");
        else
          PERROR("Failed\n");
      }
      else
        PERROR("Error: Missing arguments.\n");
    }
#ifdef HAVE_PULSEAUDIO
    else if (token == "PLAYPULSE")
    {
      if (gPlayer->PlayPulse())
        PERROR("Succeeded\n");
      else
        PERROR("Failed\n");
    }
#endif
    else if (token == "PLAYURL")
    {
      if (++it != tokens.end())
      {
        std::string param(*it);
        while(++it != tokens.end())
          param.append(" ").append(*it);
        if (gPlayer->PlayStream(param, ""))
          PERROR("Succeeded\n");
        else
          PERROR("Failed\n");
      }
      else
        PERROR("Error: Missing arguments.\n");
    }
    else if (token == "VOLUME")
    {
      if (++it != tokens.end())
      {
        bool all = true;
        std::string param(*it);
        std::string param2;
        while(++it != tokens.end())
        {
          all = false;
          if ((it + 1) == tokens.end())
            param2.append(*it);
          else
            param.append(" ").append(*it);
        }
        if (all)
          param2.assign(param);
        SONOS::ZonePtr pl = gPlayer->GetZone();
        for (SONOS::Zone::iterator ip = pl->begin(); ip != pl->end(); ++ip)
        {
          if (all || param == **ip)
          {
            uint8_t value = 0;
            string_to_uint8(param2.c_str(), &value);
            if (gPlayer->SetVolume((*ip)->GetUUID(), value))
              PERROR3("%s [%s]: volume %u\n", (*ip)->c_str(), (*ip)->GetUUID().c_str(), value);
            else
              PERROR2("%s [%s]: Failed\n", (*ip)->c_str(), (*ip)->GetUUID().c_str());
          }
        }
      }
      else
        PERROR("Error: Missing arguments.\n");
    }
    else if (token == "SHOWQUEUE")
    {
      SONOS::ContentDirectory mycontent(gSonos->GetHost(), gSonos->GetPort());
      SONOS::ContentList bdir(mycontent, "Q:0");
      PRINT1("UpdateID  : %u\n", bdir.GetUpdateID());
      PRINT1("Item count: %u\n", bdir.size());
      SONOS::ContentList::iterator ic = bdir.begin();
      int i = 0;
      while (ic != bdir.end())
      {
        PRINT3("%d: [%s] [%s]\n", ++i, (*ic)->GetValue("dc:title").c_str(), (*ic)->GetValue("res").c_str());
        ++ic;
      }
    }
    else if (token == "PLAYQUEUE")
    {
      if (gPlayer->PlayQueue(true))
        PERROR("Succeeded\n");
      else
        PERROR("Failed\n");
    }
    else if (token == "PLAYLINEIN")
    {
      if (gPlayer->PlayLineIN())
        PERROR("Succeeded\n");
      else
        PERROR("Failed\n");
    }
    else if (token == "PLAYDIGITALIN")
    {
      if (gPlayer->PlayDigitalIN())
        PERROR("Succeeded\n");
      else
        PERROR("Failed\n");
    }
    else if (token == "SHOWFV")
    {
      SONOS::ContentDirectory mycontent(gSonos->GetHost(), gSonos->GetPort());
      SONOS::ContentList bdir(mycontent, "FV:2");
      PRINT1("UpdateID  : %u\n", bdir.GetUpdateID());
      PRINT1("Item count: %u\n", bdir.size());
      SONOS::ContentList::iterator ic = bdir.begin();
      int i = 0;
      while (ic != bdir.end())
      {
        PRINT3("%d: [%s] [%s]\n", ++i, (*ic)->GetValue("dc:title").c_str(), (*ic)->GetValue("res").c_str());
        ++ic;
      }
    }
    else if (token == "PLAYFV")
    {
      if (++it != tokens.end())
      {
        SONOS::ContentDirectory mycontent(gSonos->GetHost(), gSonos->GetPort());
        SONOS::ContentList bdir(mycontent, "FV:2");
        SONOS::ContentList::iterator ic = bdir.begin();
        while (ic != bdir.end())
        {
          if ((*ic)->GetValue("res") == (*it))
          {
            SONOS::DigitalItemPtr item;
            if (SONOS::System::ExtractObjectFromFavorite((*ic), item))
            {
              if (SONOS::System::CanQueueItem(item))
              {
                PRINT2("Playing item [%s] [%s]\n", (*ic)->GetValue("dc:title").c_str(), (*ic)->GetValue("res").c_str());
                if (gPlayer->RemoveAllTracksFromQueue() && gPlayer->PlayQueue(false) && gPlayer->AddURIToQueue(item, 1) && gPlayer->SeekTrack(1) && gPlayer->Play())
                  PERROR("Succeeded\n");
                else
                  PERROR("Failed\n");
              }
              else if (gPlayer->SetCurrentURI(item) && gPlayer->Play())
                PERROR("Succeeded\n");
              else
                PERROR("Failed\n");
            }
            else
              PERROR("Failed\n");
          }
          ++ic;
        }
      }
      else
        PERROR("Error: Missing arguments.\n");
    }
    else if (token == "SHOWSQ")
    {
      SONOS::ContentDirectory mycontent(gSonos->GetHost(), gSonos->GetPort());
      SONOS::ContentList bdir(mycontent, "SQ:");
      PRINT1("UpdateID  : %u\n", bdir.GetUpdateID());
      PRINT1("Item count: %u\n", bdir.size());
      SONOS::ContentList::iterator ic = bdir.begin();
      int i = 0;
      while (ic != bdir.end())
      {
        PRINT3("%d: [%s] [%s]\n", ++i, (*ic)->GetValue("dc:title").c_str(), (*ic)->GetValue("res").c_str());
        ++ic;
      }
    }
    else if (token == "PLAYSQ")
    {
      if (++it != tokens.end())
      {
        SONOS::ContentDirectory mycontent(gSonos->GetHost(), gSonos->GetPort());
        SONOS::ContentList bdir(mycontent, "SQ:");
        SONOS::ContentList::iterator ic = bdir.begin();
        while (ic != bdir.end())
        {
          if ((*ic)->GetValue("res") == (*it))
          {
            PRINT2("Playing item [%s] [%s]\n", (*ic)->GetValue("dc:title").c_str(), (*ic)->GetValue("res").c_str());
            if (gPlayer->RemoveAllTracksFromQueue() && gPlayer->PlayQueue(false) && gPlayer->AddURIToQueue(*ic, 1) && gPlayer->SeekTrack(1) && gPlayer->Play())
              PERROR("Succeeded\n");
            else
              PERROR("Failed\n");
          }
          ++ic;
        }
      }
      else
        PERROR("Error: Missing arguments.\n");
    }
    else if (token == "PLAYDIGITALIN")
    {
      if (gPlayer->PlayDigitalIN())
        PERROR("Succeeded\n");
      else
        PERROR("Failed\n");
    }
    else
    {
      PERROR("Error: Command invalid.\n");
    }
  }
  return true;
}

static void readStream(std::istream *file)
{
  while (*file)
  {
    std::string sline;
    if (gSonos->IsConnected() && gPlayer)
      PRINT1("%s >>> ", gPlayer->GetZone()->GetZoneName().c_str());
    else
      PRINT(">>> ");
    getline(*file, sline);
    if (!parseCommand(sline))
      break;
  }
}
