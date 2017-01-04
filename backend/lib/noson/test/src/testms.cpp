#if (defined(_WIN32) || defined(_WIN64))
#define __WINDOWS__
#endif

#ifdef __WINDOWS__
#include <winsock2.h>
#include <Windows.h>
#include <time.h>
//#define usleep(t) Sleep((DWORD)(t)/1000)
//#define sleep(t)  Sleep((DWORD)(t)*1000)
#else
#include <unistd.h>
#include <sys/time.h>
#endif

#include "../../noson/src/private/debug.h"
#include "../../noson/src/sonossystem.h"
#include "../../noson/src/contentdirectory.h"
#include "../../noson/src/avtransport.h"
#include "../../noson/src/musicservices.h"
#include "../../noson/src/smapi.h"
#include "../../noson/src/didlparser.h"

#include <cstdio>
#include <string>
#include <cstdlib>

#define PRINT(a) fprintf(stderr, a)
#define PRINT1(a,b) fprintf(stderr, a, b)
#define PRINT2(a,b,c) fprintf(stderr, a, b, c)
#define PRINT3(a,b,c,d) fprintf(stderr, a, b, c, d)
#define PRINT4(a,b,c,d,e) fprintf(stderr, a, b, c, d, e)


void handleEventCB(void* handle)
{
//  fprintf(stderr, "#########################\n");
//  fprintf(stderr, "### Handling event CB ###\n");
//  fprintf(stderr, "#########################\n");
}

int main(int argc, char** argv)
{
  int ret = 0;
#ifdef __WINDOWS__
  //Initialize Winsock
  WSADATA wsaData;
  if ((ret = WSAStartup(MAKEWORD(2, 2), &wsaData)))
    return ret;
#endif /* __WINDOWS__ */

  std::string tryzone;
  if (argc > 1)
    tryzone.assign(argv[1]);
  std::string tstServiceName;
  std::string tstServiceMediaId;
  if (argc > 2)
    tstServiceName.assign(argv[2]);
  if (argc > 3)
    tstServiceMediaId.assign(argv[3]);
  else
    tstServiceMediaId.assign("root");

  SONOS::DBGLevel(2); // debug/proto

  {

    SONOS::System sonos(0, handleEventCB);
    if (sonos.Discover())
    {
      PRINT("Discovered !!!\n");

      /*
       * Print Zones list and connect to
       */
      SONOS::ZoneList zones = sonos.GetZoneList();
      for (SONOS::ZoneList::const_iterator it = zones.begin(); it != zones.end(); ++it)
      {
        fprintf(stderr, "found zone '%s' with coordinator '%s'\n", it->second->GetZoneName().c_str(), it->second->GetCoordinator()->c_str());
        if (!sonos.IsConnected())
          if (tryzone.empty() || it->second->GetZoneName() == tryzone)
            sonos.ConnectZone(it->second, 0, handleEventCB);
      }

      if (sonos.IsConnected())
      {
        SONOS::PlayerPtr playerPtr = sonos.GetPlayer();

        /*
         * Music services
         */
        for (auto&& item : playerPtr->GetAvailableServices())
       	{
          PRINT3("MusicService: %s : %s , %s\n", item->GetName().c_str(), item->GetServiceType().c_str(), (item->GetPresentationMap() ? item->GetPresentationMap()->GetAttribut("Uri").c_str() : "No presentation map"));
          PRINT1("small icon  = %s\n", SONOS::System::GetLogoForService(item, "small").c_str());
          PRINT1("xlarge icon = %s\n", SONOS::System::GetLogoForService(item, "x-large").c_str());
          PRINT1("square icon = %s\n", SONOS::System::GetLogoForService(item, "square").c_str());
          PRINT("\n");

          if (item->GetName() == tstServiceName)
          {
            PRINT1("Testing service %s ...\n", item->GetName().c_str());
            SONOS::SMAPI sm(playerPtr);
            sm.Init(item);
            SONOS::SMAPIMetadata meta;
            SONOS::DBGLevel(2);
            if (!sm.GetMetadata(tstServiceMediaId, 0, 50, false, meta))
              sm.GetMediaMetadata(tstServiceMediaId, meta);
            //sm.Search("stations", "jazz", 0, 10, meta);
            SONOS::DBGLevel(2);
            for (auto&& data : meta.GetItems())
            {
              if (data.uriMetadata)
                PRINT1("%s\n", data.uriMetadata->DIDL().c_str());
              else
                PRINT1("%s\n", data.item->DIDL().c_str());
            }
          }
        }
      }
    }
  }

  //out:
#ifdef __WINDOWS__
  WSACleanup();
#endif /* __WINDOWS__ */
  return ret;
}
