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
#else
#include <unistd.h>
#include <sys/time.h>
#endif

#include "../backend/lib/noson/noson/src/private/debug.h"
#include "../backend/lib/noson/noson/src/sonossystem.h"
#include "../backend/lib/noson/noson/src/contentdirectory.h"
#include "../backend/lib/noson/noson/src/didlparser.h"

#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <string>

#include "../backend/lib/noson/noson/src/private/tokenizer.h"
#include "../backend/lib/noson/noson/src/private/builtin.h"

#define PRINT(a) fprintf(stdout, a)
#define PRINT1(a,b) fprintf(stdout, a, b)
#define PRINT2(a,b,c) fprintf(stdout, a, b, c)
#define PRINT3(a,b,c,d) fprintf(stdout, a, b, c, d)
#define PRINT4(a,b,c,d,e) fprintf(stdout, a, b, c, d, e)
#define ERROR(a) fprintf(stderr, a)
#define ERROR1(a,b) fprintf(stderr, a, b)
#define ERROR2(a,b,c) fprintf(stderr, a, b, c)
#define ERROR3(a,b,c,d) fprintf(stderr, a, b, c, d)

void handleEventCB(void* handle)
{
}

void readStream(std::istream*);

SONOS::System * gSonos = 0;

/*
 * the main function
 */
int main(int argc, char** argv)
{
  int ret = 0;
#ifdef __WINDOWS__
  //Initialize Winsock
  WSADATA wsaData;
  if ((ret = WSAStartup(MAKEWORD(2, 2), &wsaData)))
    return ret;
#endif /* __WINDOWS__ */
  
  PRINT1("Noson CLI using libnoson %s, Copyright (C) 2018 Jean-Luc Barriere\n", LIBVERSION);
  gSonos = new SONOS::System(0, handleEventCB);
  ERROR("Searching... ");
  if (!gSonos->Discover())
  {
    ERROR("No SONOS zone found.\n");
    return EXIT_FAILURE;
  }
  ERROR("Succeeded\n");
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
  
  delete gSonos;
  gSonos = 0;
  
#ifdef __WINDOWS__
  WSACleanup();
#endif /* __WINDOWS__ */
  return ret;
}

bool parseCommand(const std::string& line)
{
  std::vector<std::string> tokens;
  tokenize(line, " ", tokens, true);
  std::vector<std::string>::const_iterator it = tokens.begin();
  if (it != tokens.end())
  { 
    std::string token(*it);
    std::string::iterator c = token.begin();
    while (c != token.end())
    {
      *c = toupper(*c);
      ++c;
    }

    if (token == "EXIT")
      return false;
    else if (token == "")
    {}
    else if (token == "HELP")
    {
      PRINT("EXIT                        Exit from CLI\n");
      PRINT("CONNECT {zone name}         Connect to a zone for control\n");
      PRINT("PLAYURL {stream URL}        Play stream from URL\n");
      PRINT("PLAYFV {URL}                Play the given favorite\n");
      PRINT("PLAYSQ {URL}                Play the given playlist\n");
      PRINT("PLAYQUEUE                   Play queue\n");
      PRINT("PLAYLINEIN                  Play line-IN\n");
      PRINT("PLAYDIGITALIN               Play digital-IN/TV\n");
      PRINT("PLAY                        Press play\n");
      PRINT("PAUSE                       Press pause\n");
      PRINT("STOP                        Press stop\n");
      PRINT("PREVIOUS                    Press skip previous\n");
      PRINT("NEXT                        Press skip next\n");
      PRINT("SEEK 1..                    Seek to track number\n");
      PRINT("VOLUME 0..100               Set volume master\n");
      PRINT("VOLUME {player} 0..100      Set volume\n");
      PRINT("SHOWQUEUE                   Show queue content\n");
      PRINT("SHOWFV                      Show favorites\n");
      PRINT("SHOWSQ                      Show playlists\n");
      PRINT("HELP                        Print this help\n");
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
        for (SONOS::ZoneList::const_iterator iz = zones.begin(); iz != zones.end(); ++iz)
        {
          if (iz->second->GetZoneName() == param)
          {
            if (gSonos->ConnectZone(iz->second, 0, handleEventCB))
              ERROR1("Connected to zone %s\n", gSonos->GetConnectedZone()->GetZoneName().c_str());
            else
              ERROR("Failed\n");
            break;
          }
        }
      }
      else
        ERROR("Error: Missing arguments.\n");
    }
    else if (!gSonos->IsConnected())
    {
      ERROR("Error: Not connected.\n");
    }
    else if (token == "PLAY")
    {
      if (gSonos->GetPlayer()->Play())
        ERROR("Succeeded\n");
      else
        ERROR("Failed\n");
    }
    else if (token == "STOP")
    {
      if (gSonos->GetPlayer()->Stop())
        ERROR("Succeeded\n");
      else
        ERROR("Failed\n");
    }
    else if (token == "PAUSE")
    {
      if (gSonos->GetPlayer()->Pause())
        ERROR("Succeeded\n");
      else
        ERROR("Failed\n");
    }
    else if (token == "PREVIOUS")
    {
      if (gSonos->GetPlayer()->Previous())
        ERROR("Succeeded\n");
      else
        ERROR("Failed\n");
    }
    else if (token == "NEXT")
    {
      if (gSonos->GetPlayer()->Next())
        ERROR("Succeeded\n");
      else
        ERROR("Failed\n");
    }
    else if (token == "SEEK")
    {
      if (++it != tokens.end())
      {
        uint32_t value;
        string_to_uint32(it->c_str(), &value);
        if (gSonos->GetPlayer()->SeekTrack(value))
          ERROR("Succeeded\n");
        else
          ERROR("Failed\n");
      }
      else
        ERROR("Error: Missing arguments.\n");
    }
    else if (token == "PLAYURL")
    {
      if (++it != tokens.end())
      {
        std::string param(*it);
        while(++it != tokens.end())
          param.append(" ").append(*it);
        if (gSonos->GetPlayer()->PlayStream(*it, ""))
          ERROR("Succeeded\n");
        else
          ERROR("Failed\n");
      }
      else
        ERROR("Error: Missing arguments.\n");
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
        SONOS::ZonePtr pl = gSonos->GetConnectedZone();
        for (SONOS::Zone::iterator ip = pl->begin(); ip != pl->end(); ++ip)
        {
          if (all || param == **ip)
          {
            uint8_t value = 0;
            string_to_uint8(param2.c_str(), &value);
            if (gSonos->GetPlayer()->SetVolume((*ip)->GetUUID(), value))
              ERROR3("%s [%s]: volume %u\n", (*ip)->c_str(), (*ip)->GetUUID().c_str(), value);
            else
              ERROR("Failed\n");
          }
        }
      }
      else
        ERROR("Error: Missing arguments.\n");
    }
    else if (token == "SHOWQUEUE")
    {
      SONOS::ContentDirectory mycontent(gSonos->GetPlayer()->GetHost(), gSonos->GetPlayer()->GetPort());
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
      if (gSonos->GetPlayer()->PlayQueue(true))
        ERROR("Succeeded\n");
      else
        ERROR("Failed\n");
    }
    else if (token == "PLAYLINEIN")
    {
      if (gSonos->GetPlayer()->PlayLineIN())
        ERROR("Succeeded\n");
      else
        ERROR("Failed\n");
    }
    else if (token == "PLAYDIGITALIN")
    {
      if (gSonos->GetPlayer()->PlayDigitalIN())
        ERROR("Succeeded\n");
      else
        ERROR("Failed\n");
    }
    else if (token == "SHOWFV")
    {
      SONOS::ContentDirectory mycontent(gSonos->GetPlayer()->GetHost(), gSonos->GetPlayer()->GetPort());
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
        SONOS::ContentDirectory mycontent(gSonos->GetPlayer()->GetHost(), gSonos->GetPlayer()->GetPort());
        SONOS::ContentList bdir(mycontent, "FV:2");
        SONOS::ContentList::iterator ic = bdir.begin();
        while (ic != bdir.end())
        {
          if ((*ic)->GetValue("res") == (*it))
          {
            SONOS::DigitalItemPtr item;
            if (SONOS::System::ExtractObjectFromFavorite((*ic), item))
            {
              SONOS::PlayerPtr player = gSonos->GetPlayer();
              if (SONOS::System::CanQueueItem(item))
              {
                PRINT2("Playing item [%s] [%s]\n", (*ic)->GetValue("dc:title").c_str(), (*ic)->GetValue("res").c_str());
                if (player->RemoveAllTracksFromQueue() && player->PlayQueue(false) && player->AddURIToQueue(item, 1) && player->SeekTrack(1) && player->Play())
                  ERROR("Succeeded\n");
                else
                  ERROR("Failed\n");
              }
              else if (player->SetCurrentURI(item) && player->Play())
                ERROR("Succeeded\n");
              else
                ERROR("Failed\n");
            }
            else
              ERROR("Failed\n");
          }
          ++ic;
        }
      }
      else
        ERROR("Error: Missing arguments.\n");
    }
    else if (token == "SHOWSQ")
    {
      SONOS::ContentDirectory mycontent(gSonos->GetPlayer()->GetHost(), gSonos->GetPlayer()->GetPort());
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
        SONOS::ContentDirectory mycontent(gSonos->GetPlayer()->GetHost(), gSonos->GetPlayer()->GetPort());
        SONOS::ContentList bdir(mycontent, "SQ:");
        SONOS::ContentList::iterator ic = bdir.begin();
        while (ic != bdir.end())
        {
          if ((*ic)->GetValue("res") == (*it))
          {
            SONOS::PlayerPtr player = gSonos->GetPlayer();
            PRINT2("Playing item [%s] [%s]\n", (*ic)->GetValue("dc:title").c_str(), (*ic)->GetValue("res").c_str());
            if (player->RemoveAllTracksFromQueue() && player->PlayQueue(false) && player->AddURIToQueue(*ic, 1) && player->SeekTrack(1) && player->Play())
              ERROR("Succeeded\n");
            else
              ERROR("Failed\n");
          }
          ++ic;
        }
      }
      else
        ERROR("Error: Missing arguments.\n");
    }
    else if (token == "PLAYDIGITALIN")
    {
      if (gSonos->GetPlayer()->PlayDigitalIN())
        ERROR("Succeeded\n");
      else
        ERROR("Failed\n");
    }
    else
    {
      ERROR("Error: Command invalid.\n");
    }
  }
  return true;
}

void readStream(std::istream *file)
{
  while (*file)
  {
    std::string sline;
    if (gSonos->IsConnected())
      PRINT1("%s >>> ", gSonos->GetConnectedZone()->GetZoneName().c_str());
    else
      PRINT(">>> ");
    getline(*file, sline);
    if (!parseCommand(sline))
      break;
  }
}
