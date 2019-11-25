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
, m_subscriptionPool()
, m_subscription()
, m_CBHandle(nullptr)
, m_eventCB(nullptr)
, m_msgCount(0)
, m_property(RCSProperty())
{
}

RenderingControl::RenderingControl(const std::string& serviceHost, unsigned servicePort, SubscriptionPoolPtr& subscriptionPool, void* CBHandle, EventCB eventCB)
: Service(serviceHost, servicePort)
, m_subscriptionPool(subscriptionPool)
, m_subscription()
, m_CBHandle(CBHandle)
, m_eventCB(eventCB)
, m_msgCount(0)
, m_property(RCSProperty())
{
  unsigned subId = m_subscriptionPool->GetEventHandler().CreateSubscription(this);
  m_subscriptionPool->GetEventHandler().SubscribeForEvent(subId, EVENT_UPNP_PROPCHANGE);
  m_subscription = m_subscriptionPool->SubscribeEvent(serviceHost, servicePort, EventURL);
  m_subscription.Start();
}

RenderingControl::~RenderingControl()
{
  if (m_subscriptionPool)
  {
    m_subscriptionPool->UnsubscribeEvent(m_subscription);
    m_subscriptionPool->GetEventHandler().RevokeAllSubscriptions(this);
  }
}

bool RenderingControl::GetVolume(uint8_t* value, const char* channel)
{
  ElementList args;
  args.push_back(ElementPtr(new Element("InstanceID", "0")));
  args.push_back(ElementPtr(new Element("Channel", channel)));
  ElementList vars = Request("GetVolume", args);
  if (!vars.empty() && vars[0]->compare("GetVolumeResponse") == 0)
  {
    ElementList::const_iterator it = vars.FindKey("CurrentVolume");
    if (it != vars.end())
      return (string_to_uint8((*it)->c_str(), value) == 0);
  }
  return false;
}

bool RenderingControl::SetVolume(uint8_t value, const char* channel)
{
  if (m_property.Get()->OutputFixed)
    return false;
  ElementList args;
  args.push_back(ElementPtr(new Element("InstanceID", "0")));
  args.push_back(ElementPtr(new Element("Channel", channel)));
  args.push_back(ElementPtr(new Element("DesiredVolume", std::to_string(value))));
  ElementList vars = Request("SetVolume", args);
  if (!vars.empty() && vars[0]->compare("SetVolumeResponse") == 0)
    return true;
  return false;
}

bool RenderingControl::GetMute(uint8_t* value, const char* channel)
{
  ElementList args;
  args.push_back(ElementPtr(new Element("InstanceID", "0")));
  args.push_back(ElementPtr(new Element("Channel", channel)));
  ElementList vars = Request("GetMute", args);
  if (!vars.empty() && vars[0]->compare("GetMuteResponse") == 0)
  {
    ElementList::const_iterator it = vars.FindKey("CurrentMute");
    if (it != vars.end())
      return (string_to_uint8((*it)->c_str(), value) == 0);
  }
  return false;
}

bool RenderingControl::SetMute(uint8_t value, const char* channel)
{
  ElementList args;
  args.push_back(ElementPtr(new Element("InstanceID", "0")));
  args.push_back(ElementPtr(new Element("Channel", channel)));
  args.push_back(ElementPtr(new Element("DesiredMute", std::to_string(value))));
  ElementList vars = Request("SetMute", args);
  if (!vars.empty() && vars[0]->compare("SetMuteResponse") == 0)
    return true;
  return false;
}

bool RenderingControl::GetNightmode(uint8_t *value)
{
  ElementList args;
  args.push_back(ElementPtr(new Element("InstanceID", "0")));
  ElementList vars = Request("GetEQ", args);
  if (!vars.empty() && vars[0]->compare("GetEQResponse") == 0)
  {
    ElementList::const_iterator it = vars.FindKey("NightMode");
    if (it != vars.end())
      return (string_to_uint8((*it)->c_str(), value) == 0);
  }
  return false;
}

bool RenderingControl::SetNightmode(uint8_t value)
{
  ElementList args;
  args.push_back(ElementPtr(new Element("InstanceID", "0")));
  args.push_back(ElementPtr(new Element("EQType", "NightMode")));
  args.push_back(ElementPtr(new Element("DesiredValue", std::to_string(value))));
  ElementList vars = Request("SetEQ", args);
  if (!vars.empty() && vars[0]->compare("SetEQResponse") == 0)
    return true;
  return false;
}

bool RenderingControl::GetTreble(int8_t* value)
{
  ElementList args;
  args.push_back(ElementPtr(new Element("InstanceID", "0")));
  ElementList vars = Request("GetTreble", args);
  if (!vars.empty() && vars[0]->compare("GetTrebleResponse") == 0)
  {
    ElementList::const_iterator it = vars.FindKey("CurrentTreble");
    if (it != vars.end())
      return (string_to_int8((*it)->c_str(), value) == 0);
  }
  return false;
}

bool RenderingControl::SetTreble(int8_t value)
{
  ElementList args;
  args.push_back(ElementPtr(new Element("InstanceID", "0")));
  args.push_back(ElementPtr(new Element("DesiredTreble", std::to_string(value))));
  ElementList vars = Request("SetTreble", args);
  if (!vars.empty() && vars[0]->compare("SetTrebleResponse") == 0)
    return true;
  return false;
}

bool RenderingControl::GetBass(int8_t* value)
{
  ElementList args;
  args.push_back(ElementPtr(new Element("InstanceID", "0")));
  ElementList vars = Request("GetBass", args);
  if (!vars.empty() && vars[0]->compare("GetBassResponse") == 0)
  {
    ElementList::const_iterator it = vars.FindKey("CurrentBass");
    if (it != vars.end()) {
      return (string_to_int8((*it)->c_str(), value) == 0);
    }
  }
  return false;
}

bool RenderingControl::SetBass(int8_t value)
{
  ElementList args;
  args.push_back(ElementPtr(new Element("InstanceID", "0")));
  args.push_back(ElementPtr(new Element("DesiredBass", std::to_string(value))));
  ElementList vars = Request("SetBass", args);
  if (!vars.empty() && vars[0]->compare("SetBassResponse") == 0)
    return true;
  return false;
}

bool RenderingControl::GetSupportsOutputFixed(uint8_t *value)
{
  ElementList args;
  args.push_back(ElementPtr(new Element("InstanceID", "0")));
  ElementList vars = Request("GetSupportsOutputFixed", args);
  if (!vars.empty() && vars[0]->compare("GetSupportsOutputFixedResponse") == 0)
  {
    ElementList::const_iterator it = vars.FindKey("CurrentSupportsFixed");
    if (it != vars.end())
      return (string_to_uint8((*it)->c_str(), value) == 0);
  }
  return false;
}

bool RenderingControl::GetOutputFixed(uint8_t *value)
{
  ElementList args;
  args.push_back(ElementPtr(new Element("InstanceID", "0")));
  ElementList vars = Request("GetOutputFixed", args);
  if (!vars.empty() && vars[0]->compare("GetOutputFixedResponse") == 0)
  {
    ElementList::const_iterator it = vars.FindKey("CurrentFixed");
    if (it != vars.end())
      return (string_to_uint8((*it)->c_str(), value) == 0);
  }
  return false;
}

bool RenderingControl::SetOutputFixed(uint8_t value)
{
  ElementList args;
  args.push_back(ElementPtr(new Element("InstanceID", "0")));
  args.push_back(ElementPtr(new Element("DesiredFixed", std::to_string(value))));
  ElementList vars = Request("SetOutputFixed", args);
  if (!vars.empty() && vars[0]->compare("SetOutputFixedResponse") == 0)
    return true;
  return false;
}

bool RenderingControl::GetLoudness(uint8_t* value, const char* channel)
{
  ElementList args;
  args.push_back(ElementPtr(new Element("InstanceID", "0")));
  args.push_back(ElementPtr(new Element("Channel", channel)));
  ElementList vars = Request("GetLoudness", args);
  if (!vars.empty() && vars[0]->compare("GetLoudnessResponse") == 0)
  {
    ElementList::const_iterator it = vars.FindKey("CurrentLoudness");
    if (it != vars.end())
      return (string_to_uint8((*it)->c_str(), value) == 0);
  }
  return false;
}

bool RenderingControl::SetLoudness(uint8_t value, const char* channel)
{
  ElementList args;
  args.push_back(ElementPtr(new Element("InstanceID", "0")));
  args.push_back(ElementPtr(new Element("Channel", channel)));
  args.push_back(ElementPtr(new Element("DesiredLoudness", std::to_string(value))));
  ElementList vars = Request("SetLoudness", args);
  if (!vars.empty() && vars[0]->compare("SetLoudnessResponse") == 0)
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
      {
        // BEGIN CRITICAL SECTION
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
          else if (*it == "NightMode")
          {
            if (string_to_int32((*++it).c_str(), &num) == 0)
              prop->NightMode = num;
          }
          else if (*it == "Bass")
          {
            if (string_to_int32((*++it).c_str(), &num) == 0)
              prop->Bass = num;
          }
          else if (*it == "Treble")
          {
            if (string_to_int32((*++it).c_str(), &num) == 0)
              prop->Treble = num;
          }
          else if (*it == "OutputFixed")
          {
            if (string_to_int32((*++it).c_str(), &num) == 0)
              prop->OutputFixed = num;
          }
          else if (*it == "Loudness/Master")
          {
            if (string_to_int32((*++it).c_str(), &num) == 0)
              prop->LoudnessMaster = num;
          }

          ++it;
        }
        // END CRITICAL SECTION
      }
      // Signal
      ++m_msgCount;
      if (m_eventCB)
        m_eventCB(m_CBHandle);
    }
  }
}
