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
#include "mainpagebroker.h"
#include "../eventhandler.h"
#include "debug.h"
#include "tinyxml2.h"
#include "xmldict.h"

#include <vector>

using namespace NSROOT;

MainPageBroker::MainPageBroker()
: SONOS::RequestBroker()
{
}

bool MainPageBroker::HandleRequest(handle * handle)
{
  if (!IsAborted())
  {
    if (RequestBroker::GetRequestMethod(handle) == Method_GET &&
            RequestBroker::GetRequestURI(handle).compare("/") == 0)
    {
      Process(handle);
      return true;
    }
  }
  return false;
}

RequestBroker::ResourcePtr MainPageBroker::GetResource(const std::string& title)
{
  return ResourcePtr();
}

RequestBroker::ResourceList MainPageBroker::GetResourceList()
{
  ResourceList list;
  return list;
}

RequestBroker::ResourcePtr MainPageBroker::RegisterResource(const std::string& title,
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

void MainPageBroker::UnregisterResource(const std::string& uri)
{
  (void)uri;
}

void MainPageBroker::Process(handle * handle)
{
  static const char * _begin = "<!DOCTYPE html>"
  "<html lang=\"en\"><head><meta charset=\"utf-8\"><title>Noson Event Broker</title>"
  "<style>#document{font-family:Tahoma,Geneva,sans-serif;font-size:16px;letter-spaci"
  "ng:2px;word-spacing:2px;color:#000;font-weight:400;text-decoration:none;font-styl"
  "e:normal;font-variant:normal;text-transform:none}table.paleGreyRows{border:1px so"
  "lid #fff;width:800px;height:200px;text-align:left;border-collapse:collapse}table."
  "paleGreyRows td,table.paleGreyRows th{border:1px solid #fff;padding:3px 2px}table"
  ".paleGreyRows tbody td{font-size:14px}table.paleGreyRows tr:nth-child(even){backg"
  "round:#e5e5e5}table.paleGreyRows thead{background:#fff;border-bottom:3px solid #0"
  "00}table.paleGreyRows thead th{font-size:16px;font-weight:700;color:#000;text-ali"
  "gn:left;border-left:2px solid #fff}table.paleGreyRows thead th:first-child{border"
  "-left:none}</style></head><body><div id=\"document\"><h1>Noson Event Broker</h1><"
  "p>Version <b>" LIBVERSION "</b>, compiled on " __DATE__ " at " __TIME__ ".</p>";
  static const char * _end = "</div></body></html>";

  std::string resp;
  resp.assign(RequestBroker::MakeResponseHeader(Status_OK));
  std::string data;
  data.assign(_begin);
  std::vector<RequestBrokerPtr> rbs = handle->handler->AllRequestBroker();
  if (rbs.size() > 0)
  {
    data.append(
      "<table class=\"paleGreyRows\"><thead><tr>"
      "<th>Module</th><th style=\"text-align: center;\">Enabled</th>"
      "<th style=\"text-align: center;\">200</th>"
      "<th style=\"text-align: center;\">400</th>"
      "<th style=\"text-align: center;\">404</th>"
      "<th style=\"text-align: center;\">429</th>"
      "<th style=\"text-align: center;\">500</th>"
      "<th style=\"text-align: center;\">503</th>"
      "</tr></thead><tbody>"
    );
    for (std::vector<RequestBrokerPtr>::iterator itrb = rbs.begin(); itrb != rbs.end(); ++itrb)
    {
      unsigned int replies[6];
      replies[0] = (*itrb)->Count200();
      replies[1] = (*itrb)->Count400();
      replies[2] = (*itrb)->Count404();
      replies[3] = (*itrb)->Count429();
      replies[4] = (*itrb)->Count500();
      replies[5] = (*itrb)->Count503();
      data.append("<tr><td>").append((*itrb)->CommonName())
          .append("</td><td style=\"text-align: center;\">")
          .append(((*itrb)->IsAborted() ? "<b>No</b>" : "Yes"))
          .append("</td>");
      for (int i = 0; i < 6; ++i)
      {
        data.append("<td style=\"text-align: center;\">")
            .append(replies[i] ? std::to_string(replies[i]) : "&middot;")
            .append("</td>");
      }
      data.append("</tr>");
    }
    data.append("</tbody></table>");
  }
  data.append(_end);
  resp.append("Content-type: text/html\r\n")
      .append("Content-length: ").append(std::to_string(data.length())).append("\r\n")
      .append("\r\n");
  RequestBroker::Reply(handle, resp.c_str(), resp.length());
  RequestBroker::Reply(handle, data.c_str(), data.length());
  return;
}
