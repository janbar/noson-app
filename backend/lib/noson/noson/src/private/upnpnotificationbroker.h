/*
 *      Copyright (C) 2019 Jean-Luc Barriere
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef UPNPNOTIFICATIONBROKER_H
#define UPNPNOTIFICATIONBROKER_H

#include "../requestbroker.h"

#define UPNPNOTIFICATIONBROKER_CNAME    "[upnp-notification-broker]"
#define UPNPNOTIFICATIONBROKER_URI      "/"

namespace NSROOT
{

class UPNPNotificationBroker : public RequestBroker
{
public:
  UPNPNotificationBroker();
  ~UPNPNotificationBroker() override { }
  virtual bool HandleRequest(handle * handle) override;

  const char * CommonName() override { return UPNPNOTIFICATIONBROKER_CNAME; }
  RequestBroker::ResourcePtr GetResource(const std::string& title) override;
  RequestBroker::ResourceList GetResourceList() override;
  RequestBroker::ResourcePtr RegisterResource(const std::string& title,
                                              const std::string& description,
                                              const std::string& path,
                                              StreamReader * delegate) override;
  void UnregisterResource(const std::string& uri) override;

private:

  void Reply500(handle * handle);

  void Process(handle * handle);
};

}

#endif /* UPNPNOTIFICATIONBROKER_H */

