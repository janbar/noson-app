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
  fprintf(stderr, "#########################\n");
  fprintf(stderr, "### Handling event CB ###\n");
  fprintf(stderr, "#########################\n");
}

void usage(const char* cmd)
{
  fprintf(stderr,
        "Usage: %s [options]\n"
        "  --zone <zone name>         Connect to zone\n"
        "  --search <media id>        Testing search for id, default is 'Q:0'\n"
        "  --debug                    Enable debug output\n"
        "  --help                     print this help\n"
        "\n", cmd
        );
}

static int g_loglevel = 2;

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
  std::string search = "Q:0";

  int i = 0;
  while (++i < argc)
  {
    if (strcmp(argv[i], "--debug") == 0)
    {
      g_loglevel = 4;
      fprintf(stderr, "debug=Yes, ");
    }
    else if (strcmp(argv[i], "--zone") == 0 && ++i < argc)
    {
      fprintf(stderr, "zone=%s, ", argv[i]);
      tryzone.assign(argv[i]);
    }
    else if (strcmp(argv[i], "--search") == 0 && ++i < argc)
    {
      fprintf(stderr, "search=%s, ", argv[i]);
      search.assign(argv[i]);
    }
    else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0)
    {
      usage(argv[0]);
      return 0;
    }
  }
  fprintf(stderr, "\n");
  SONOS::DBGLevel(g_loglevel);

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

      /*
       * Print player infos
       */
      SONOS::ZonePlayerList players = sonos.GetZonePlayerList();
      for (SONOS::ZonePlayerList::const_iterator it = players.begin(); it != players.end(); ++it)
      {
        PRINT3("found player '%s' at location %s [%s]\n", it->second->c_str(), it->second->GetLocation().c_str(),
                it->second->GetIconName().c_str());
      }

      if (sonos.IsConnected())
      {
        SONOS::PlayerPtr playerPtr = sonos.GetPlayer();

        SONOS::ElementList vars;
        playerPtr->GetTransportInfo(vars);
        for(int i = 0; i < vars.size(); ++i)
          PRINT2("TransportInfo: %s : %s\n", vars[i]->GetKey().c_str(), vars[i]->c_str());

        SONOS::ContentDirectory mycontent(playerPtr->GetHost(), playerPtr->GetPort());
        SONOS::ContentList bdir(mycontent, search);
        PRINT1("UpdateID: %u\n", bdir.GetUpdateID());
        PRINT1("Item count: %u\n", bdir.size());
        SONOS::ContentList::iterator it = bdir.begin();
        int i = 0;
        while (it != bdir.end())
        {
          PRINT4("Item %d: [%d] [%s] [%s]\n", ++i, (*it)->IsItem(), (*it)->GetValue("dc:title").c_str(), (*it)->GetObjectID().c_str());
          PRINT3("     %d: %s, %s\n", i, (*it)->GetValue("res").c_str(), (*it)->GetProperty("res")->GetAttribut("protocolInfo").c_str());
          SONOS::DigitalItemPtr payload;
          if (SONOS::System::ExtractObjectFromFavorite(*it, payload))
            PRINT2("   F %d: %s\n", i, payload->GetObjectID().c_str());
          else
            PRINT2("   I %d: %s\n", i, playerPtr->GetItemIdFromUriMetadata(*it).c_str());
          ++it;
        }

        /*
         * Using class ContentBrowser to browse content ...
         *
        SONOS::ContentBrowser bdir2(mycontent, search);
        PRINT1("Item count: %u\n", bdir2.total());
        unsigned s = bdir2.index();
        while (s < bdir2.total() && bdir2.Browse(s, 100))
        {
          for (unsigned i = 0; i < bdir2.count(); ++i)
            PRINT3("Item %d: [%d] [%s]\n", i, bdir2.table()[i]->IsItem(), bdir2.table()[i]->GetValue("dc:title").c_str());
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
