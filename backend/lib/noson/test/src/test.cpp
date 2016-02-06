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

#include <cstdio>
#include <string>
#include <cstdlib>

void handleEventCB(void* handle)
{
  fprintf(stderr, "#########################\n");
  fprintf(stderr, "### Handling event CB ###\n");
  fprintf(stderr, "#########################\n");
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
  std::string search = "Q:0";
  if (argc > 2)
    search.assign(argv[2]);


  SONOS::DBGLevel(3); // debug/proto

  {

    SONOS::System sonos;
    if (sonos.Discover())
    {
      fprintf(stderr, "Discovered !!!\n");

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

      /*
       * Print player infos
       */
      SONOS::ZonePlayerList players = sonos.GetZonePlayerList();
      for (SONOS::ZonePlayerList::const_iterator it = players.begin(); it != players.end(); ++it)
      {
        fprintf(stderr, "found player '%s' at location %s [%s]\n", it->second->c_str(), it->second->GetLocation().c_str(),
                it->second->GetIconName().c_str());
      }

      if (sonos.IsConnected())
      {
        SONOS::PlayerPtr playerPtr = sonos.GetPlayer();

        SONOS::ElementList vars;
        playerPtr->GetTransportInfo(vars);
        for(int i = 0; i < vars.size(); ++i)
          fprintf(stderr, "TransportInfo: %s : %s\n", vars[i]->GetKey().c_str(), vars[i]->c_str());

        SONOS::ContentDirectory mycontent(playerPtr->GetHost(), playerPtr->GetPort());
        SONOS::ContentList bdir(mycontent, search);
        fprintf(stderr, "UpdateID: %u\n", bdir.GetUpdateID());
        fprintf(stderr, "Item count: %u\n", bdir.size());
        SONOS::ContentList::iterator it = bdir.begin();
        int i = 0;
        while (it != bdir.end())
        {
          fprintf(stderr, "Item %d: [%d] [%s] [%s]\n", ++i, (*it)->IsItem(), (*it)->GetValue("dc:title").c_str(), (*it)->GetObjectID().c_str());
          ++it;
        }

        /*
         * Using class ContentBrowser to browse content ...
         *
        SONOS::ContentBrowser bdir2(mycontent, search);
        fprintf(stderr, "Item count: %u\n", bdir2.total());
        unsigned s = bdir2.index();
        while (s < bdir2.total() && bdir2.Browse(s, 100))
        {
          for (unsigned i = 0; i < bdir2.count(); ++i)
            fprintf(stderr, "Item %d: [%d] [%s]\n", i, bdir2.table()[i]->IsItem(), bdir2.table()[i]->GetValue("dc:title").c_str());
          s += bdir2.count();
        }
        */
      }
    }
  }

  //out:
#ifdef __WINDOWS__
  WSACleanup();
#endif /* __WINDOWS__ */
  return ret;
}
