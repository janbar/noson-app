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

#include "musicservices.h"
#include "private/builtin.h"
#include "private/debug.h"
#include "private/cppdef.h"
#include "private/tinyxml2.h"
#include "private/xmldict.h"
#include "private/wsrequest.h"
#include "private/wsresponse.h"
#include "private/os/threads/mutex.h"

using namespace NSROOT;

const std::string MusicServices::Name("MusicServices");
const std::string MusicServices::ControlURL("/MusicServices/Control");
const std::string MusicServices::EventURL("/MusicServices/Event");
const std::string MusicServices::SCPDURL("/xml/MusicServices1.xml");

SMService::SMService(const std::string& agent, const ElementList& vars)
: m_agent(agent)
, m_vars(vars)
{
  m_type = ServiceType(GetId());
  m_account = SMAccountPtr(new SMAccount(m_type));
  m_desc.assign("");
}

SMService::SMService(const std::string& agent, const ElementList& vars, const std::string& serialNum)
: m_agent(agent)
, m_vars(vars)
{
  m_type = ServiceType(GetId());
  m_account = SMAccountPtr(new SMAccount(m_type, serialNum));
  m_desc.assign("");
}

SMServicePtr SMService::Clone(const std::string& serialNum)
{
  return SMServicePtr(new SMService(m_agent, m_vars, serialNum));
}

const std::string& SMService::GetId() const
{
  return m_vars.GetValue("Id");
}

const std::string& SMService::GetName() const
{
  return m_vars.GetValue("Name");
}

const std::string& SMService::GetVersion() const
{
  return m_vars.GetValue("Version");
}

const std::string& SMService::GetUri() const
{
  return m_vars.GetValue("Uri");
}

const std::string& SMService::GetSecureUri() const
{
  return m_vars.GetValue("SecureUri");
}

const std::string& SMService::GetContainerType() const
{
  return m_vars.GetValue("ContainerType");
}

const std::string& SMService::GetCapabilities() const
{
  return m_vars.GetValue("Capabilities");
}

ElementPtr SMService::GetPolicy() const
{
  ElementList::const_iterator it = m_vars.FindKey("Policy");
  if (it != m_vars.end())
    return (*it);
  return ElementPtr();
}

ElementPtr SMService::GetStrings() const
{
  ElementList::const_iterator it = m_vars.FindKey("Strings");
  if (it != m_vars.end())
    return (*it);
  return ElementPtr();
}

ElementPtr SMService::GetPresentationMap() const
{
  ElementList::const_iterator it = m_vars.FindKey("PresentationMap");
  if (it != m_vars.end())
    return (*it);
  return ElementPtr();
}

std::string SMService::ServiceType(const std::string& id)
{
  int num = 0;
  if (string_to_int32(id.c_str(), &num) == 0)
    num = num * 256 + 7;
  char st[12];
  int32_to_string(num, st);
  return std::string(st);
}

const std::string& SMService::GetServiceType() const
{
  return m_type;
}

const std::string& SMService::GetServiceDesc() const
{
  if (m_desc.empty())
  {
    m_desc.assign("SA_RINCON").append(m_type).append("_");
    const std::string& auth = GetPolicy()->GetAttribut("Auth");
    if (auth == "UserId")
    {
      m_desc.append(m_account->GetCredentials().username);
    }
    else if (auth == "DeviceLink")
    {
      //@FIXME: failed for service 'Bandcamp'
      m_desc.append("X_#Svc").append(m_type).append("-0-Token");
    }
    else if (auth == "AppLink")
    {
      m_desc.append("X_#Svc").append(m_type).append("-0-Token");
    }
  }
  return m_desc;
}

SMAccountPtr SMService::GetAccount() const
{
  return m_account;
}

const std::string& SMService::GetAgent() const
{
  return m_agent;
}

///////////////////////////////////////////////////////////////////////////////
////
//// MusicServices
////

MusicServices::MusicServices(const std::string& serviceHost, unsigned servicePort)
: Service(serviceHost, servicePort)
, m_version("")
{
}

bool MusicServices::GetSessionId(const std::string& serviceId, const std::string& username, ElementList& vars)
{
  ElementList args;
  args.push_back(ElementPtr(new Element("ServiceId", serviceId)));
  args.push_back(ElementPtr(new Element("Username", username)));
  vars = Request("GetSessionId", args);
  if (!vars.empty() && vars[0]->compare("GetSessionIdResponse") == 0)
    return true;
  return false;
}

SMServiceList MusicServices::GetAvailableServices()
{
  // hold version's lock until return
  Locked<std::string>::pointer versionPtr = m_version.Get();
  SMServiceList list;
  // load services
  ElementList vars;
  std::vector<ElementList> data;
  if (!ListAvailableServices(vars) || !ParseAvailableServices(vars, data))
    DBG(DBG_ERROR, "%s: query services failed\n", __FUNCTION__);
  else
  {
    // store new value of version
    versionPtr->assign(vars.GetValue("AvailableServiceListVersion"));
    std::string agent;
    //@FIXME make the user agent string according to the template: Linux UPnP/1.0 Sonos/26.99-12345
    //Resolved by SoCo: https://github.com/SoCo/SoCo/blob/18ee1ec11bba8463c4536aa7c2a25f5c20a051a4/soco/music_services/music_service.py#L55
    agent.assign("Linux UPnP/1.0 Sonos/26.99-12345");

    // Fill the list of services.
    for (std::vector<ElementList>::const_iterator it = data.begin(); it != data.end(); ++it)
    {
        list.push_back(SMServicePtr(new SMService(agent, *it)));
    }
  }
  DBG(DBG_DEBUG, "%s: version (%s)\n", __FUNCTION__, versionPtr->c_str());
  return list;
}

bool MusicServices::ListAvailableServices(ElementList& vars)
{
  ElementList args;
  vars = Request("ListAvailableServices", args);
  if (!vars.empty() && vars[0]->compare("ListAvailableServicesResponse") == 0)
    return true;
  return false;
}

bool MusicServices::ParseAvailableServices(const ElementList& vars, std::vector<ElementList>& data)
{
  const std::string& xml = vars.GetValue("AvailableServiceDescriptorList");
  tinyxml2::XMLDocument rootdoc;
  // Parse xml content
  if (rootdoc.Parse(xml.c_str(), xml.size()) != tinyxml2::XML_SUCCESS)
  {
    DBG(DBG_ERROR, "%s: parse xml failed\n", __FUNCTION__);
    return false;
  }
  const tinyxml2::XMLElement* elem; // an element
  // Check for response: Services
  if (!(elem = rootdoc.RootElement()) || strncmp(elem->Name(), "Services", 8) != 0)
  {
    DBG(DBG_ERROR, "%s: invalid or not supported content\n", __FUNCTION__);
    tinyxml2::XMLPrinter out;
    rootdoc.Accept(&out);
    DBG(DBG_ERROR, "%s\n", out.CStr());
    return false;
  }
  data.clear();
  elem = elem->FirstChildElement();
  while (elem)
  {
    unsigned uid = 0; // unique item id
    char sid[12];
    memset(sid, '\0', sizeof(sid));
    const tinyxml2::XMLAttribute* attr = elem->FirstAttribute();
    ElementList service;
    while (attr)
    {
      service.push_back(ElementPtr(new Element(attr->Name(), attr->Value())));
      attr = attr->Next();
    }
    DBG(DBG_DEBUG, "%s: service '%s' (%s)\n", __FUNCTION__, service.GetValue("Name").c_str(), service.GetValue("Id").c_str());
    // browse childs
    const tinyxml2::XMLElement* child = elem->FirstChildElement();
    while (child)
    {
      if (XMLNS::NameEqual(child->Name(), "Policy"))
      {
        const tinyxml2::XMLAttribute* cattr = child->FirstAttribute();
        uint32_to_string(++uid, sid);
        ElementPtr policyPtr(new Element(child->Name(), sid));
        while (cattr)
        {
          policyPtr->SetAttribut(cattr->Name(), cattr->Value());
          cattr = cattr->Next();
        }
        service.push_back(policyPtr);
      }
      if (XMLNS::NameEqual(child->Name(), "Presentation"))
      {
        const tinyxml2::XMLElement* child2 = child->FirstChildElement();
        while (child2)
        {
          const tinyxml2::XMLAttribute* cattr = child2->FirstAttribute();
          uint32_to_string(++uid, sid);
          ElementPtr mapPtr(new Element(child2->Name(), sid));
          while (cattr)
          {
            mapPtr->SetAttribut(cattr->Name(), cattr->Value());
            cattr = cattr->Next();
          }
          service.push_back(mapPtr);
          child2 = child2->NextSiblingElement(NULL);
        }
      }
      child = child->NextSiblingElement(NULL);
    }
    data.push_back(service);
    elem = elem->NextSiblingElement(NULL);
  }
  return true;
}
