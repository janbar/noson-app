/*
 *      Copyright (C) 2014-2016 Jean-Luc Barriere
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

#ifndef MUSICSERVICES_H
#define MUSICSERVICES_H

#include "local_config.h"
#include "service.h"
#include "smaccount.h"
#include "locked.h"

#include <list>
#include <vector>

namespace NSROOT
{

  namespace OS
  {
    class CMutex;
  }

  class SMService;

  typedef shared_ptr<SMService> SMServicePtr;
  typedef std::list<SMServicePtr> SMServiceList;

  class SMService
  {
    friend class MusicServices;
  public:
    SMService(const std::string& agent, const ElementList& vars);
    SMService(const std::string& agent, const ElementList& vars, const std::string& serialNum);
    virtual ~SMService() {}

    SMServicePtr Clone(const std::string& serialNum);

    const std::string& GetId() const;
    const std::string& GetName() const;
    const std::string& GetVersion() const;
    const std::string& GetUri() const;
    const std::string& GetSecureUri() const;
    const std::string& GetContainerType() const;
    const std::string& GetCapabilities() const;
    ElementPtr GetPolicy() const;
    ElementPtr GetStrings() const;
    ElementPtr GetPresentationMap() const;

    static std::string ServiceType(const std::string& id);

    const std::string& GetServiceType() const;
    const std::string& GetServiceDesc() const;
    SMAccountPtr GetAccount() const;
    const std::string& GetAgent() const;

  private:
    std::string m_agent;    ///< The agent string to announce in API call
    SMAccountPtr m_account; ///< The account relates this service
    ElementList m_vars;
    std::string m_type;     ///< The type id to use for this service
    mutable std::string m_desc;     ///< The sonos descriptor to use for this service
  };

  class MusicServices : public Service
  {
  public:
    MusicServices(const std::string& serviceHost, unsigned servicePort);
    ~MusicServices() {}

    static const std::string Name;
    static const std::string ControlURL;
    static const std::string EventURL;
    static const std::string SCPDURL;

    const std::string& GetName() const { return Name; }

    const std::string& GetControlURL() const { return ControlURL; }

    const std::string& GetEventURL() const { return EventURL; }

    const std::string& GetSCPDURL() const { return SCPDURL; }

    /**
     * Retrieves session info for a service
     * @param serviceId The id of the service
     * @param username The user name for the service
     * @param vars (out) The elements of the returned session
     * @return succeeded
     */
    bool GetSessionId(const std::string& serviceId, const std::string& username, ElementList& vars);

    /**
     * Returns the list of available services
     * @return The service list
     */
    SMServiceList GetAvailableServices();

    Locked<std::string>& GetVersion() { return m_version; }

  private:
    Locked<std::string> m_version;  ///< Current version

    /**
     * Query service ListAvailableServices
     * @param vars (out) Response elements
     * @return succeeded
     */
    bool ListAvailableServices(ElementList& vars);

    bool ParseAvailableServices(const ElementList& vars, std::vector<ElementList>& data);

    static SMAccountList GetAccountsForService(const SMAccountList& accounts, const std::string& serviceType);

  };

}

#endif /* MUSICSERVICES_H */

