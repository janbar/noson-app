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
 *  along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef MUSICSERVICES_H
#define MUSICSERVICES_H

#include <local_config.h>
#include "service.h"
#include "locked.h"

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
    friend class SMOAKeyring;
  public:
    SMAccount();
    virtual ~SMAccount();
    const std::string& GetType() const { return GetAttribut("Type"); }
    const std::string& GetSerialNum() const { return GetAttribut("SerialNum"); }
    const std::string& GetUserName() const { return GetAttribut("UN"); }
    const std::string& GetMetadata() const { return GetAttribut("MD"); }
    const std::string& GetNickname() const { return GetAttribut("NN"); }

    bool HasOACredentials() const { return !GetAttribut("Key").empty(); }

    struct OACredentials
    {
      OACredentials(const std::string& _devId, const std::string& _token, const std::string& _key)
      : devId(_devId)
      , token(_token)
      , key(_key)
      {}
      const std::string devId;  ///< unique account identifier: OADevID
      std::string token;
      std::string key;
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

    bool LoadAccounts(SMAccountList& accounts, std::string& agentStr);

    static SMAccountList GetAccountsForService(const SMAccountList& accounts, const std::string& serviceType);

  };

  //////////////////////////////////////////////////////////////////////////////
  ////
  //// Store for AppLink keyring
  ////
  class SMOAKeyring
  {
  public:
    /**
     * Add AppLink auth in keyring.
     * Known auth must be registered before getting enabled services. In other cases AppLink account won't be valid.
     * @param type The related account type
     * @param serialNum The account serial
     * @param key The key required to initiate handshake and to refresh the token
     * @param token The current token (optional)
     */
    static void Store(const std::string& type, const std::string& serialNum, const std::string& key, const std::string& token = "");

    /**
     * Load auth with keyring data for related account.
     * @param type The related account type
     * @param serialNum The account serial
     * @param key (out) key to fill
     * @param token (out) token to fill
     */
    static void Load(SMAccount& account);

    /**
     * Reset keyring data.
     */
    static void Reset();

    struct OAuth
    {
      OAuth() { /* empty auth */ }
      OAuth(  const std::string& _type, const std::string& _sn,
              const std::string& _key, const std::string& _token)
      : type(_type), serialNum(_sn), key(_key), token(_token) { }
      std::string type;
      std::string serialNum;
      std::string key;
      std::string token;
    };

    typedef std::vector<OAuth> keyring;

  private:
    static Locked<keyring> g_keyring;
  };
}

#endif /* MUSICSERVICES_H */

