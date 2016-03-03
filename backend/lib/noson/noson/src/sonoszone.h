/*
 *      Copyright (C) 2014-2015 Jean-Luc Barriere
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published
 *  by the Free Software Foundation; either version 3, or (at your option)
 *  any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this library; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 51 Franklin Street, Fifth Floor, Boston,
 *  MA 02110-1301 USA
 *  http://www.gnu.org/copyleft/gpl.html
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

