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

#ifndef SONOSSUBSCRIPTION_H
#define	SONOSSUBSCRIPTION_H

#include <local_config.h>
#include "sharedptr.h"

#include <string>

#define SUBSCRIPTION_TIMEOUT 300

namespace NSROOT
{

  class Subscription
  {
  public:
    Subscription();
    Subscription(const std::string& host, unsigned port, const std::string& url, unsigned bindingPort, unsigned timeout);
    ~Subscription();

    bool IsValid();
    
    bool Start();

    const std::string GetSID() { return m_imp ? m_imp->m_SID : ""; }

    void AskRenewal();

    class SubscriptionThread
    {
      friend class Subscription;
    public:
      SubscriptionThread();
      virtual ~SubscriptionThread();
      virtual bool IsValid() = 0;
      virtual bool Start() = 0;
      virtual void Stop() = 0;
      virtual bool IsRunning() = 0;
      virtual void AskRenewal() = 0;

    protected:
      std::string m_SID;
    };

    typedef SHARED_PTR<SubscriptionThread> SubscriptionThreadPtr;

  private:
    SubscriptionThreadPtr m_imp;
  };
}

#endif	/* SONOSSUBSCRIPTION_H */

