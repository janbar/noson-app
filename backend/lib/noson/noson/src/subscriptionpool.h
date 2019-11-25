/*
 *      Copyright (C) 2019 Jean-Luc Barriere
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

#ifndef SUBSCRIPTIONPOOL_H
#define SUBSCRIPTIONPOOL_H

#include "local_config.h"
#include "subscription.h"
#include "eventhandler.h"
#include "sharedptr.h"

#include <map>

namespace NSROOT
{

  class SubscriptionPool;

  typedef SHARED_PTR<SubscriptionPool> SubscriptionPoolPtr;
   
  class SubscriptionPool
  {
  public :
    SubscriptionPool(EventHandler& eventHandler);
    ~SubscriptionPool();

    EventHandler& GetEventHandler() { return m_eventHandler; }

    Subscription SubscribeEvent(const std::string& host, unsigned port, const std::string& eventURL);
    void UnsubscribeEvent(Subscription& subscription);

    void RenewSubscriptions();

  private:
    EventHandler m_eventHandler;

    struct Lease { Subscription subscription; int count; };

    typedef std::map<std::string, Lease*> SubscriptionMap;
    Locked<SubscriptionMap> m_subscriptions;

    static void MakeSubscriptionUrl(std::string& url, const std::string& host, unsigned port, const std::string& eventURL, unsigned bindingPort);
  };

}

#endif /* SUBSCRIPTIONPOOL_H */

