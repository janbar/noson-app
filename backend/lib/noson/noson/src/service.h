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

#ifndef SERVICE_H
#define	SERVICE_H

#include <local_config.h>
#include "sonostypes.h"
#include "element.h"

#include <string>

namespace NSROOT
{
  namespace OS
  {
    class CMutex;
  }

  class Service
  {
  public:
    Service(const std::string& serviceHost, unsigned servicePort);
    virtual ~Service();

    const std::string& GetHost() { return m_host; }

    unsigned GetPort() { return m_port; }

    virtual const std::string& GetName() const = 0;

    virtual const std::string& GetControlURL() const = 0;

    virtual const std::string& GetEventURL() const = 0;

    virtual const std::string& GetSCPDURL() const = 0;

    ElementList GetLastFault();

  protected:
    std::string m_host;
    unsigned m_port;

    ElementList Request(const std::string& action, const ElementList& args);

  private:
    OS::CMutex* m_mutex;
    ElementList m_fault;

    void SetFault(const ElementList& vars);

    // prevent copy
    Service(const Service&);
    Service& operator=(const Service&);
  };
}

#endif	/* SERVICE_H */

