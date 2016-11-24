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

    typedef std::pair<std::string, std::string> OACredentials;
    OACredentials GetOACredentials() const;
    void SetOACredentials(OACredentials auth);

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
    SMAccountPtr GetAccount() const;
    const std::string& GetAgent() const;

  private:
    std::string m_agent;    ///< The agent string to announce in API call
    SMAccountPtr m_account; ///< The account relates this service
    ElementList m_vars;
    std::string m_type;     ///< The type id of this service
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

    bool GetSessionId(const std::string& serviceId, const std::string& username, ElementList& vars);

    SMServiceList GetEnabledServices();

  private:
    bool ListAvailableServices(ElementList& vars);

    std::string m_agent;                ///< The announced agent of sonos device
    SMAccountList m_accounts;           ///< The known accounts
    std::list<ElementList> m_services;  ///< The available services

    bool ListAccounts();
    SMAccountList GetAccountsForService(const std::string& serviceType) const;
    bool ParseAvailableServiceDescriptorList(const std::string& xml);
  };
}

#endif /* MUSICSERVICES_H */

