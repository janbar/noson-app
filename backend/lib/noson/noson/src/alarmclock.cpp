/*
 *      Copyright (C) 2018 Jean-Luc Barriere
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

#include "alarmclock.h"
#include "private/builtin.h"
#include "private/cppdef.h"
#include "private/debug.h"
#include "private/tinyxml2.h"
#include "private/xmldict.h"

using namespace NSROOT;

const std::string AlarmClock::Name("AlarmClock");
const std::string AlarmClock::ControlURL("/AlarmClock/Control");
const std::string AlarmClock::EventURL("/AlarmClock/Event");
const std::string AlarmClock::SCPDURL("/xml/AlarmClock1.xml");

AlarmClock::AlarmClock(const std::string& serviceHost, unsigned servicePort)
: Service(serviceHost, servicePort)
, m_subscriptionPool()
, m_subscription()
, m_CBHandle(nullptr)
, m_eventCB(nullptr)
, m_msgCount(0)
, m_property(ACProperty())
{
}

AlarmClock::AlarmClock(const std::string& serviceHost, unsigned servicePort, SubscriptionPoolPtr& subscriptionPool, void* CBHandle, EventCB eventCB)
: Service(serviceHost, servicePort)
, m_subscriptionPool(subscriptionPool)
, m_subscription()
, m_CBHandle(CBHandle)
, m_eventCB(eventCB)
, m_msgCount(0)
, m_property(ACProperty())
{
  unsigned subId = m_subscriptionPool->GetEventHandler().CreateSubscription(this);
  m_subscriptionPool->GetEventHandler().SubscribeForEvent(subId, EVENT_UPNP_PROPCHANGE);
  m_subscription = m_subscriptionPool->SubscribeEvent(serviceHost, servicePort, EventURL);
  m_subscription.Start();
}

AlarmClock::~AlarmClock()
{
  if (m_subscriptionPool)
  {
    m_subscriptionPool->UnsubscribeEvent(m_subscription);
    m_subscriptionPool->GetEventHandler().RevokeAllSubscriptions(this);
  }
}

bool AlarmClock::CreateAlarm(Alarm& alarm)
{
  ElementList args = alarm.MakeArguments();
  ElementList::iterator it = args.FindKey("ID");
  if (it != args.end())
    args.erase(it);
  ElementList vars = Request("CreateAlarm", args);
  if (!vars.empty() && vars[0]->compare("CreateAlarmResponse") == 0)
  {
    ElementList::const_iterator it = vars.FindKey("AssignedID");
    if (it != vars.end())
    {
      alarm.SetId(**it);
      return true;
    }
  }
  return false;
}

bool AlarmClock::UpdateAlarm(Alarm& alarm)
{
  ElementList args = alarm.MakeArguments();
  ElementList vars = Request("UpdateAlarm", args);
  if (!vars.empty() && vars[0]->compare("UpdateAlarmResponse") == 0)
    return true;
  return false;
}

bool AlarmClock::DestroyAlarm(const std::string& id)
{
  ElementList args;
  args.push_back(ElementPtr(new Element("ID", id)));
  ElementList vars = Request("DestroyAlarm", args);
  if (!vars.empty() && vars[0]->compare("DestroyAlarmResponse") == 0)
    return true;
  return false;
}

bool AlarmClock::ListAlarms(AlarmList& alarms)
{
  ElementList args;
  ElementList vars;
  vars = Request("ListAlarms", args);
  if (!vars.empty() && vars[0]->compare("ListAlarmsResponse") == 0)
  {
    for (ElementList::const_iterator it = vars.begin(); it != vars.end(); ++it)
    {
      if ((*it)->GetKey() == "CurrentAlarmList")
        ParseAlarmList(**it, alarms);
    }
    return true;
  }
  return false;
}

void AlarmClock::HandleEventMessage(EventMessagePtr msg)
{
  if (!msg)
    return;
  if (msg->event == EVENT_UPNP_PROPCHANGE)
  {
    if (m_subscription.GetSID() == msg->subject[0] && msg->subject[2] == "PROPERTY")
    {
      {
        // BEGIN CRITICAL SECTION
        Locked<ACProperty>::pointer prop = m_property.Get();

        DBG(DBG_DEBUG, "%s: %s SEQ=%s %s\n", __FUNCTION__, msg->subject[0].c_str(), msg->subject[1].c_str(), msg->subject[2].c_str());
        std::vector<std::string>::const_iterator it = msg->subject.begin();
        while (it != msg->subject.end())
        {
          uint32_t num;
          if (*it == "AlarmListVersion")
            prop->alarmListVersion.assign(*++it);
          else if (*it == "DailyIndexRefreshTime")
            prop->dailyIndexRefreshTime.assign(*++it);
          else if (*it == "DateFormat")
            prop->dateFormat.assign(*++it);
          else if (*it == "TimeFormat")
            prop->timeFormat.assign(*++it);
          else if (*it == "TimeGeneration")
          {
            string_to_uint32((*++it).c_str(), &num);
            prop->timeGeneration = (unsigned)num;
          }
          else if (*it == "TimeServer")
            prop->timeServer.assign(*++it);
          else if (*it == "TimeZone")
            prop->timeZone.assign(*++it);

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

bool AlarmClock::ParseAlarmList(const std::string& xml, AlarmList& alarms)
{
  tinyxml2::XMLDocument rootdoc;
  // Parse xml content
  if (rootdoc.Parse(xml.c_str(), xml.size()) != tinyxml2::XML_SUCCESS)
  {
    DBG(DBG_ERROR, "%s: parse xml failed\n", __FUNCTION__);
    return false;
  }
  const tinyxml2::XMLElement* elem; // an element
  // Check for response: Alarms
  if (!(elem = rootdoc.RootElement()) || !XMLNS::NameEqual(elem->Name(), "Alarms"))
  {
    DBG(DBG_ERROR, "%s: invalid or not supported content\n", __FUNCTION__);
    tinyxml2::XMLPrinter out;
    rootdoc.Accept(&out);
    DBG(DBG_ERROR, "%s\n", out.CStr());
    return false;
  }
  elem = elem->FirstChildElement();
  while (elem)
  {
    const tinyxml2::XMLAttribute* attr = elem->FirstAttribute();
    Element var(XMLNS::LocalName(elem->Name()));
    while (attr)
    {
      var.SetAttribut(attr->Name(), attr->Value());
      attr = attr->Next();
    }
    Alarm* alarm = new Alarm(var);
    alarms.push_back(AlarmPtr(alarm));
    elem = elem->NextSiblingElement(NULL);
  }
  return true;
}
