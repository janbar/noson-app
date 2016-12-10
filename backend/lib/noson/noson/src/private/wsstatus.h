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

#ifndef WSSTATUS_H
#define	WSSTATUS_H

#include <local_config.h>

#include <string>
#include <cassert>

namespace NSROOT
{

  typedef enum
  {
    HSC_Unknown   = 0,
    HSC_OK        = 1,
    HSC_Created,
    HSC_Accepted,
    HSC_No_Content,
    HSC_Moved_Permanently,
    HSC_Moved_Temporarily,
    HSC_Not_Modified,
    HSC_Bad_Request,
    HSC_Unauthorized,
    HSC_Forbidden,
    HSC_Not_Found,
    HSC_Internal_Server_Error,
    HSC_Not_Implemented,
    HSC_Bad_Gateway,
    HSC_Service_Unavailable,
  } HSC_t;

  typedef struct { HSC_t value; int statusInt; const char* statusStr; const char* msgStr; } HSC_rec;
  static const HSC_rec HSC_table[] = {
    { HSC_Unknown               , 0   , "000" , "?" },
    { HSC_OK                    , 200 , "200" , "OK" },
    { HSC_Created               , 201 , "201" , "Created" },
    { HSC_Accepted              , 202 , "202" , "Accepted" },
    { HSC_No_Content            , 204 , "204" , "No content" },
    { HSC_Moved_Permanently     , 301 , "301" , "Moved permanently" },
    { HSC_Moved_Temporarily     , 302 , "302" , "Moved temporarily" },
    { HSC_Not_Modified          , 304 , "304" , "Not modified" },
    { HSC_Bad_Request           , 400 , "400" , "Bad request" },
    { HSC_Unauthorized          , 401 , "401" , "Unauthorized" },
    { HSC_Forbidden             , 403 , "403" , "Forbidden" },
    { HSC_Not_Found             , 404 , "404" , "Not found" },
    { HSC_Internal_Server_Error , 500 , "500" , "Internal server error" },
    { HSC_Not_Implemented       , 501 , "501" , "Not implemented" },
    { HSC_Bad_Gateway           , 502 , "502" , "Bad gateway" },
    { HSC_Service_Unavailable   , 503 , "503" , "Service unavailable" },
  };

  class WSStatus
  {
  public:
    WSStatus() : m_value(HSC_Unknown) { }
    WSStatus(HSC_t value) : m_value(value) { }
    virtual ~WSStatus() { }

    operator bool() const
    {
      return m_value != HSC_Unknown;
    }

    bool operator!() const
    {
      return m_value == HSC_Unknown;
    }

    bool operator==(const WSStatus& other) const
    {
      return m_value == other.m_value;
    }

    bool operator!=(const WSStatus& other) const
    {
      return m_value != other.m_value;
    }

    bool operator==(HSC_t value) const
    {
      return m_value == value;
    }

    bool operator!=(HSC_t value) const
    {
      return m_value != value;
    }
    
    static WSStatus FromInt(int num)
    {
      unsigned i = 0;
      while (i < sizeof(HSC_table) / sizeof(HSC_rec))
      {
        if (num == HSC_table[i].statusInt)
          return WSStatus(HSC_table[i].value);
        ++i;
      }
      return WSStatus();
    }

    static WSStatus FromString(const std::string& str)
    {
      unsigned i = 0;
      while (i < sizeof(HSC_table) / sizeof(HSC_rec))
      {
        if (str == HSC_table[i].statusStr)
          return WSStatus(HSC_table[i].value);
        ++i;
      }
      return WSStatus();
    }

    void Set(HSC_t value) { m_value = value; }

    int GetInt() const
    {
      unsigned i = 0;
      while (i < sizeof(HSC_table) / sizeof(HSC_rec))
      {
        if (m_value == HSC_table[i].value)
          return HSC_table[i].statusInt;
        ++i;
      }
      return 0;
    }

    const char* GetString() const
    {
      unsigned i = 0;
      while (i < sizeof(HSC_table) / sizeof(HSC_rec))
      {
        if (m_value == HSC_table[i].value)
          return HSC_table[i].statusStr;
        ++i;
      }
      return NULL;
    }

    const char* GetMessage() const
    {
      unsigned i = 0;
      while (i < sizeof(HSC_table) / sizeof(HSC_rec))
      {
        if (m_value == HSC_table[i].value)
          return HSC_table[i].msgStr;
        ++i;
      }
      return NULL;
    }

  private:
    HSC_t m_value;

  };

}

#endif	/* WSSTATUS_H */

