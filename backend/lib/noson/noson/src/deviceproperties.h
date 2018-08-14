/*
 *      Copyright (C) 2014-2016 Jean-Luc Barriere
 *
 *  This file is part of Noson
 *
 *  Noson is free software: you can redistribute it and/or modify
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

#ifndef DEVICEPROPERTIES_H
#define	DEVICEPROPERTIES_H

#include "local_config.h"
#include "service.h"

namespace NSROOT
{

  class DeviceProperties : public Service
  {
  public:
    DeviceProperties(const std::string& serviceHost, unsigned servicePort);
    ~DeviceProperties() {}

    static const std::string Name;
    static const std::string ControlURL;
    static const std::string EventURL;
    static const std::string SCPDURL;
    
    const std::string& GetName() const { return Name; }

    const std::string& GetControlURL() const { return ControlURL; }

    const std::string& GetEventURL() const { return EventURL; }

    const std::string& GetSCPDURL() const { return SCPDURL; }

    bool GetZoneInfo(ElementList& vars);

    bool GetZoneAttributes(ElementList& vars);

    bool GetHouseholdID(ElementList& vars);
  };
}

#endif	/* DEVICEPROPERTIES_H */

