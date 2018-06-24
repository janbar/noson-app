/*
 *      Copyright (C) 2014-2018 Jean-Luc Barriere
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

#include "sonoszone.h"
#include "deviceproperties.h"
#include "private/uriparser.h"
#include "private/builtin.h"

#include <algorithm>

using namespace NSROOT;

ZonePlayer::ZonePlayer(const std::string& name)
: Element("ZonePlayer", name)
, m_URIparsed(false)
, m_host()
, m_port(0)
{
}

const std::string& ZonePlayer::GetHost()
{
  ParseLocation();
  return m_host;
}

unsigned ZonePlayer::GetPort()
{
  ParseLocation();
  return m_port;
}

bool ZonePlayer::ParseLocation()
{
  if (!m_URIparsed)
  {
    URIParser uri(GetLocation());
    if (uri.Scheme() && uri.Host() && uri.Port())
    {
      m_host = uri.Host();
      m_port = uri.Port();
      m_URIparsed = true;
    }
  }
  return m_URIparsed;
}

const std::string& ZonePlayer::GetIconName()
{
  if (m_icon.empty())
  {
    std::string icon = GetAttribut(ZP_ICON);
    size_t p = icon.find_last_of(":");
    if (p++ != std::string::npos)
      m_icon = icon.substr(p);
  }
  return m_icon;
}

std::string Zone::GetZoneName() const
{
  // Concat ZonePlayer: zp1 + zp2 + ...
  std::string name;
  for (const_iterator it = begin(); it != end(); ++it)
    if (*it)
    {
      if (!name.empty())
        name.append(" + ");
      name.append(**it);
    }
  return name;
}

std::string Zone::GetZoneShortName() const
{
  // Concat coordinator and subordinate count: zp1 + n
  std::string name;
  ZonePlayerPtr coordinator = GetCoordinator();
  if (!coordinator)
    return GetZoneName();
  name.append(*coordinator);
  if (size() > 1)
  {
    name.append(" + ").append(std::to_string((uint8_t)(size() - 1)));
  }
  return name;
}

ZonePlayerPtr Zone::GetCoordinator() const
{
  for (const_iterator it = begin(); it != end(); ++it)
    if (*it && (*it)->GetAttribut(ZP_COORDINATOR) == "true")
      return *it;
  return ZonePlayerPtr();
}

void Zone::Revamp()
{
  std::vector<ZonePlayerPtr> tmp(this->begin(), this->end());
  std::sort(tmp.begin(), tmp.end(), _compare);
  ZonePlayerPtr coordinator(GetCoordinator());
  this->clear();
  std::string cuuid = "";
  // push first the coordinator if any
  if (coordinator)
  {
    cuuid = coordinator->GetUUID();
    this->push_back(coordinator);
  }
  // push others in sorted order
  for (std::vector<ZonePlayerPtr>::const_iterator it = tmp.begin(); it != tmp.end(); ++it)
  {
    if (cuuid.empty() || (*it)->GetUUID() != cuuid)
      this->push_back(*it);
  }
}
