/*
 *      Copyright (C) 2019 Jean-Luc Barriere
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#include "upnpnotificationbroker.h"
#include "../eventhandler.h"
#include "debug.h"
#include "tinyxml2.h"
#include "xmldict.h"

using namespace NSROOT;

UPNPNotificationBroker::UPNPNotificationBroker()
: SONOS::RequestBroker()
{
}

bool UPNPNotificationBroker::HandleRequest(handle * handle)
{
  if (!IsAborted())
  {
    if (RequestBroker::GetRequestMethod(handle) == Method_NOTIFY &&
          RequestBroker::GetRequestHeader(handle, "NT") == "upnp:event" &&
          RequestBroker::GetRequestHeader(handle, "CONTENT-TYPE").compare(0, 8, "text/xml") == 0 &&
          RequestBroker::HasContent(handle))
    {
      Process(handle);
      return true;
    }
  }
  return false;
}

RequestBroker::ResourcePtr UPNPNotificationBroker::GetResource(const std::string& title)
{
  return ResourcePtr();
}

RequestBroker::ResourceList UPNPNotificationBroker::GetResourceList()
{
  ResourceList list;
  return list;
}

RequestBroker::ResourcePtr UPNPNotificationBroker::RegisterResource(const std::string& title,
                                                          const std::string& description,
                                                          const std::string& path,
                                                          StreamReader * delegate)
{
  (void)title;
  (void)description;
  (void)path;
  (void)delegate;
  return ResourcePtr();
}

void UPNPNotificationBroker::UnregisterResource(const std::string& uri)
{
  (void)uri;
}

void UPNPNotificationBroker::Reply500(handle * handle)
{
  std::string resp;
  resp.assign(RequestBroker::MakeResponseHeader(Status_Internal_Server_Error))
      .append("\r\n");
  RequestBroker::Reply(handle, resp.c_str(), resp.length());
}

#define NS_RCS "urn:schemas-upnp-org:metadata-1-0/RCS/"
#define NS_AVT "urn:schemas-upnp-org:metadata-1-0/AVT/"
#define NS_RIN "urn:schemas-rinconnetworks-com:metadata-1-0/"
#define QN_RIN "r:"

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

void UPNPNotificationBroker::Process(handle * handle)
{
  // Receive content data
  std::string data;
  size_t len = RequestBroker::ReadContent(handle, data);
  // Parse xml content
  tinyxml2::XMLDocument rootdoc;
  if (rootdoc.Parse(data.c_str(), len) != tinyxml2::XML_SUCCESS)
  {
    DBG(DBG_ERROR, "%s: parse xml failed\n", __FUNCTION__);
    Reply500(handle);
    return;
  }

  // Setup new event message
  EventMessage msg;
  msg.event = EVENT_UPNP_PROPCHANGE;
  msg.subject.push_back(RequestBroker::GetRequestHeader(handle, "SID"));
  msg.subject.push_back(RequestBroker::GetRequestHeader(handle, "SEQ"));

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
          DBG(DBG_ERROR, "%s: dump => %s\n", __FUNCTION__, data.c_str());
          Reply500(handle);
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
          DBG(DBG_WARN, "%s: dump => %s\n", __FUNCTION__, data.c_str());
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
    DBG(DBG_ERROR, "%s: dump => %s\n", __FUNCTION__, data.c_str());
    Reply500(handle);
    return;
  }

  handle->handler->DispatchEvent(msg);
  std::string resp;
  resp.assign(RequestBroker::MakeResponseHeader(Status_OK))
      .append("\r\n");
  RequestBroker::Reply(handle, resp.c_str(), resp.length());
  return;
}
