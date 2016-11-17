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
#include "private/tinyxml2.h"

using namespace NSROOT;

const std::string MusicServices::Name("MusicServices");
const std::string MusicServices::ControlURL("/MusicServices/Control");
const std::string MusicServices::EventURL("/MusicServices/Event");
const std::string MusicServices::SCPDURL("/xml/MusicServices1.xml");

MusicServices::MusicServices(const std::string& serviceHost, unsigned servicePort)
: Service(serviceHost, servicePort)
{
}

bool MusicServices::ListAvailableServices(ElementList& vars)
{
  ElementList args;
  vars = Request("ListAvailableServices", args);
  if (!vars.empty() && vars[0]->compare("u:ListAvailableServicesResponse") == 0)
    return true;
  return false;
}


///////////////////////////////////////////////////////////////////////////////
////
//// MusicServiceList
////

const std::string& SMService::GetId() const
{
  return this->GetValue("Id");
}

const std::string& SMService::GetName() const
{
  return this->GetValue("Name");
}

const std::string& SMService::GetVersion() const
{
  return this->GetValue("Version");
}

const std::string& SMService::GetUri() const
{
  return this->GetValue("Uri");
}

const std::string& SMService::GetSecureUri() const
{
  return this->GetValue("SecureUri");
}

const std::string& SMService::GetContainerType() const
{
  return this->GetValue("ContainerType");
}

const std::string& SMService::GetCapabilities() const
{
  return this->GetValue("Capabilities");
}

ElementPtr SMService::GetPolicy() const
{
  ElementList::const_iterator it = this->FindKey("Policy");
  if (it != this->end())
    return (*it);
  return ElementPtr();
}

ElementPtr SMService::GetStrings() const
{
  ElementList::const_iterator it = this->FindKey("Strings");
  if (it != this->end())
    return (*it);
  return ElementPtr();
}

ElementPtr SMService::GetPresentationMap() const
{
  ElementList::const_iterator it = this->FindKey("PresentationMap");
  if (it != this->end())
    return (*it);
  return ElementPtr();
}

std::string SMService::GetServiceType() const
{
  int num = 0;
  if (string_to_int32(GetId().c_str(), &num) == 0)
    num = num * 256 + 7;
  char st[10];
  int32_to_string(num, st);
  return st;
}

MusicServiceList::MusicServiceList(MusicServices& service)
: m_succeeded(false)
, m_service(service)
, m_totalCount(0)
{
  m_succeeded = Browse();
}

bool MusicServiceList::Next(List::iterator& i)
{
  const List::iterator e(m_list.end());
  if (i != e)
  {
    ++i;
    return true;
  }
  return false;
}

bool MusicServiceList::Previous(List::iterator& i)
{
  if (i != m_list.begin())
  {
    --i;
    return true;
  }
  return false;
}

bool MusicServiceList::Browse()
{
  DBG(DBG_PROTO, "%s: browse\n", __FUNCTION__);
  ElementList vars;
  ElementList::const_iterator it;
  if ((m_service.ListAvailableServices(vars)) && ParseAvailableServiceDescriptorList(vars.GetValue("AvailableServiceDescriptorList")))
  {
      m_totalCount = m_list.size();
      DBG(DBG_PROTO, "%s: count %u\n", __FUNCTION__, m_totalCount);
      return true;
  }
  return false;
}

bool MusicServiceList::ParseAvailableServiceDescriptorList(const std::string& xml)
{
  tinyxml2::XMLDocument rootdoc;
  // Parse xml content
  if (rootdoc.Parse(xml.c_str(), xml.size()) != tinyxml2::XML_SUCCESS)
  {
    DBG(DBG_ERROR, "%s: parse xml failed\n", __FUNCTION__);
    return false;
  }
  tinyxml2::XMLElement* elem; // an element
  // Check for response: Services
  if (!(elem = rootdoc.RootElement()) || strncmp(elem->Name(), "Services", 8) != 0)
  {
    DBG(DBG_ERROR, "%s: invalid or not supported content\n", __FUNCTION__);
    tinyxml2::XMLPrinter out;
    rootdoc.Accept(&out);
    DBG(DBG_ERROR, "%s\n", out.CStr());
    return false;
  }
  m_list.clear();
  elem = elem->FirstChildElement();
  while (elem)
  {
    unsigned uid = 0; // unique item id
    char sid[10];
    memset(sid, '\0', sizeof(sid));
    const tinyxml2::XMLAttribute* attr = elem->FirstAttribute();
    SMServicePtr servicePtr(new SMService());
    while (attr)
    {
      servicePtr->push_back(ElementPtr(new Element(attr->Name(), attr->Value())));
      attr = attr->Next();
    }
    DBG(DBG_DEBUG, "%s: service '%s' (%s)\n", __FUNCTION__, servicePtr->GetValue("Name").c_str(), servicePtr->GetValue("Id").c_str());
    // browse childs
    tinyxml2::XMLElement* child = elem->FirstChildElement();
    while (child)
    {
      if (strncmp(child->Name(), "Policy", 6) == 0)
      {
        const tinyxml2::XMLAttribute* cattr = child->FirstAttribute();
        uint32_to_string(++uid, sid);
        ElementPtr policyPtr(new Element(child->Name(), sid));
        while (cattr)
        {
          policyPtr->SetAttribut(cattr->Name(), cattr->Value());
          cattr = cattr->Next();
        }
        servicePtr->push_back(policyPtr);
      }
      if (strncmp(child->Name(), "Presentation", 6) == 0)
      {
        tinyxml2::XMLElement* child2 = child->FirstChildElement();
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
          servicePtr->push_back(mapPtr);
          child2 = child2->NextSiblingElement(NULL);
        }
      }
      child = child->NextSiblingElement(NULL);
    }
    m_list.push_back(servicePtr);
    elem = elem->NextSiblingElement(NULL);
  }
  return true;
}
