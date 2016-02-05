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

#ifndef RENDERINGCONTROL_H
#define	RENDERINGCONTROL_H

#include <local_config.h>
#include "service.h"
#include "eventhandler.h"
#include "subscription.h"
#include "locked.h"

#include <stdint.h>

namespace NSROOT
{
  class Subscription;

  class RenderingControl : public Service, public EventSubscriber
  {
  public:
    RenderingControl(const std::string& serviceHost, unsigned servicePort);
    RenderingControl(const std::string& serviceHost, unsigned servicePort, EventHandler& eventHandler, Subscription& subscription, void* CBHandle = 0, EventCB eventCB = 0);
    ~RenderingControl();

    static const std::string Name;
    static const std::string ControlURL;
    static const std::string EventURL;
    static const std::string SCPDURL;

    static const char* CH_MASTER;

    const std::string& GetName() const { return Name; }

    const std::string& GetControlURL() const { return ControlURL; }

    const std::string& GetEventURL() const { return EventURL; }

    const std::string& GetSCPDURL() const { return SCPDURL; }

    bool GetVolume(uint8_t* value, const char* channel = CH_MASTER);

    bool SetVolume(uint8_t value, const char* channel = CH_MASTER);

    bool GetMute(uint8_t* value, const char* channel = CH_MASTER);

    bool SetMute(uint8_t value, const char* channel = CH_MASTER);

    // Implements EventSubscriber
    virtual void HandleEventMessage(EventMessagePtr msg);

    bool Empty() { return m_msgCount == 0; }

    Locked<RCSProperty>& GetRenderingProperty() { return m_property; }

  private:
    EventHandler m_eventHandler;
    Subscription m_subscription;
    void* m_CBHandle;
    EventCB m_eventCB;
    unsigned m_msgCount;

    Locked<RCSProperty> m_property;
  };
}

#endif	/* RENDERINGCONTROL_H */

