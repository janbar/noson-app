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

#ifndef MUSICSERVICES_H
#define MUSICSERVICES_H

#include <local_config.h>
#include "service.h"

#include <list>
#include <vector>

namespace NSROOT
{

  namespace OS
  {
    class CMutex;
  }

  class SMAccount : protected Element
  {
    friend class MusicServices;
  public:
    SMAccount();
    virtual ~SMAccount();
    const std::string& GetType() const { return GetAttribut("Type"); }
    const std::string& GetSerialNum() const { return GetAttribut("SerialNum"); }
    const std::string& GetUserName() const { return GetAttribut("UN"); }
    const std::string& GetMetadata() const { return GetAttribut("MD"); }
    const std::string& GetNickname() const { return GetAttribut("NN"); }

    struct OACredentials
    {
      OACredentials(const std::string& _id, const std::string& _key)
      : id(_id)
      , key(_key)
      {}
      const std::string id;
      const std::string key;
    };

    OACredentials GetOACredentials() const;
    void SetOACredentials(const OACredentials& auth);

  private:
    OS::CMutex* m_mutex;
  };

  typedef shared_ptr<SMAccount> SMAccountPtr;
  typedef std::list<SMAccountPtr> SMAccountList;

  class SMService
  {
    friend class MusicServices;
  public:
    SMService(const std::string& agent, const SMAccountPtr& account, const ElementList& vars);
    virtual ~SMService() {}

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

    static void ServiceType(const std::string& id, std::string& type);

    const std::string& GetServiceType() const;
    const std::string& GetServiceDesc() const;
    SMAccountPtr GetAccount() const;
    const std::string& GetAgent() const;


  private:
    std::string m_agent;    ///< The agent string to announce in API call
    SMAccountPtr m_account; ///< The account relates this service
    ElementList m_vars;
    std::string m_type;     ///< The type id to use for this service
    std::string m_desc;     ///< The sonos descriptor to use for this service
  };

  typedef shared_ptr<SMService> SMServicePtr;
  typedef std::list<SMServicePtr> SMServiceList;

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
     * Returns the list of enabled services
     * @return The service list
     */
    SMServiceList GetEnabledServices();

  private:
    /**
     * Query service ListAvailableServices
     * @param vars (out) Response elements
     * @return succeeded
     */
    bool ListAvailableServices(ElementList& vars);

    bool ParseAvailableServices(const ElementList& vars, std::vector<ElementList>& data);

    bool LoadAccounts(SMAccountList& accounts, std::string& agentStr);

    static SMAccountList GetAccountsForService(const SMAccountList& accounts, const std::string& serviceType);

  };
}

#endif /* MUSICSERVICES_H */

