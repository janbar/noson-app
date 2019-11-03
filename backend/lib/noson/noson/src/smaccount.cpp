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

#include "smaccount.h"
#include "private/os/threads/mutex.h"
#include "private/cppdef.h"

using namespace NSROOT;

SMAccount::SMAccount(const std::string& type)
: m_mutex(new OS::CMutex)
, m_type(type)
, m_serialNum("")
, m_auth("", "", "", "")
{
}

SMAccount::SMAccount(const std::string& type, const std::string& serialNum)
: m_mutex(new OS::CMutex)
, m_type(type)
, m_serialNum(serialNum)
, m_auth("", "", "", "")
{
}

SMAccount::~SMAccount()
{
  SAFE_DELETE(m_mutex);
}

const SMAccount::Credentials& SMAccount::GetCredentials() const
{
  OS::CLockGuard lock(*m_mutex);
  return m_auth;
}

void SMAccount::SetCredentials(const Credentials& auth)
{
  OS::CLockGuard lock(*m_mutex);
  m_auth.devId = auth.devId;
  m_auth.key = auth.key;
  m_auth.token = auth.token;
  m_auth.username = auth.username;
  // update keyring store for reuse on account reload
  SMOAKeyring::Store(m_type, m_serialNum, m_auth.key, m_auth.token, m_auth.username);
}

SMAccountList SMAccount::CreateAccounts(const std::string& type)
{
  SMAccountList list;
  // hold keyring lock until return
  Locked<SMOAKeyring::keyring>::pointer p = SMOAKeyring::g_keyring.Get();
  for (SMOAKeyring::keyring::iterator it = p->begin(); it != p->end(); ++it)
  {
    if (it->type == type)
    {
      SMAccountPtr ac(new SMAccount(it->type, it->serialNum));
      ac->SetCredentials(Credentials("", it->key, it->token, it->username));
      list.push_back(ac);
    }
  }
  return list;
}

////////////////////////////////////////////////////////////////////////////////
////
//// Store for keyring
////
SMOAKeyring::keyring empty_keyring;
Locked<SMOAKeyring::keyring> SMOAKeyring::g_keyring(empty_keyring);

void SMOAKeyring::Store(const std::string& type, const std::string& serialNum, const std::string& key, const std::string& token, const std::string& username)
{
  // hold keyring lock until return
  Locked<SMOAKeyring::keyring>::pointer p = g_keyring.Get();
  for (keyring::iterator it = p->begin(); it != p->end(); ++it)
  {
    if (it->type == type && it->serialNum == serialNum)
    {
      it->key.assign(key);
      it->token.assign(token);
      it->username.assign(username);
      return;
    }
  }
  p->push_back(Data(type, serialNum, key, token, username));
}

void SMOAKeyring::Purge(const std::string& type, const std::string& serialNum)
{
  // hold keyring lock until return
  Locked<SMOAKeyring::keyring>::pointer p = g_keyring.Get();
  for (keyring::iterator it = p->begin(); it != p->end(); ++it)
  {
    if (it->type == type && it->serialNum == serialNum)
    {
      p->erase(it);
      return;
    }
  }
}

void SMOAKeyring::Reset()
{
  g_keyring.Store(empty_keyring);
}
