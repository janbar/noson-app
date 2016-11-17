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

#include "renderingcontrol.h"
#include "private/builtin.h"
#include "private/cppdef.h"
#include "private/debug.h"

using namespace NSROOT;

const std::string RenderingControl::Name("RenderingControl");
const std::string RenderingControl::ControlURL("/MediaRenderer/RenderingControl/Control");
const std::string RenderingControl::EventURL("/MediaRenderer/RenderingControl/Event");
const std::string RenderingControl::SCPDURL("/xml/RenderingControl1.xml");

const char* RenderingControl::CH_MASTER = "Master";

RenderingControl::RenderingControl(const std::string& serviceHost, unsigned servicePort)
: Service(serviceHost, servicePort)
, m_eventHandler()
, m_subscription()
, m_CBHandle(0)
, m_eventCB(0)
, m_msgCount(0)
, m_property(RCSProperty())
{
}

RenderingControl::RenderingControl(const std::string& serviceHost, unsigned servicePort, EventHandler& eventHandler, Subscription& subscription, void* CBHandle, EventCB eventCB)
: Service(serviceHost, servicePort)
, m_eventHandler(eventHandler)
, m_subscription(subscription)
, m_CBHandle(CBHandle)
, m_eventCB(eventCB)
, m_msgCount(0)
, m_property(RCSProperty())
{
  unsigned subId = m_eventHandler.CreateSubscription(this);
  m_eventHandler.SubscribeForEvent(subId, EVENT_UPNP_PROPCHANGE);
}

RenderingControl::~RenderingControl()
{
  m_eventHandler.RevokeAllSubscriptions(this);
}

bool RenderingControl::GetVolume(uint8_t* value, const char* channel)
{
  ElementList args;
  args.push_back(ElementPtr(new Element("InstanceID", "0")));
  args.push_back(ElementPtr(new Element("Channel", channel)));
  ElementList vars = Request("GetVolume", args);
  if (!vars.empty() && vars[0]->compare("u:GetVolumeResponse") == 0)
  {
    ElementList::const_iterator it = vars.FindKey("CurrentVolume");
    if (it != vars.end())
      return (string_to_uint8((*it)->c_str(), value) == 0);
  }
  return false;
}

bool RenderingControl::SetVolume(uint8_t value, const char* channel)
{
  char buf[4];
  memset(buf, 0, sizeof (buf));
  uint8_to_string(value, buf);
  ElementList args;
  args.push_back(ElementPtr(new Element("InstanceID", "0")));
  args.push_back(ElementPtr(new Element("Channel", channel)));
  args.push_back(ElementPtr(new Element("DesiredVolume", buf)));
  ElementList vars = Request("SetVolume", args);
  if (!vars.empty() && vars[0]->compare("u:SetVolumeResponse") == 0)
    return true;
  return false;
}

bool RenderingControl::GetMute(uint8_t* value, const char* channel)
{
  ElementList args;
  args.push_back(ElementPtr(new Element("InstanceID", "0")));
  args.push_back(ElementPtr(new Element("Channel", channel)));
  ElementList vars = Request("GetMute", args);
  if (!vars.empty() && vars[0]->compare("u:GetMuteResponse") == 0)
  {
    ElementList::const_iterator it = vars.FindKey("CurrentMute");
    if (it != vars.end())
      return (string_to_uint8((*it)->c_str(), value) == 0);
  }
  return false;
}

bool RenderingControl::SetMute(uint8_t value, const char* channel)
{
  char buf[4];
  memset(buf, 0, sizeof (buf));
  uint8_to_string(value, buf);
  ElementList args;
  args.push_back(ElementPtr(new Element("InstanceID", "0")));
  args.push_back(ElementPtr(new Element("Channel", channel)));
  args.push_back(ElementPtr(new Element("DesiredMute", buf)));
  ElementList vars = Request("SetMute", args);
  if (!vars.empty() && vars[0]->compare("u:SetMuteResponse") == 0)
    return true;
  return false;
}

void RenderingControl::HandleEventMessage(EventMessagePtr msg)
{
  if (!msg)
    return;
  if (msg->event == EVENT_UPNP_PROPCHANGE)
  {
    if (m_subscription.GetSID() == msg->subject[0] && msg->subject[2] == "RCS")
    {
      Locked<RCSProperty>::pointer prop = m_property.Get();

      DBG(DBG_DEBUG, "%s: %s SEQ=%s %s\n", __FUNCTION__, msg->subject[0].c_str(), msg->subject[1].c_str(), msg->subject[2].c_str());
      std::vector<std::string>::const_iterator it = msg->subject.begin();
      while (it != msg->subject.end())
      {
        int32_t num;
        if (*it == "Volume/Master")
        {
          if (string_to_int32((*++it).c_str(), &num) == 0)
            prop->VolumeMaster = num;
        }
        else if (*it == "Volume/LF")
        {
          if (string_to_int32((*++it).c_str(), &num) == 0)
            prop->VolumeLF = num;
        }
        else if (*it == "Volume/RF")
        {
          if (string_to_int32((*++it).c_str(), &num) == 0)
            prop->VolumeRF = num;
        }
        else if (*it == "Mute/Master")
        {
          if (string_to_int32((*++it).c_str(), &num) == 0)
            prop->MuteMaster = num;
        }
        else if (*it == "Mute/LF")
        {
          if (string_to_int32((*++it).c_str(), &num) == 0)
            prop->MuteLF = num;
        }
        else if (*it == "Mute/RF")
        {
          if (string_to_int32((*++it).c_str(), &num) == 0)
            prop->MuteRF = num;
        }

        ++it;
      }
      // Signal
      ++m_msgCount;
      if (m_eventCB)
        m_eventCB(m_CBHandle);
    }
  }
}
