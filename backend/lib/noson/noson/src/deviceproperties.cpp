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

#include "deviceproperties.h"
#include "private/builtin.h"

using namespace NSROOT;

const std::string DeviceProperties::Name("DeviceProperties");
const std::string DeviceProperties::ControlURL("/DeviceProperties/Control");
const std::string DeviceProperties::EventURL("/DeviceProperties/Event");
const std::string DeviceProperties::SCPDURL("/xml/DeviceProperties1.xml");

DeviceProperties::DeviceProperties(const std::string& serviceHost, unsigned servicePort)
: Service(serviceHost, servicePort)
{
}

bool DeviceProperties::GetZoneInfo(ElementList& vars)
{
  ElementList args;
  vars = Request("GetZoneInfo", args);
  if (!vars.empty() && vars[0]->compare("u:GetZoneInfoResponse") == 0)
    return true;
  return false;
}

bool DeviceProperties::GetZoneAttributes(ElementList& vars)
{
  ElementList args;
  vars = Request("GetZoneAttributes", args);
  if (!vars.empty() && vars[0]->compare("u:GetZoneAttributesResponse") == 0)
    return true;
  return false;
}

bool DeviceProperties::GetHouseholdID(ElementList& vars)
{
  ElementList args;
  vars = Request("GetHouseholdID", args);
  if (!vars.empty() && vars[0]->compare("u:GetHouseholdIDResponse") == 0)
    return true;
  return false;
}
