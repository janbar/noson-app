/*
 *      Copyright (C) 2016 Jean-Luc Barriere
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

#include "musicservices.h"
#include "private/builtin.h"
#include "private/debug.h"
#include "private/cppdef.h"
#include "private/tinyxml2.h"
#include "private/xmlname.h"
#include "private/wsrequest.h"
#include "private/wsresponse.h"
#include "private/os/threads/mutex.h"

using namespace NSROOT;

const std::string MusicServices::Name("MusicServices");
const std::string MusicServices::ControlURL("/MusicServices/Control");
const std::string MusicServices::EventURL("/MusicServices/Event");
const std::string MusicServices::SCPDURL("/xml/MusicServices1.xml");

SMAccount::SMAccount()
: Element("Account")
, m_mutex(new OS::CMutex)
{
}

SMAccount::~SMAccount()
{
  SAFE_DELETE(m_mutex);
}

SMAccount::OACredentials SMAccount::GetOACredentials() const
{
  OS::CLockGuard lock(*m_mutex);
  return std::make_pair(GetAttribut("OADevID"), GetAttribut("Key"));
}

void SMAccount::SetOACredentials(OACredentials auth)
{
  OS::CLockGuard lock(*m_mutex);
  SetAttribut("OADevID", auth.first);
  SetAttribut("Key", auth.second);
}

SMService::SMService(const std::string& agent, const SMAccountPtr& account, const ElementList& vars)
: m_agent(agent)
, m_account(account)
, m_vars(vars)
{
  ServiceType(GetId(), m_type);
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

void SMService::ServiceType(const std::string& id, std::string& type)
{
  int num = 0;
  if (string_to_int32(id.c_str(), &num) == 0)
    num = num * 256 + 7;
  char st[10];
  int32_to_string(num, st);
  type.assign(st);
}

const std::string& SMService::GetServiceType() const
{
  return m_type;
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
{
}

bool MusicServices::GetSessionId(const std::string& serviceId, const std::string& username, ElementList& vars)
{
  ElementList args;
  args.push_back(ElementPtr(new Element("ServiceId", serviceId)));
  args.push_back(ElementPtr(new Element("Username", username)));
  vars = Request("GetSessionId", args);
  if (!vars.empty() && vars[0]->compare("u:GetSessionIdResponse") == 0)
    return true;
  return false;
}

SMServiceList MusicServices::GetEnabledServices()
{
  SMServiceList list;
  ElementList vars;
  if ((ListAvailableServices(vars)) && ParseAvailableServiceDescriptorList(vars.GetValue("AvailableServiceDescriptorList")))
  {
    if (!ListAccounts())
      DBG(DBG_ERROR, "%s: query accounts failed\n");
    for (std::list<ElementList>::const_iterator it = m_services.begin(); it != m_services.end(); ++it)
    {
      std::string serviceType;
      SMService::ServiceType(it->GetValue("Id"), serviceType);
      ElementList::const_iterator policy = it->FindKey("Policy");
      if (policy != it->end() && (*policy)->GetAttribut("Auth") == "Anonymous")
      {
        SMAccountPtr ac(new SMAccount());
        ac->SetAttribut("Type", serviceType);
        ac->SetAttribut("SerialNum", "0");
        SMServicePtr sm(new SMService(m_agent, ac, *it));
        list.push_back(sm);
      }
      else
      {
        SMAccountList la = GetAccountsForService(serviceType);
        if (!la.empty())
          list.push_back(SMServicePtr(new SMService(m_agent, la.front(), *it)));
      }
    }
  }
  return list;
}

bool MusicServices::ListAvailableServices(ElementList& vars)
{
  ElementList args;
  vars = Request("ListAvailableServices", args);
  if (!vars.empty() && vars[0]->compare("u:ListAvailableServicesResponse") == 0)
    return true;
  return false;
}

bool MusicServices::ListAccounts()
{
  WSRequest request(m_host, m_port);
  request.RequestService("/status/accounts");
  WSResponse response(request);
  if (!response.IsSuccessful() || !response.GetHeaderValue("SERVER", m_agent))
    return false;
  size_t s = response.GetContentLength();
  std::string data;
  size_t l = 0;
  char buf[4000];
  if (s)
  {
    data.reserve(s);
    while (response.GetConsumed() < s && (l = response.ReadContent(buf, sizeof(buf))))
      data.append(buf, l);
  }
  else
  {
    data.reserve(1000);
    while ((l = response.ReadContent(buf, sizeof(buf))))
      data.append(buf, l);
  }

  /*
    <ZPSupportInfo>
    <Accounts LastUpdateDevice="RINCON_000XXXXXXXXXX1400" Version="4" NextSerialNum="2">
    <Account Type="519" SerialNum="1" Deleted="1">
    <UN>username</UN>
    <MD>1</MD>
    <NN>nickname</NN>
    <OADevID></OADevID>
    <Key></Key>
    </Account>
    </Accounts>
    </ZPSupportInfo>
  */
  tinyxml2::XMLDocument rootdoc;
  // Parse xml content
  if (rootdoc.Parse(data.c_str(), data.size()) != tinyxml2::XML_SUCCESS)
  {
    DBG(DBG_ERROR, "%s: parse xml failed\n", __FUNCTION__);
    return false;
  }
  const tinyxml2::XMLElement* elem; // an element
  // Check for response: Services
  if (!(elem = rootdoc.RootElement()) || !XMLName::XMLNameEqual(elem->Name(), "ZPSupportInfo")
          || !(elem = elem->FirstChildElement("Accounts")))
  {
    DBG(DBG_ERROR, "%s: invalid or not supported content\n", __FUNCTION__);
    tinyxml2::XMLPrinter out;
    rootdoc.Accept(&out);
    DBG(DBG_ERROR, "%s\n", out.CStr());
    return false;
  }
  m_accounts.clear();
  elem = elem->FirstChildElement("Account");
  while (elem)
  {
    SMAccountPtr item(new SMAccount());
    const tinyxml2::XMLAttribute* attr = elem->FirstAttribute();
    while (attr)
    {
      item->SetAttribut(attr->Name(), attr->Value());
      attr = attr->Next();
    }
    const tinyxml2::XMLElement* child = elem->FirstChildElement(NULL);
    while (child)
    {
      if (child->GetText())
        item->SetAttribut(child->Name(), child->GetText());
      child = child->NextSiblingElement(NULL);
    }
    if (item->GetAttribut("Deleted") == "1")
      DBG(DBG_DEBUG, "%s: account %s (%s) is deleted\n", __FUNCTION__, item->GetSerialNum().c_str(), item->GetType().c_str());
    else
    {
      DBG(DBG_DEBUG, "%s: account %s (%s) is available\n", __FUNCTION__, item->GetSerialNum().c_str(), item->GetType().c_str());
      m_accounts.push_back(item);
    }
    elem = elem->NextSiblingElement(NULL);
  }
  return true;
}

SMAccountList MusicServices::GetAccountsForService(const std::string& serviceType) const
{
  SMAccountList list;
  for (SMAccountList::const_iterator it = m_accounts.begin(); it != m_accounts.end(); ++it)
  {
    if ((*it)->GetType() == serviceType)
    list.push_back((*it));
  }
  return list;
}

bool MusicServices::ParseAvailableServiceDescriptorList(const std::string& xml)
{
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
  m_services.clear();
  elem = elem->FirstChildElement();
  while (elem)
  {
    unsigned uid = 0; // unique item id
    char sid[10];
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
      if (XMLName::XMLNameEqual(child->Name(), "Policy"))
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
      if (XMLName::XMLNameEqual(child->Name(), "Presentation"))
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
    m_services.push_back(service);
    elem = elem->NextSiblingElement(NULL);
  }
  return true;
}
