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

#include "eventbroker.h"
#include "wsstatus.h"
#include "tinyxml2.h"
#include "xmldict.h"
#include "cppdef.h"
#include "debug.h"
#include "builtin.h"

#include <cstdio>

#define NS_RCS "urn:schemas-upnp-org:metadata-1-0/RCS/"
#define NS_AVT "urn:schemas-upnp-org:metadata-1-0/AVT/"
#define NS_RIN "urn:schemas-rinconnetworks-com:metadata-1-0/"
#define QN_RIN "r:"

using namespace NSROOT;

namespace NSROOT
{
  XMLDict __initRCSDict()
  {
    XMLDict dict;
    dict.DefineNS("", NS_RCS);
    dict.DefineNS(QN_RIN, NS_RIN);
    return dict;
  }
  XMLDict RCSDict = __initRCSDict();

  XMLDict __initAVTDict()
  {
    XMLDict dict;
    dict.DefineNS("", NS_AVT);
    dict.DefineNS(QN_RIN, NS_RIN);
    return dict;
  }
  XMLDict AVTDict = __initAVTDict();
}

EventBroker::EventBroker(EventHandler::EventHandlerThread* handler, SHARED_PTR<TcpSocket>& sockPtr)
: m_handler(handler)
, m_sockPtr(sockPtr)
, m_buffer(NULL)
{
}

EventBroker::~EventBroker()
{
  SAFE_DELETE_ARRAY(m_buffer);
}


void EventBroker::Process()
{
  if (!m_handler || !m_sockPtr || !m_sockPtr->IsValid())
    return;

  struct timeval socket_timeout = { 0, 500000 };
  WSRequestBroker rb(m_sockPtr.get(), socket_timeout);
  std::string resp;

  if (!rb.IsParsed())
  {
    WSStatus status(HSC_Bad_Request);
    resp.append(REQUEST_PROTOCOL " ").append(status.GetString()).append(" ").append(status.GetMessage());
    resp.append("\r\n\r\n");
    m_sockPtr->SendData(resp.c_str(), resp.size());
    m_sockPtr->Disconnect();
    return;
  }

  // Check for request NOTIFY
  if (rb.GetParsedMethod() == HRM_NOTIFY &&
          rb.GetParsedNamedEntry("NT") == "upnp:event" &&
          rb.GetParsedNamedEntry("CONTENT-TYPE").compare(0, 8, "text/xml") == 0 &&
          rb.HasContent())
  {
    // Receive content data
    size_t len = 0, l = 0;
    std::string data;
    char buffer[4096];
    while ((l = rb.ReadContent(buffer, sizeof(buffer))))
    {
      data.append(buffer, l);
      len += l;
    }

    // Parse xml content
    tinyxml2::XMLDocument rootdoc;
    if (rootdoc.Parse(data.c_str(), len) != tinyxml2::XML_SUCCESS)
    {
      DBG(DBG_ERROR, "%s: parse xml failed\n", __FUNCTION__);
      WSStatus status(HSC_Internal_Server_Error);
      resp.append(REQUEST_PROTOCOL " ").append(status.GetString()).append(" ").append(status.GetMessage());
      resp.append("\r\n\r\n");
      m_sockPtr->SendData(resp.c_str(), resp.size());
      m_sockPtr->Disconnect();
      return;
    }

    // Setup new event message
    EventMessage msg;
    msg.event = EVENT_UPNP_PROPCHANGE;
    msg.subject.push_back(rb.GetParsedNamedEntry("SID"));
    msg.subject.push_back(rb.GetParsedNamedEntry("SEQ"));

    // Parse document
    const tinyxml2::XMLElement* root; // root element
    const tinyxml2::XMLElement* elem; // an element
    const tinyxml2::XMLNode* node;    // a node
    tinyxml2::XMLDocument doc;  // a document
    const char* str;
    if ((root = rootdoc.RootElement()) && XMLNS::NameEqual(root->Name(), "propertyset"))
    {
      if ((node = root->FirstChild()) && XMLNS::NameEqual(node->Value(), "property"))
      {
        // Check prior for embedded doc 'Event': propertyset/property/LastChange
        if ((elem = node->FirstChildElement("LastChange")))
        {
          if (doc.Parse(elem->GetText()) != tinyxml2::XML_SUCCESS ||
                !(elem = doc.RootElement()))
          {
            DBG(DBG_ERROR, "%s: invalid or not supported content\n", __FUNCTION__);
            DBG(DBG_ERROR, "%s: dump => %s\n", __FUNCTION__, m_buffer);
            WSStatus status(HSC_Internal_Server_Error);
            resp.append(REQUEST_PROTOCOL " ").append(status.GetString()).append(" ").append(status.GetMessage());
            resp.append("\r\n\r\n");
            m_sockPtr->SendData(resp.c_str(), resp.size());
            m_sockPtr->Disconnect();
            return;
          }

          XMLNames docns;
          docns.AddXMLNS(elem);

          /*
           * Processing RCS notification
           */
          if (docns.FindName(NS_RCS) && (node = elem->FirstChildElement("InstanceID")))
          {
            msg.subject.push_back("RCS");
            elem = node->FirstChildElement(NULL);
            while (elem)
            {
              std::string name(RCSDict.TranslateQName(docns, elem->Name()));
              if ((str = elem->Attribute("channel")))
                name.append("/").append(str);
              msg.subject.push_back(name);
              if ((str = elem->Attribute("val")))
                msg.subject.push_back(str);
              else
                msg.subject.push_back("");
              DBG(DBG_PROTO, "%s: %s = %s\n", __FUNCTION__, name.c_str(), str);
              elem = elem->NextSiblingElement(NULL);
            }
          }
          /*
           * Processing AVT notification
           */
          else if (docns.FindName(NS_AVT) && (node = elem->FirstChildElement("InstanceID")))
          {
            msg.subject.push_back("AVT");
            elem = node->FirstChildElement(NULL);
            while (elem)
            {
              std::string name(AVTDict.TranslateQName(docns, elem->Name()));
              msg.subject.push_back(name);
              if ((str = elem->Attribute("val")))
                msg.subject.push_back(str);
              else
                msg.subject.push_back("");
              DBG(DBG_PROTO, "%s: %s = %s\n", __FUNCTION__, name.c_str(), str);
              elem = elem->NextSiblingElement(NULL);
            }
          }
          else
          {
            DBG(DBG_WARN, "%s: not supported content\n", __FUNCTION__);
            DBG(DBG_WARN, "%s: dump => %s\n", __FUNCTION__, m_buffer);
          }
        }
        // Else treat propertyset/property/
        else
        {
          msg.subject.push_back("PROPERTY");
          do
          {
            if ((elem = node->FirstChildElement(NULL)))
            {
              std::string name(XMLNS::LocalName(elem->Name()));
              msg.subject.push_back(name);
              if ((str = elem->GetText()))
                msg.subject.push_back(str);
              else
                msg.subject.push_back("");
              DBG(DBG_PROTO, "%s: %s = %s\n", __FUNCTION__, name.c_str(), str);
            }
            node = node->NextSibling();
          } while (node && XMLNS::NameEqual(node->Value(), "property"));
        }
      }
    }
    else
    {
      DBG(DBG_ERROR, "%s: invalid or not supported content\n", __FUNCTION__);
      DBG(DBG_ERROR, "%s: dump => %s\n", __FUNCTION__, m_buffer);
      WSStatus status(HSC_Internal_Server_Error);
      resp.append(REQUEST_PROTOCOL " ").append(status.GetString()).append(" ").append(status.GetMessage());
      resp.append("\r\n\r\n");
      m_sockPtr->SendData(resp.c_str(), resp.size());
      m_sockPtr->Disconnect();
      return;
    }

    m_handler->DispatchEvent(msg);
    WSStatus status(HSC_OK);
    resp.append(REQUEST_PROTOCOL " ").append(status.GetString()).append(" ").append(status.GetMessage());
    resp.append("\r\n\r\n");
    m_sockPtr->SendData(resp.c_str(), resp.size());
    m_sockPtr->Disconnect();
    return;
  }

  // Check for others request
  WSStatus status(HSC_Internal_Server_Error);
  switch (rb.GetParsedMethod())
  {
    case HRM_HEAD:
    case HRM_GET:
    {
      if (rb.GetParsedURI().compare("/") == 0)
      {
        static const char* version_string = "<html><h1>Noson Event Broker</h1><p>Version <b>" LIBVERSION "</b>, compiled on " __DATE__ " at " __TIME__ "</html>";
        char buf[8];
        uint32_to_string(strlen(version_string), buf);
        status.Set(HSC_OK);
        resp.append(REQUEST_PROTOCOL " ").append(status.GetString()).append(" ").append(status.GetMessage()).append("\r\n");
        resp.append("CONTENT-TYPE: text/html\r\n");
        resp.append("CONTENT-LENGTH: ").append(buf).append("\r\n");
        resp.append("\r\n");
        resp.append(version_string);
      }
      else
      {
        EventMessage msg;
        msg.event = EVENT_UNKNOWN;
        msg.subject.push_back("GET");
        msg.subject.push_back(rb.GetParsedURI());
        m_handler->DispatchEvent(msg);
        status.Set(HSC_OK);
        resp.append(REQUEST_PROTOCOL " ").append(status.GetString()).append(" ").append(status.GetMessage());
        resp.append("\r\n\r\n");
      }
      break;
    }
    default:
      status.Set(HSC_Internal_Server_Error);
      resp.append(REQUEST_PROTOCOL " ").append(status.GetString()).append(" ").append(status.GetMessage());
      resp.append("\r\n\r\n");
  }
  m_sockPtr->SendData(resp.c_str(), resp.size());
  m_sockPtr->Disconnect();
}
