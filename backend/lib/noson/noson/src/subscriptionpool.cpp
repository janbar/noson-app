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

#include "subscriptionpool.h"
#include "private/debug.h"

using namespace NSROOT;

SubscriptionPool::SubscriptionPool(EventHandler& eventHandler)
: m_eventHandler(eventHandler)
, m_subscriptions(SubscriptionMap())
{
  DBG(DBG_DEBUG, "%s: (%p)(%u)\n", __FUNCTION__, this, m_eventHandler.GetPort());
}

SubscriptionPool::~SubscriptionPool()
{
  Locked<SubscriptionMap>::pointer sm = m_subscriptions.Get();
  for (SubscriptionMap::iterator it = sm->begin(); it != sm->end(); ++it)
  {
    DBG(DBG_DEBUG, "%s: (%p)(%s)\n", __FUNCTION__, this, it->first.c_str());
    it->second->subscription.Stop();
    delete it->second;
  }
  sm->clear();
}

Subscription SubscriptionPool::SubscribeEvent(const std::string& host, unsigned port, const std::string& eventURL)
{
  std::string url;
  MakeSubscriptionUrl(url, host, port, eventURL, m_eventHandler.GetPort());
  Locked<SubscriptionMap>::pointer sm = m_subscriptions.Get();
  SubscriptionMap::iterator it = sm->find(url);
  if (it != sm->end())
  {
    it->second->count++;
    DBG(DBG_DEBUG, "%s: (%s)(%d)\n", __FUNCTION__, url.c_str(), it->second->count);
    // the subscription must be stopped until the caller becomes ready
    // obviously the caller is responsive for restarting it.
    it->second->subscription.Stop();
    return it->second->subscription;
  }
  DBG(DBG_DEBUG, "%s: (%s)\n", __FUNCTION__, url.c_str());
  Lease * lease = new Lease();
  lease->count = 1;
  lease->subscription = Subscription(host, port, eventURL, m_eventHandler.GetPort(), SUBSCRIPTION_TIMEOUT);
  sm->insert(std::make_pair(url, lease));
  return lease->subscription;
}

void SubscriptionPool::UnsubscribeEvent(Subscription& subscription)
{
  std::string url;
  MakeSubscriptionUrl(url, subscription.GetHost(), subscription.GetPort(), subscription.GetUrl(), subscription.GetBindingPort());
  Locked<SubscriptionMap>::pointer sm = m_subscriptions.Get();
  SubscriptionMap::iterator it = sm->find(url);
  if (it != sm->end())
  {
    it->second->count--;
    if (it->second->count > 0)
    {
      DBG(DBG_DEBUG, "%s: (%s)(%d)\n", __FUNCTION__, url.c_str(), it->second->count);
      return;
    }
    DBG(DBG_DEBUG, "%s: (%s)\n", __FUNCTION__, url.c_str());
    it->second->subscription.Stop();
    delete it->second;
    sm->erase(it);
  }
}

void SubscriptionPool::RenewSubscriptions()
{
  DBG(DBG_DEBUG, "%s\n", __FUNCTION__);
  Locked<SubscriptionMap>::pointer sm = m_subscriptions.Get();
  for (SubscriptionMap::iterator it = sm->begin(); it != sm->end(); ++it)
    it->second->subscription.AskRenewal();
}

void SubscriptionPool::MakeSubscriptionUrl(std::string& url, const std::string& host, unsigned port, const std::string& eventURL, unsigned bindingPort)
{
  url.assign(host).append(":").append(std::to_string(port)).append(eventURL).append(":").append(std::to_string(bindingPort));
}
