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
 *  along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef SMACCOUNT_H
#define SMACCOUNT_H

#include "local_config.h"
#include "sharedptr.h"
#include "locked.h"

#include <string>
#include <list>
#include <vector>

namespace NSROOT
{

  namespace OS
  {
    class CMutex;
  }

  class SMAccount;

  typedef shared_ptr<SMAccount> SMAccountPtr;
  typedef std::list<SMAccountPtr> SMAccountList;

  class SMAccount
  {
  public:
    SMAccount(const std::string& type);
    SMAccount(const std::string& type, const std::string& serialNum);
    virtual ~SMAccount();
    const std::string& GetType() const { return m_type; }
    const std::string& GetSerialNum() const { return m_serialNum; }

    struct Credentials
    {
      Credentials(const std::string& _devId, const std::string& _key, const std::string& _token, const std::string& _username)
      : devId(_devId)
      , key(_key)
      , token(_token)
      , username(_username)
      {}
      std::string devId;
      std::string key;
      std::string token;
      std::string username;
    };

    const Credentials& GetCredentials() const;
    void SetCredentials(const Credentials& auth);

    static SMAccountList CreateAccounts(const std::string& type);

  private:
    OS::CMutex* m_mutex;
    std::string m_type;
    std::string m_serialNum;
    Credentials m_auth;
  };

  //////////////////////////////////////////////////////////////////////////////
  ////
  //// Store for keyring
  ////
  class SMOAKeyring
  {
    friend class SMAccount;
  public:
    /**
     * Add auth in keyring.
     * Known auth must be registered before getting enabled services. In other cases AppLink account won't be valid.
     * @param type The related account type
     * @param serialNum The account serial
     * @param key The key required to initiate handshake and to refresh the token
     * @param token The current token (optional)
     */
    static void Store(const std::string& type, const std::string& serialNum, const std::string& key, const std::string& token, const std::string& username);
    static void Purge(const std::string& type, const std::string& serialNum);

    /**
     * Reset keyring data.
     */
    static void Reset();

    struct Data
    {
      Data() { /* empty auth */ }
      Data(  const std::string& _type, const std::string& _sn,
              const std::string& _key, const std::string& _token,
             const std::string& _username)
      : type(_type), serialNum(_sn), key(_key), token(_token), username(_username) { }
      std::string type;
      std::string serialNum;
      std::string key;
      std::string token;
      std::string username;
    };

    typedef std::vector<Data> keyring;

  private:
    static Locked<keyring> g_keyring;
  };

}

#endif /* SMACCOUNT_H */

