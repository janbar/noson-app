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

#ifndef ALARM_H
#define ALARM_H

#include "local_config.h"
#include "sonostypes.h"
#include "element.h"
#include "sharedptr.h"
#include "digitalitem.h"

#define ALARM_BUZZER_URI "x-rincon-buzzer:0"

namespace NSROOT
{

  class Alarm;

  typedef SHARED_PTR<Alarm> AlarmPtr;
  typedef std::vector<AlarmPtr> AlarmList;

  typedef enum {
    Recurrence_DAILY      = 0,
    Recurrence_WEEKDAYS   = 1,
    Recurrence_WEEKENDS,
    Recurrence_ON,
    Recurrence_unknown,
  } Recurrence_t;

  extern const char* RecurrenceTable[Recurrence_unknown + 1];

  typedef enum {
    Day_SUNDAY    = 0,
    Day_MONDAY    = 1,
    Day_TUESDAY   = 2,
    Day_WEDNESDAY = 3,
    Day_THURSDAY  = 4,
    Day_FRIDAY    = 5,
    Day_SATURDAY  = 6,
  } Day_t;

  extern const char* DayTable[7];

  class Alarm {
  public:
    Alarm();

    Alarm(Element& elem);

    virtual ~Alarm() { }

    ElementList MakeArguments();

    const std::string& GetId() const { return m_id; }

    void SetId(const std::string& id) { m_id.assign(id); }

    bool GetEnabled() const { return m_enabled; }

    void SetEnabled(bool enabled) { m_enabled = enabled; }

    const std::string& GetProgramURI() const { return m_programURI; }

    void SetProgramURI(const std::string& uri) { m_programURI.assign(uri); }

    const DigitalItemPtr GetProgramMetadata() const { return m_programMetadata; }

    void SetProgramMetadata(const DigitalItemPtr& metadata) { m_programMetadata = metadata; }

    const std::string& GetPlayMode() const { return m_playMode; }

    void SetPlayMode(PlayMode_t mode) { m_playMode.assign(PlayModeTable[mode]); }

    int GetVolume() const { return m_volume; }

    void SetVolume(int volume) { m_volume = volume; }

    bool GetIncludeLinkedZones() const { return m_includeLinkedZones; }

    void SetIncludeLinkedZones(bool include) { m_includeLinkedZones = include; }

    const std::string& GetRoomUUID() const { return m_roomUUID; }

    void SetRoomUUID(const std::string& uuid) { m_roomUUID.assign(uuid); }

    const std::string& GetStartLocalTime() const { return m_startLocalTime; }

    void SetStartLocalTime(const std::string& start) { m_startLocalTime.assign(start); }

    const std::string& GetDuration() const { return m_duration; }

    void SetDuration(const std::string& duration) { m_duration.assign(duration); }

    const std::string& GetRecurrence() const { return m_days; }

    void SetRecurrence(const std::string& days);

  private:
    std::string m_id;
    bool m_enabled;
    std::string m_programURI;
    DigitalItemPtr m_programMetadata;
    std::string m_playMode;
    int m_volume;
    bool m_includeLinkedZones;
    std::string m_roomUUID;
    std::string m_startLocalTime;
    std::string m_duration;
    std::string m_recurrence;
    std::string m_days;

    void setDays(char mask);
    void parse(Element& elem);
  };
}

#endif /* ALARM_H */

