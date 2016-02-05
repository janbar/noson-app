/*
 *      Copyright (C) 2015 Jean-Luc Barriere
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

#ifndef EVENTHANDLER_H
#define	EVENTHANDLER_H

#include <local_config.h>
#include "sharedptr.h"

#include <string>
#include <vector>

#define EVENTHANDLER_STARTED        "STARTED"
#define EVENTHANDLER_STOPPED        "STOPPED"
#define EVENTHANDLER_FAILED         "FAILED"
#define EVENTHANDLER_TIMEOUT        1 // 1 sec

namespace NSROOT
{
  typedef enum
  {
    EVENT_HANDLER_STATUS = 0,     // Internal event: Backend status change
    EVENT_HANDLER_TIMER,          // Internal event: Every cycle without event
    EVENT_UPNP_PROPCHANGE,        // upnp:propchange
    EVENT_UNKNOWN,
  } EVENT_t;

  struct EventMessage
  {
    EVENT_t                   event;
    std::vector<std::string>  subject;

    EventMessage()
    : event(EVENT_UNKNOWN)
    {}
  };

  typedef SHARED_PTR<EventMessage> EventMessagePtr;

  class EventSubscriber
  {
  public:
    virtual ~EventSubscriber() {};
    virtual void HandleEventMessage(EventMessagePtr msg) = 0;
  };

  class EventHandler
  {
  public:
    EventHandler();
    EventHandler(unsigned bindingPort);

    bool Start() { return m_imp ? m_imp->Start(): false; }
    void Stop() { if (m_imp) m_imp->Stop(); }
    std::string GetAddress() const { return m_imp ? m_imp->GetAddress() : ""; }
    unsigned GetPort() const { return m_imp ? m_imp->GetPort(): 0; }
    bool IsRunning() { return m_imp ? m_imp->IsRunning() : false; }

    unsigned CreateSubscription(EventSubscriber *sub) { return m_imp ? m_imp->CreateSubscription(sub) : 0; }
    bool SubscribeForEvent(unsigned subid, EVENT_t event) { return m_imp ? m_imp->SubscribeForEvent(subid, event) : false; }
    void RevokeSubscription(unsigned subid) { if (m_imp) m_imp->RevokeSubscription(subid); }
    void RevokeAllSubscriptions(EventSubscriber *sub) { if (m_imp) m_imp->RevokeAllSubscriptions(sub); }

    class EventHandlerThread
    {
      friend class EventHandler;
    public:
      EventHandlerThread(unsigned bindingPort);
      virtual ~EventHandlerThread();
      virtual std::string GetAddress() const { return m_listenerAddress; }
      virtual unsigned GetPort() const { return m_port; }
      virtual bool Start() = 0;
      virtual void Stop() = 0;
      virtual bool IsRunning() = 0;
      virtual unsigned CreateSubscription(EventSubscriber *sub) = 0;
      virtual bool SubscribeForEvent(unsigned subid, EVENT_t event) = 0;
      virtual void RevokeSubscription(unsigned subid) = 0;
      virtual void RevokeAllSubscriptions(EventSubscriber *sub) = 0;
      virtual void DispatchEvent(const EventMessage& msg) = 0;

    protected:
      std::string m_listenerAddress;
      unsigned m_port;
    };

    typedef SHARED_PTR<EventHandlerThread> EventHandlerThreadPtr;

  private:
    EventHandlerThreadPtr m_imp;
  };

}

#endif	/* EVENTHANDLER_H */
