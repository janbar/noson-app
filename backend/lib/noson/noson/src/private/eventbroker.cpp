/*
 *      Copyright (C) 2014-2019 Jean-Luc Barriere
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
#include "debug.h"
#include "requestbrokeropaque.h"

using namespace NSROOT;

EventBroker::EventBroker(EventHandlerThread* handler, SHARED_PTR<TcpSocket>& sockPtr)
: m_handler(handler)
, m_sockPtr(sockPtr)
{
}

EventBroker::~EventBroker()
{
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
    resp.append(REQUEST_PROTOCOL " ").append(status.GetString()).append(" ").append(status.GetMessage()).append("\r\n");
    resp.append("Server: ").append(REQUEST_USER_AGENT).append("\r\n");
    resp.append("Connection: close\r\n");
    resp.append("\r\n");
    m_sockPtr->SendData(resp.c_str(), resp.size());
    m_sockPtr->Disconnect();
    return;
  }

  RequestBroker::opaque payload = { m_sockPtr.get(), &rb };
  RequestBroker::handle handle { m_handler, &payload };
  std::vector<RequestBrokerPtr> vect = m_handler->AllRequestBroker();
  for (std::vector<RequestBrokerPtr>::iterator itrb = vect.begin(); itrb != vect.end(); ++itrb)
  {
    // loop until the request is processed
    if ((*itrb)->HandleRequest(&handle))
    {
      m_sockPtr->Disconnect();
      return;
    }
  }

  // default response for "HEAD /"
  if (rb.GetParsedMethod() == HRM_HEAD && rb.GetParsedURI().compare("/") == 0)
  {
    WSStatus status(HSC_OK);
    resp.append(REQUEST_PROTOCOL " ").append(status.GetString()).append(" ").append(status.GetMessage()).append("\r\n");
    resp.append("Server: ").append(REQUEST_USER_AGENT).append("\r\n");
    resp.append("Connection: close\r\n");
    resp.append("\r\n");
    m_sockPtr->SendData(resp.c_str(), resp.size());
    m_sockPtr->Disconnect();
    return;
  }

  // bad request!!!
  WSStatus status(HSC_Bad_Request);
  resp.append(REQUEST_PROTOCOL " ").append(status.GetString()).append(" ").append(status.GetMessage()).append("\r\n");
  resp.append("Server: ").append(REQUEST_USER_AGENT).append("\r\n");
  resp.append("Connection: close\r\n");
  resp.append("\r\n");
  m_sockPtr->SendData(resp.c_str(), resp.size());
  m_sockPtr->Disconnect();
}
