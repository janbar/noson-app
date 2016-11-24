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
#include "private/xmlname.h"
#include "private/os/threads/mutex.h"
#include "sonosplayer.h"

#define UPNP_SERVICE_NAMESPACE  "urn:schemas-upnp-org:service"
#define SOAP_ENVELOPE_NAMESPACE "http://schemas.xmlsoap.org/soap/envelope/"
#define SOAP_ENCODING_NAMESPACE "http://schemas.xmlsoap.org/soap/encoding/"

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
  soapaction.append("\"" UPNP_SERVICE_NAMESPACE ":").append(GetName()).append(":1#").append(action).append("\"");

  std::string content;
  content.append("<?xml version=\"1.0\" encoding=\"utf-8\"?>");
  // start envelope
  content.append("<s:Envelope xmlns:s=\"" SOAP_ENVELOPE_NAMESPACE "\" s:encodingStyle=\"" SOAP_ENCODING_NAMESPACE "\">");
  // start body
  content.append("<s:Body>");
  content.append("<u:").append(action).append(" xmlns:u=\"" UPNP_SERVICE_NAMESPACE ":").append(GetName()).append(":1\">");
  for (ElementList::const_iterator it = args.begin(); it != args.end(); ++it)
    content.append((*it)->XML());
  content.append("</u:").append(action).append(">");
  // end body
  content.append("</s:Body>");
  // end envelope
  content.append("</s:Envelope>");

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
  const tinyxml2::XMLElement* elem; // an element
  // Check for response: s:Envelope/s:Body/{respTag}
  if (!(elem = rootdoc.RootElement()) || !XMLName::XMLNameEqual(elem->Name(), "Envelope") ||
          !(elem = elem->FirstChildElement()) || !XMLName::XMLNameEqual(elem->Name(), "Body") ||
          !(elem = elem->FirstChildElement()))
  {
    DBG(DBG_ERROR, "%s: invalid or not supported response\n", __FUNCTION__);
    tinyxml2::XMLPrinter out;
    rootdoc.Accept(&out);
    DBG(DBG_ERROR, "%s\n", out.CStr());
    return vars;
  }
  vars.push_back(ElementPtr(new Element("TAG", elem->Name())));
  if (XMLName::XMLNameEqual(vars.back()->c_str(), "Fault"))
  {
    const tinyxml2::XMLElement* felem;
    if ((felem = elem->FirstChildElement("faultstring")) && felem->GetText())
    {
      vars.push_back(ElementPtr(new Element(felem->Name(), felem->GetText())));
      if (vars.back()->compare("UPnPError") == 0 &&
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
    elem = elem->FirstChildElement(NULL);
    while (elem)
    {
      if (elem->GetText())
      {
        vars.push_back(ElementPtr(new Element(elem->Name(), elem->GetText())));
        DBG(DBG_PROTO, "%s: %s = %s\n", __FUNCTION__, vars.back()->GetKey().c_str(), vars.back()->c_str());
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
