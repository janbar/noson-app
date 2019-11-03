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

#include "alarm.h"
#include "private/builtin.h"
#include "sonostypes.h"
#include "didlparser.h"
#include "private/cppdef.h"
#include "private/tokenizer.h"

using namespace NSROOT;

const char* NSROOT::RecurrenceTable[Recurrence_unknown + 1] = {
  "DAILY",
  "WEEKDAYS",
  "WEEKENDS",
  "ON_",
  "",
};

Alarm::Alarm()
: m_enabled(false)
, m_programURI(ALARM_BUZZER_URI)
, m_programMetadata(0)
, m_playMode(PlayModeTable[PlayMode_SHUFFLE])
, m_volume(20)
, m_includeLinkedZones(false)
{
}

Alarm::Alarm(Element& elem)
: m_enabled(false)
, m_programURI(ALARM_BUZZER_URI)
, m_programMetadata(0)
, m_playMode(PlayModeTable[PlayMode_SHUFFLE])
, m_volume(20)
, m_includeLinkedZones(false)
{
  parse(elem);
}

ElementList Alarm::MakeArguments()
{
  ElementList args;

  args.push_back(ElementPtr(new Element("ID", m_id)));
  args.push_back(ElementPtr(new Element("StartLocalTime", m_startLocalTime)));
  args.push_back(ElementPtr(new Element("Duration", m_duration)));
  args.push_back(ElementPtr(new Element("Recurrence", m_recurrence)));
  args.push_back(ElementPtr(new Element("Enabled", m_enabled ? "1" : "0")));
  args.push_back(ElementPtr(new Element("RoomUUID", m_roomUUID)));
  args.push_back(ElementPtr(new Element("ProgramURI", m_programURI)));
  args.push_back(ElementPtr(new Element("ProgramMetaData", m_programMetadata ? m_programMetadata->DIDL() : "")));
  args.push_back(ElementPtr(new Element("PlayMode", m_playMode)));
  args.push_back(ElementPtr(new Element("Volume", std::to_string((uint16_t)m_volume))));
  args.push_back(ElementPtr(new Element("IncludeLinkedZones", m_includeLinkedZones ? "1" : "0")));

  return args;
}

const char* NSROOT::DayTable[7] = { "SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT" };

void Alarm::setDays(char mask)
{
  std::string daystr("");
  switch (mask)
  {
  case 0b0111110:
    daystr.append(DayTable[1]).push_back(',');
    daystr.append(DayTable[2]).push_back(',');
    daystr.append(DayTable[3]).push_back(',');
    daystr.append(DayTable[4]).push_back(',');
    daystr.append(DayTable[5]);
    break;
  case 0b1000001:
    daystr.append(DayTable[0]).push_back(',');
    daystr.append(DayTable[6]);
    break;
  case 0b1111111:
    daystr.append(DayTable[0]).push_back(',');
    daystr.append(DayTable[1]).push_back(',');
    daystr.append(DayTable[2]).push_back(',');
    daystr.append(DayTable[3]).push_back(',');
    daystr.append(DayTable[4]).push_back(',');
    daystr.append(DayTable[5]).push_back(',');
    daystr.append(DayTable[6]);
    break;
  default:
    for (unsigned i = 0; i < 7; ++i)
    {
      if ((mask & (1 << i)))
      {
        if (!daystr.empty())
          daystr.push_back(',');
        daystr.append(DayTable[i]);
      }
    }
  }
  m_days.assign(daystr);
}

void Alarm::SetRecurrence(const std::string& days)
{
  char mask = 0;
  std::vector<std::string> tokens;
  tokenize(days, ",", tokens, true);
  for (std::vector<std::string>::const_iterator it = tokens.begin(); it != tokens.end(); ++it)
  {
    if (*it == DayTable[Day_SUNDAY])
      mask |= 0b0000001;
    else if (*it == DayTable[Day_MONDAY])
      mask |= 0b0000010;
    else if (*it == DayTable[Day_TUESDAY])
      mask |= 0b0000100;
    else if (*it == DayTable[Day_WEDNESDAY])
      mask |= 0b0001000;
    else if (*it == DayTable[Day_THURSDAY])
      mask |= 0b0010000;
    else if (*it == DayTable[Day_FRIDAY])
      mask |= 0b0100000;
    else if (*it == DayTable[Day_SATURDAY])
      mask |= 0b1000000;
  }
  std::string val;
  switch (mask)
  {
  case 0b1111111:
    val.assign(RecurrenceTable[Recurrence_DAILY]);
    break;
  case 0b1000001:
    val.assign(RecurrenceTable[Recurrence_WEEKENDS]);
    break;
  case 0b0111110:
    val.assign(RecurrenceTable[Recurrence_WEEKDAYS]);
    break;
  default:
    val.assign(RecurrenceTable[Recurrence_ON]);
    for (unsigned i = 0; i < 7; ++i)
    {
      if ((mask & (1 << i)))
        val.push_back((char)(i + 0x30));
    }
  }
  setDays(mask);
  m_recurrence.assign(val);
}

void Alarm::parse(Element& elem)
{
  std::vector<Element>& attr = elem.Attributs();
  for (std::vector<Element>::iterator it = attr.begin(); it != attr.end(); ++it)
  {
    if (it->GetKey() == "ID")
      m_id.assign(*it);
    else if (it->GetKey() == "StartTime" || it->GetKey() == "StartLocalTime")
      m_startLocalTime.assign(*it);
    else if (it->GetKey() == "Duration")
      m_duration.assign(*it);
    else if (it->GetKey() == "Recurrence")
    {
      char mask = 0;
      unsigned lon = strlen(RecurrenceTable[Recurrence_ON]);
      if (it->length() > lon && it->substr(0, lon) == RecurrenceTable[Recurrence_ON])
      {
        std::string days = it->substr(lon, std::string::npos);
        for (unsigned i = 0; i < days.length(); ++i)
        {
          char dno = days[i] - 0x30;
          if (dno >= 0 && dno <= 6)
            mask |= 1 << dno;
        }
      }
      else if (*it == RecurrenceTable[Recurrence_WEEKDAYS])
        mask = 0b0111110;
      else if (*it == RecurrenceTable[Recurrence_WEEKENDS])
        mask = 0b1000001;
      else if (*it == RecurrenceTable[Recurrence_DAILY])
        mask = 0b1111111;
      setDays(mask);
      m_recurrence.assign(*it);
    }
    else if (it->GetKey() == "Enabled")
      m_enabled = (*it == "0" ? false : true);
    else if (it->GetKey() == "RoomUUID")
      m_roomUUID.assign(*it);
    else if (it->GetKey() == "ProgramURI")
      m_programURI.assign(*it);
    else if (it->GetKey() == "ProgramMetaData")
    {
      DIDLParser didl(it->c_str());
      if (didl.IsValid() && didl.GetItems().size() > 0)
        m_programMetadata = didl.GetItems()[0];
      else
        m_programMetadata.reset();
    }
    else if (it->GetKey() == "PlayMode")
      m_playMode.assign(*it);
    else if (it->GetKey() == "Volume")
    {
      uint16_t value;
      string_to_uint16(it->c_str(), &value);
      m_volume = (int)value;
    }
    else if (it->GetKey() == "IncludeLinkedZones")
      m_includeLinkedZones = (*it == "0" ? false : true);
  }
}
