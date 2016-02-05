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

#include "sonoszone.h"
#include "deviceproperties.h"
#include "private/uriparser.h"
#include "private/wsrequestbroker.h" // for Tokenize

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
    ParseLocation();
    DeviceProperties dp(m_host, m_port);
    ElementList vars;
    if (dp.GetZoneAttributes(vars))
    {
      std::string icon = vars.GetValue("CurrentIcon");
      size_t p = icon.find_last_of(":");
      if (p++ != std::string::npos)
        m_icon = icon.substr(p);
    }
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
        name.append("+");
      name.append(**it);
    }
  return name;
}

ZonePlayerPtr Zone::GetCoordinator() const
{
  for (const_iterator it = begin(); it != end(); ++it)
    if (*it && (*it)->GetAttribut("coordinator") == "true")
      return *it;
  return ZonePlayerPtr();
}
