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

#ifndef SONOSZONE_H
#define	SONOSZONE_H

#include <local_config.h>
#include "sharedptr.h"
#include "element.h"

#include <map>
#include <vector>

#define ZP_UUID         "uuid"
#define ZP_LOCATION     "location"
#define ZP_ICON         "icon"
#define ZP_VERSION      "version"
#define ZP_MCVERSION    "mincompatibleversion"
#define ZP_LCVERSION    "legacycompatibleversion"
#define ZP_COORDINATOR  "coordinator"

namespace NSROOT
{
  class ZonePlayer;

  typedef SHARED_PTR<ZonePlayer> ZonePlayerPtr;
  typedef std::map<std::string, ZonePlayerPtr> ZonePlayerList;

  class ZonePlayer : public Element
  {
  public:
    ZonePlayer(const std::string& name);
    virtual ~ZonePlayer() {};

    const std::string& GetUUID() { return GetAttribut(ZP_UUID); }

    const std::string& GetLocation() { return GetAttribut(ZP_LOCATION); }

    bool IsValid() { return ParseLocation(); }

    const std::string& GetHost();

    unsigned GetPort();

    const std::string& GetIconName();

  private:
    bool m_URIparsed;
    std::string m_host;
    unsigned m_port;
    std::string m_icon;
    
    bool ParseLocation();
  };

  class Zone;

  typedef SHARED_PTR<Zone> ZonePtr;
  typedef std::map<std::string, ZonePtr> ZoneList;

  class Zone : public std::vector<ZonePlayerPtr>
  {
  public:
    Zone() {}
    Zone(const std::string& group) : m_group(group) {}
    virtual ~Zone() {}

    const std::string& GetGroup() const { return m_group; }

    std::string GetZoneName() const;

    std::string GetZoneShortName() const;

    ZonePlayerPtr GetCoordinator() const;

    void Revamp();

  private:
    std::string m_group;

    static bool _compare(const ZonePlayerPtr& first, const ZonePlayerPtr& last)
    {
      return first->compare(*last) < 0 ? true : false;
    }
  };
}

#endif	/* SONOSZONE_H */

