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

#include "service.h"
#include "private/wsrequest.h"
#include "private/wsresponse.h"
#include "private/debug.h"
#include "private/cppdef.h"
#include "private/tinyxml2.h"
#include "private/os/threads/mutex.h"
#include "sonosplayer.h"

using namespace NSROOT;

Service::Service(const std::string& serviceHost, unsigned servicePort)
: m_host(serviceHost)
, m_port(servicePort)
, m_mutex(new OS::CMutex)
{
}

Service::~Service()
{
  m_mutex->Lock();
  SAFE_DELETE(m_mutex);
}

ElementList Service::GetLastFault()
{
  OS::CLockGuard lock(*m_mutex);
  return m_fault;
}

ElementList Service::Request(const std::string& action, const ElementList& args)
{
  ElementList vars;

  std::string soapaction;
  soapaction.append("\"urn:schemas-upnp-org:service:").append(GetName()).append(":1#").append(action).append("\"");

  std::string content;
  content.append(
    "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
    "<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">"
    "<s:Body>");
  content.append("<u:").append(action).append(" xmlns:u=\"urn:schemas-upnp-org:service:").append(GetName()).append(":1\">");
  for (ElementList::const_iterator it = args.begin(); it != args.end(); ++it)
    content.append((*it)->XML());
  content.append("</u:").append(action).append(">");
  content.append("</s:Body></s:Envelope>");

  WSRequest request(m_host, m_port);
  request.RequestService(GetControlURL(), HRM_POST);
  request.SetHeader("SOAPAction", soapaction);
  request.SetContentCustom(CT_XML, content.c_str());
  WSResponse response(request);

  if (!response.IsSuccessful())
  {
    DBG(DBG_ERROR, "%s: invalid response\n", __FUNCTION__);
    return vars;
  }

  // Receive content data
  size_t len = 0, l = 0;
  std::string data;
  char buffer[4096];
  while ((l = response.ReadContent(buffer, sizeof(buffer))))
  {
    data.append(buffer, l);
    len += l;
  }

  // Parse xml content
  tinyxml2::XMLDocument rootdoc;
  if (rootdoc.Parse(data.c_str(), len) != tinyxml2::XML_SUCCESS)
  {
    DBG(DBG_ERROR, "%s: parse xml failed\n", __FUNCTION__);
    return vars;
  }
  tinyxml2::XMLElement* elem; // an element
  // Check for response: s:Envelope/s:Body/{respTag}
  if (!(elem = rootdoc.RootElement()) ||
          !(elem = elem->FirstChildElement("s:Body")) ||
          !(elem = elem->FirstChildElement()))
  {
    DBG(DBG_ERROR, "%s: invalid or not supported response\n", __FUNCTION__);
    tinyxml2::XMLPrinter out;
    rootdoc.Accept(&out);
    DBG(DBG_ERROR, "%s\n", out.CStr());
    return vars;
  }
  vars.push_back(ElementPtr(new Element("TAG", elem->Name())));
  if (vars[0]->compare("s:Fault") == 0)
  {
    tinyxml2::XMLElement* felem;
    if ((felem = elem->FirstChildElement("faultstring")) && felem->GetText())
    {
      ElementPtr var(new Element(felem->Name(), felem->GetText()));
      vars.push_back(var);
      if (var->compare("UPnPError") == 0 &&
              (felem = elem->FirstChildElement("detail")) &&
              (felem = felem->FirstChildElement()) &&
              (felem = felem->FirstChildElement("errorCode")) &&
              (felem->GetText()))
        vars.push_back(ElementPtr(new Element(felem->Name(), felem->GetText())));
    }
    SetFault(vars);
  }
  else
  {
    elem = elem->FirstChildElement();
    while (elem)
    {
      if (elem->GetText())
      {
        ElementPtr var(new Element(elem->Name(), elem->GetText()));
        vars.push_back(var);
        DBG(DBG_PROTO, "%s: %s = %s\n", __FUNCTION__, var->GetKey().c_str(), var->c_str());
      }
      elem = elem->NextSiblingElement(NULL);
    }
  }
  return vars;
}

void Service::SetFault(const ElementList& vars)
{
  OS::CLockGuard lock(*m_mutex);
  m_fault = vars;
  for (ElementList::const_iterator it = vars.begin(); it != vars.end(); ++it)
    DBG(DBG_ERROR, "%s: %s (%s)\n", __FUNCTION__, (*it)->GetKey().c_str(), (*it)->c_str());
}
