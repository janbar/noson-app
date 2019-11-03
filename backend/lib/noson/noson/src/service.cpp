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
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "service.h"
#include "private/wsrequest.h"
#include "private/wsresponse.h"
#include "private/debug.h"
#include "private/cppdef.h"
#include "private/tinyxml2.h"
#include "private/xmldict.h"
#include "private/os/threads/mutex.h"
#include "sonosplayer.h"

#define NS_PREFIX               "urn:schemas-upnp-org:service:"
#define NS_SUFFIX               ":1"
#define SOAP_ENVELOPE_NAMESPACE "http://schemas.xmlsoap.org/soap/envelope/"
#define SOAP_ENCODING_NAMESPACE "http://schemas.xmlsoap.org/soap/encoding/"

using namespace NSROOT;

namespace NSROOT
{
  void __traceServiceError(tinyxml2::XMLDocument& doc)
  {
    DBG(DBG_ERROR, "%s: invalid or not supported response\n", __FUNCTION__);
    tinyxml2::XMLPrinter out;
    doc.Accept(&out);
    DBG(DBG_ERROR, "%s\n", out.CStr());
  }
}

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
  soapaction.append("\"" NS_PREFIX).append(GetName()).append(NS_SUFFIX "#").append(action).append("\"");

  std::string content;
  content.append("<?xml version=\"1.0\" encoding=\"utf-8\"?>");
  // start envelope
  content.append("<s:Envelope xmlns:s=\"" SOAP_ENVELOPE_NAMESPACE "\" s:encodingStyle=\"" SOAP_ENCODING_NAMESPACE "\">");
  // start body
  content.append("<s:Body>");
  content.append("<u:").append(action).append(" xmlns:u=\"" NS_PREFIX).append(GetName()).append(NS_SUFFIX "\">");
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
  // Check for response: Envelope/Body/{respTag}
  if (!(elem = rootdoc.RootElement()) || !XMLNS::NameEqual(elem->Name(), "Envelope"))
  {
    __traceServiceError(rootdoc);
    SetFault(vars);
    return vars;
  }

  // search the element 'Body'
  elem = elem->FirstChildElement();
  while (elem && !XMLNS::NameEqual(elem->Name(), "Body"))
    elem = elem->NextSiblingElement(NULL);

  if (!elem || !(elem = elem->FirstChildElement()))
  {
    __traceServiceError(rootdoc);
    SetFault(vars);
    return vars;
  }
  vars.push_back(ElementPtr(new Element("TAG", XMLNS::LocalName(elem->Name()))));
  if (vars.back()->compare("Fault") == 0)
  {
    const tinyxml2::XMLElement* felem = elem->FirstChildElement(NULL);
    while (felem)
    {
      if (XMLNS::NameEqual(felem->Name(), "faultcode") && felem->GetText())
        vars.push_back(ElementPtr(new Element("faultcode", felem->GetText())));
      else if (XMLNS::NameEqual(felem->Name(), "faultstring") && felem->GetText())
        vars.push_back(ElementPtr(new Element("faultstring", felem->GetText())));
      else if (XMLNS::NameEqual(felem->Name(), "detail"))
      {
        const tinyxml2::XMLElement* delem = felem->FirstChildElement(NULL);
        if (delem && (delem = delem->FirstChildElement(NULL)))
        {
          do
          {
            if (delem->GetText())
            {
              // remove the namespace qualifier to handle local name as key
              vars.push_back(ElementPtr(new Element(XMLNS::LocalName(delem->Name()), delem->GetText())));
              DBG(DBG_PROTO, "%s: [fault] %s = %s\n", __FUNCTION__, vars.back()->GetKey().c_str(), vars.back()->c_str());
            }
            delem = delem->NextSiblingElement(NULL);
          } while (delem);
        }
      }
      felem = felem->NextSiblingElement(NULL);
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
        // remove the namespace qualifier to handle local name as key
        vars.push_back(ElementPtr(new Element(XMLNS::LocalName(elem->Name()), elem->GetText())));
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
