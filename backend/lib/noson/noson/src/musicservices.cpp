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

SMAccount::SMAccount()
: Element("Account")
, m_mutex(new OS::CMutex)
{
}

SMAccount::~SMAccount()
{
  SAFE_DELETE(m_mutex);
}

SMAccount::Credentials SMAccount::GetCredentials() const
{
  OS::CLockGuard lock(*m_mutex);
  return Credentials(GetAttribut("OADevID"), GetAttribut("Key"), GetAttribut("Token"));
}

void SMAccount::SetCredentials(const Credentials& auth)
{
  OS::CLockGuard lock(*m_mutex);
  SetAttribut("OADevID", auth.devId);
  SetAttribut("Key", auth.key);
  SetAttribut("Token", auth.token);
  // update keyring store for reuse on account reload
  SMOAKeyring::Store(GetType(), GetSerialNum(), auth.key, auth.token);
}

SMService::SMService(const std::string& agent, const SMAccountPtr& account, const ElementList& vars)
: m_agent(agent)
, m_account(account)
, m_vars(vars)
{
  ServiceType(GetId(), m_type);
  // make the sonos descriptor
  m_desc.assign("SA_RINCON").append(m_type).append("_").append(account->GetUsername());
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

const std::string& SMService::GetServiceDesc() const
{
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

SMServiceList MusicServices::GetEnabledServices()
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
    // load accounts
    SMAccountList accounts; // it is filled with retrieved accounts from the sonos device
    std::string agent; // it is filled with the SERVER tag from http response header
    if (!LoadAccounts(accounts, agent))
      DBG(DBG_ERROR, "%s: query accounts failed\n", __FUNCTION__);
    // Continue to treat services don't require any account

    // Fill the list of enabled services.
    // Service is enabled when an account is available for the service type.
    // Otherwise 'TuneIn' is special case as it is always enabled and no account exists for it.
    for (std::vector<ElementList>::const_iterator it = data.begin(); it != data.end(); ++it)
    {
      std::string serviceType;
      SMService::ServiceType(it->GetValue("Id"), serviceType);
      if (serviceType == "65031") /* TuneIn */
      {
        SMAccountPtr ac(new SMAccount());
        ac->SetAttribut("Type", serviceType);
        ac->SetAttribut("SerialNum", "0");
        SMServicePtr sm(new SMService(agent, ac, *it));
        list.push_back(sm);
      }
      else
      {
        SMAccountList la = GetAccountsForService(accounts, serviceType);
        for (SMAccountList::iterator ita = la.begin(); ita != la.end(); ++ita)
          list.push_back(SMServicePtr(new SMService(agent, *ita, *it)));
      }
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

bool MusicServices::LoadAccounts(SMAccountList& accounts, std::string& agentStr)
{
  WSRequest request(m_host, m_port);
  request.RequestService("/status/accounts");
  WSResponse response(request);
  if (!response.IsSuccessful() || !response.GetHeaderValue("SERVER", agentStr))
    return false;

  size_t len = 0, l = 0;
  std::string data;
  char buffer[4000];
  while ((l = response.ReadContent(buffer, sizeof(buffer))))
  {
    data.append(buffer, l);
    len += l;
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
  if (rootdoc.Parse(data.c_str(), len) != tinyxml2::XML_SUCCESS)
  {
    DBG(DBG_ERROR, "%s: parse xml failed\n", __FUNCTION__);
    return false;
  }
  const tinyxml2::XMLElement* elem; // an element
  // Check for response: Services
  if (!(elem = rootdoc.RootElement()) || !XMLNS::NameEqual(elem->Name(), "ZPSupportInfo")
          || !(elem = elem->FirstChildElement("Accounts")))
  {
    DBG(DBG_ERROR, "%s: invalid or not supported content\n", __FUNCTION__);
    tinyxml2::XMLPrinter out;
    rootdoc.Accept(&out);
    DBG(DBG_ERROR, "%s\n", out.CStr());
    return false;
  }
  accounts.clear();
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
      SMOAKeyring::Load(*item); // load Auth data from keyring if exist
      accounts.push_back(item);
    }
    elem = elem->NextSiblingElement(NULL);
  }
  return true;
}

SMAccountList MusicServices::GetAccountsForService(const SMAccountList& accounts, const std::string& serviceType)
{
  SMAccountList list;
  for (SMAccountList::const_iterator it = accounts.begin(); it != accounts.end(); ++it)
  {
    if ((*it)->GetType() == serviceType)
      list.push_back((*it));
  }
  return list;
}

////////////////////////////////////////////////////////////////////////////////
////
//// Store for AppLink keyring
////
SMOAKeyring::keyring new_keyring;
Locked<SMOAKeyring::keyring> SMOAKeyring::g_keyring(new_keyring);

void SMOAKeyring::Store(const std::string& type, const std::string& serialNum, const std::string& key, const std::string& token)
{
  // hold keyring lock until return
  Locked<SMOAKeyring::keyring>::pointer p = g_keyring.Get();
  for (keyring::iterator it = p->begin(); it != p->end(); ++it)
  {
    if (it->type == type && it->serialNum == serialNum)
    {
      it->key.assign(key);
      it->token.assign(token);
      return;
    }
  }
  p->push_back(Credentials(type, serialNum, key, token));
}

void SMOAKeyring::Load(SMAccount& account)
{
  const std::string& type = account.GetType();
  const std::string& serialNum = account.GetSerialNum();
  // hold keyring lock until return
  Locked<SMOAKeyring::keyring>::pointer p = g_keyring.Get();
  for (keyring::iterator it = p->begin(); it != p->end(); ++it)
  {
    if (it->type == type && it->serialNum == serialNum)
    {
      account.SetAttribut("Key", it->key);
      account.SetAttribut("Token", it->token);
      return;
    }
  }
}

void SMOAKeyring::Reset()
{
  g_keyring.Store(new_keyring);
}
