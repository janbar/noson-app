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

#ifndef MAINPAGEBROKER_H
#define MAINPAGEBROKER_H

#include "../requestbroker.h"

#define MAINPAGEBROKER_CNAME    "[main-page]"
#define MAINPAGE_URI            "/"

namespace NSROOT
{

class MainPageBroker : public RequestBroker
{
public:
  MainPageBroker();
  ~MainPageBroker() override { }
  virtual bool HandleRequest(handle * handle) override;

  const char * CommonName() override { return MAINPAGEBROKER_CNAME; }
  RequestBroker::ResourcePtr GetResource(const std::string& title) override;
  RequestBroker::ResourceList GetResourceList() override;
  RequestBroker::ResourcePtr RegisterResource(const std::string& title,
                                              const std::string& description,
                                              const std::string& path,
                                              StreamReader * delegate) override;
  void UnregisterResource(const std::string& uri) override;

private:

  void ProcessGET(handle * handle);
  void ProcessHEAD(handle * handle);
};

}

#endif /* MAINPAGEBROKER_H */

