/*
 *      Copyright (C) 2014-2018 Jean-Luc Barriere
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

#include "subscription.h"
#include "private/cppdef.h"
#include "private/os/threads/thread.h"
#include "private/os/threads/event.h"
#include "private/wsresponse.h"
#include "private/uriparser.h"
#include "private/socket.h"
#include "private/builtin.h"
#include "private/debug.h"
#include "sonossystem.h" // for definitions

#define TIMEOUT_FIRST_RETRY  1000 // millisec
#define TIMEOUT_AGAIN_RETRY  5000 // millisec

using namespace NSROOT;

Subscription::SubscriptionThread::SubscriptionThread()
{
}

Subscription::SubscriptionThread::~SubscriptionThread()
{
}

namespace NSROOT
{

  class SubscriptionThreadImpl : public Subscription::SubscriptionThread, private OS::CThread
  {
  public:
    SubscriptionThreadImpl(const std::string& host, unsigned port, const std::string& url, unsigned bindingPort, unsigned ttl)
    : CThread()
    , m_SID()
    , m_host(host)
    , m_port(port)
    , m_url(url)
    , m_bindingPort(bindingPort)
    , m_ttl(SUBSCRIPTION_TIMEOUT_MAX)
    , m_configured(false)
    , m_renewable(false)
    {
      m_ttl = (ttl < SUBSCRIPTION_TIMEOUT_MIN ? SUBSCRIPTION_TIMEOUT_MIN :
              (ttl > SUBSCRIPTION_TIMEOUT_MAX ? SUBSCRIPTION_TIMEOUT_MAX : ttl));
      // Try to configure
      Configure();
    }

    virtual ~SubscriptionThreadImpl()
    {
      Stop();
    }

    virtual bool IsValid()
    {
      return m_configured;
    }

    virtual bool Start()
    {
      return OS::CThread::StartThread();
    }

    virtual void Stop()
    {
      OS::CThread::StopThread(false);
      m_event.Signal();
      OS::CThread::StopThread(true);
    }

    virtual bool IsRunning() { return OS::CThread::IsRunning(); }

    virtual void AskRenewal()
    {
      if (IsRunning())
      {
        m_event.Signal();
      }
    }

    virtual const std::string& GetSID() { return m_SID; }

    virtual const std::string& GetHost() { return m_host; }

    virtual unsigned GetPort() { return m_port; }

    virtual const std::string& GetUrl() { return m_url; }

    virtual unsigned GetBindingPort() { return m_bindingPort; }

  private:
    std::string m_SID;
    std::string m_host;
    unsigned m_port;
    std::string m_url;
    unsigned m_bindingPort;
    unsigned m_ttl;
    bool m_configured;
    bool m_renewable;
    std::string m_myIP;
    OS::CTimeout m_timeout;
    OS::CEvent m_event;

    virtual void* Process();
    bool Configure();
    bool SubscribeForEvent(bool renew = false);
    bool UnSubscribeForEvent();
  };
}

void* SubscriptionThreadImpl::Process()
{
  bool success = false;
  unsigned retry = TIMEOUT_FIRST_RETRY;
  while (!IsStopped())
  {
    // Reconfigure: IP may be leased for a time
    if (Configure() && (success = SubscribeForEvent(success)))
    {
      m_event.Wait(m_timeout.TimeLeft() * SUBSCRIPTION_RENEW_PCT / 100);
      retry = TIMEOUT_FIRST_RETRY;
    }
    else
    {
      // wait before retry
      m_event.Wait(retry);
      retry = TIMEOUT_AGAIN_RETRY;
    }
  }
  if (success)
    UnSubscribeForEvent();
  return NULL;
}

bool SubscriptionThreadImpl::Configure()
{
  TcpSocket sock;
  sock.Connect(m_host.c_str(), m_port, 0);
  std::string myIP = sock.GetHostAddrInfo();
  sock.Disconnect();
  if (!myIP.empty())
  {
    if (myIP == m_myIP)
      m_renewable = true; // IP leasing is still valid
    else
      m_renewable = false; // IP has changed, therefore don't try to renew
    m_myIP = myIP;
    return m_configured = true;
  }
  m_myIP.clear();
  return m_configured = false;
}

bool SubscriptionThreadImpl::SubscribeForEvent(bool renew)
{
  WSRequest request(m_host, m_port);
  request.RequestService(m_url, HRM_SUBSCRIBE);
  // is renewable ?
  if (renew && m_renewable && m_timeout.TimeLeft() > 0)
  {
    DBG(DBG_DEBUG, "%s: renew subscription (%s)\n", __FUNCTION__, m_SID.c_str());
    request.SetHeader("SID", m_SID);
  }
  else
  {
    std::string cb;
    cb.assign("<http://").append(m_myIP).append(":").append(std::to_string(m_bindingPort)).append(">");
    request.SetHeader("Callback", cb);
    request.SetHeader("NT", "upnp:event");
  }
  std::string tmo;
  tmo.assign("Second-").append(std::to_string((uint32_t)m_ttl));
  request.SetHeader("TIMEOUT", tmo);
  WSResponse response(request);
  if (response.IsSuccessful() && response.GetHeaderValue("SID", m_SID))
  {
    m_timeout.Set(m_ttl * 1000);
    return true;
  }
  m_SID.clear();
  m_timeout.Clear();
  return false;
}

bool SubscriptionThreadImpl::UnSubscribeForEvent()
{
  if (!m_SID.empty())
  {
    WSRequest request(m_host, m_port);
    request.RequestService(m_url, HRM_UNSUBSCRIBE);
    request.SetHeader("SID", m_SID);
    WSResponse response(request);
    if (!response.IsSuccessful())
      return false;
    m_SID.clear();
    m_timeout.Clear();
  }
  return true;
}

Subscription::Subscription()
: m_imp()
{
}

Subscription::Subscription(const std::string& host, unsigned port, const std::string& url, unsigned bindingPort, unsigned ttl)
: m_imp()
{
  m_imp = SubscriptionThreadPtr(new SubscriptionThreadImpl(host, port, url, bindingPort, ttl));
}

Subscription::~Subscription()
{
}

bool Subscription::IsValid()
{
  return m_imp ? m_imp->IsValid() : false;
}

bool Subscription::Start()
{
  if (m_imp)
    return m_imp->Start();
  return false;
}

void Subscription::Stop()
{
  if (m_imp)
     m_imp->Stop();
}

void Subscription::AskRenewal()
{
  if (m_imp)
    m_imp->AskRenewal();
}

const std::string& Subscription::GetSID()
{
  static std::string nil;
  return m_imp ? m_imp->GetSID() : nil;
}

const std::string& Subscription::GetHost()
{
  static std::string nil;
  return m_imp ? m_imp->GetHost() : nil;
}

unsigned Subscription::GetPort()
{
  return m_imp ? m_imp->GetPort() : 0;
}

const std::string& Subscription::GetUrl()
{
  static std::string nil;
  return m_imp ? m_imp->GetUrl() : nil;
}

unsigned Subscription::GetBindingPort()
{
  return m_imp ? m_imp->GetBindingPort() : 0;
}
