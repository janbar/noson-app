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


void usage(const char* cmd)
{
  fprintf(stderr,
        "Usage: %s [options]\n"
        "  --zone <zone name>         Connect to zone\n"
        "  --service <service name>   Testing for music service, default is 'TuneIn'\n"
        "  --search <media id>        Testing search for id, default is 'root'\n"
        "  --username <user name>     If policy auth is UserId: username\n"
        "  --password <password>      If policy auth is UserId: password\n"
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
  std::string tstServiceName = "TuneIn";
  std::string tstServiceMediaId = "root";
  std::string username = "";
  std::string password = "";

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
    else if (strcmp(argv[i], "--service") == 0 && ++i < argc)
    {
      fprintf(stderr, "service=%s, ", argv[i]);
      tstServiceName.assign(argv[i]);
    }
    else if (strcmp(argv[i], "--search") == 0 && ++i < argc)
    {
      fprintf(stderr, "search=%s, ", argv[i]);
      tstServiceMediaId.assign(argv[i]);
    }
    else if (strcmp(argv[i], "--username") == 0 && ++i < argc)
    {
      fprintf(stderr, "username=%s, ", argv[i]);
      username.assign(argv[i]);
    }
    else if (strcmp(argv[i], "--search") == 0 && ++i < argc)
    {
      fprintf(stderr, "password=%s, ", argv[i]);
      password.assign(argv[i]);
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

      if (sonos.IsConnected())
      {
        SONOS::PlayerPtr playerPtr = sonos.GetPlayer();

        /*
         * Music services
         */
        for (auto&& item : playerPtr->GetAvailableServices())
       	{
          PRINT3("MusicService: %s : %s , %s\n", item->GetName().c_str(), item->GetServiceType().c_str(), (item->GetPresentationMap() ? item->GetPresentationMap()->GetAttribut("Uri").c_str() : "No presentation map"));
          PRINT2("Policy      = Auth:%s , Poll:%s\n", item->GetPolicy()->GetAttribut("Auth").c_str(), item->GetPolicy()->GetAttribut("PollInterval").c_str());
          PRINT1("Capabilities= %s\n", item->GetCapabilities().c_str());
          PRINT1("small icon  = %s\n", SONOS::System::GetLogoForService(item, "small").c_str());
          PRINT1("xlarge icon = %s\n", SONOS::System::GetLogoForService(item, "x-large").c_str());
          PRINT1("square icon = %s\n", SONOS::System::GetLogoForService(item, "square").c_str());
          PRINT("\n");

          if (tstServiceName.empty() && item->GetName() == "TuneIn")
          {
            SONOS::DBGLevel((g_loglevel < 3 ? 3 : g_loglevel));
            PRINT1("Testing service %s ...\n", item->GetName().c_str());
            SONOS::SMAPI sm(playerPtr);
            sm.Init(item, "fr_FR");
            SONOS::SMAPIMetadata meta;
            PRINT1("\n...search stations for term '%s'\n", "jazz");
            sm.Search("stations", "jazz", 0, 10, meta);
            for (auto&& data : meta.GetItems())
            {
              PRINT2("item: %s, %s\n", data.item->GetObjectID().c_str(), data.item->GetValue("dc:title").c_str());
              if (data.uriMetadata)
                PRINT1("%s\n\n", data.uriMetadata->DIDL().c_str());
              else
                PRINT1("%s\n\n", data.item->DIDL().c_str());
            }
            SONOS::DBGLevel(g_loglevel);
          }
          if (item->GetName() == tstServiceName)
          {
            SONOS::DBGLevel((g_loglevel < 3 ? 3 : g_loglevel));
            PRINT1("Testing service %s ...\n", item->GetName().c_str());
            SONOS::SMAPI sm(playerPtr);
            sm.Init(item, "fr_FR");

            for (auto&& search : sm.AvailableSearchCategories())
              PRINT2("Search category: %s, %s\n", search->GetKey().c_str(), search->c_str());

            bool rs;
            SONOS::SMAPIMetadata meta;
            PRINT1("\n...browse id '%s'\n", tstServiceMediaId.c_str());
            if (!(rs = sm.GetMetadata(tstServiceMediaId, 0, 10, false, meta)))
              rs = sm.GetMediaMetadata(tstServiceMediaId, meta);
            if (!rs)
            {
              std::string regUrl;
              std::string linkCode;
              if (!sm.AuthTokenExpired())
                PRINT1("!!! Browsing failed for service %s !!!\n", item->GetName().c_str());
              else
              {
                SONOS::SMOAKeyring::Data auth;
                switch (sm.GetPolicyAuth())
                {
                case SONOS::SMAPI::Auth_UserId:
                  if (sm.GetSessionId(username, password, auth))
                  {
                    PRINT1("Session ID = %s\n", auth.key.c_str());
                    if (!(rs = sm.GetMetadata(tstServiceMediaId, 0, 10, false, meta)))
                      rs = sm.GetMediaMetadata(tstServiceMediaId, meta);
                  }
                  else
                    PRINT1("!!! Getting session token failed for service %s !!!\n", item->GetName().c_str());
                  break;

                case SONOS::SMAPI::Auth_DeviceLink:
                  if (sm.GetDeviceLinkCode(regUrl, linkCode))
                  {
                    PRINT1("Go to manual registration URL: %s\n", regUrl.c_str());
                    if (!linkCode.empty())
                      PRINT1("Link code: %s\n", linkCode.c_str());
                    while (sm.GetDeviceAuthToken(auth))
                    {
                      PRINT1("Retrying %s\n", "...");
                      sleep(5);
                    }
                    PRINT1("OAuth key   = %s\n", auth.key.c_str());
                    PRINT1("OAuth token = %s\n", auth.token.c_str());
                    if (!auth.key.empty())
                    {
                      if (!(rs = sm.GetMetadata(tstServiceMediaId, 0, 10, false, meta)))
                        rs = sm.GetMediaMetadata(tstServiceMediaId, meta);
                    }
                    else
                      PRINT1("!!! Getting auth token failed for service %s !!!\n", item->GetName().c_str());
                  }
                  break;

                case SONOS::SMAPI::Auth_AppLink:
                  if (sm.GetAppLink(regUrl, linkCode))
                  {
                    PRINT1("Go to manual registration URL: %s\n", regUrl.c_str());
                    if (!linkCode.empty())
                      PRINT1("Link code: %s\n", linkCode.c_str());
                    while (sm.GetDeviceAuthToken(auth))
                    {
                      PRINT1("Retrying %s\n", "...");
                      sleep(5);
                    }
                    PRINT1("OAuth key   = %s\n", auth.key.c_str());
                    PRINT1("OAuth token = %s\n", auth.token.c_str());
                    if (!auth.key.empty())
                    {
                      if (!(rs = sm.GetMetadata(tstServiceMediaId, 0, 10, false, meta)))
                        rs = sm.GetMediaMetadata(tstServiceMediaId, meta);
                    }
                    else
                      PRINT1("!!! Getting auth token failed for service %s !!!\n", item->GetName().c_str());
                  }
                  break;

                default:
                  break;
                }
              }
            }
            for (auto&& data : meta.GetItems())
            {
              PRINT2("item: %s, %s\n", data.item->GetObjectID().c_str(), data.item->GetValue("dc:title").c_str());
              if (data.uriMetadata)
                PRINT1("%s\n\n", data.uriMetadata->DIDL().c_str());
              else
                PRINT1("%s\n\n", data.item->DIDL().c_str());
            }
            SONOS::DBGLevel(g_loglevel);
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
