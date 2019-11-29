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

#ifndef SONOSSUBSCRIPTION_H
#define	SONOSSUBSCRIPTION_H

#include "local_config.h"
#include "sharedptr.h"

#include <string>

#define SUBSCRIPTION_TIMEOUT_MIN  60
#define SUBSCRIPTION_TIMEOUT_MAX  3600
#define SUBSCRIPTION_TIMEOUT      SUBSCRIPTION_TIMEOUT_MAX
#define SUBSCRIPTION_RENEW_PCT    90

namespace NSROOT
{

  class Subscription
  {
  public:
    Subscription();
    Subscription(const std::string& host, unsigned port, const std::string& url, unsigned bindingPort, unsigned ttl);
    ~Subscription();

    bool IsValid();

    bool Start();

    void Stop();

    const std::string& GetSID();

    const std::string& GetHost();

    unsigned GetPort();

    const std::string& GetUrl();

    unsigned GetBindingPort();

    void AskRenewal();

    class SubscriptionThread
    {
    public:
      SubscriptionThread();
      virtual ~SubscriptionThread();
      virtual bool IsValid() = 0;
      virtual bool Start() = 0;
      virtual void Stop() = 0;
      virtual bool IsRunning() = 0;
      virtual void AskRenewal() = 0;
      virtual const std::string& GetSID() = 0;
      virtual const std::string& GetHost() = 0;
      virtual unsigned GetPort() = 0;
      virtual const std::string& GetUrl() = 0;
      virtual unsigned GetBindingPort() = 0;
    };

    typedef SHARED_PTR<SubscriptionThread> SubscriptionThreadPtr;

  private:
    SubscriptionThreadPtr m_imp;
  };
}

#endif	/* SONOSSUBSCRIPTION_H */

