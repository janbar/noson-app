/*
 * Copyright (C) 2015 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: James Henstridge <james.henstridge@canonical.com>
 *              Michi Henning <michi.henning@canonical.com>
 */

#ifndef RATELIMITER_H
#define RATELIMITER_H

#include <functional>
#include <memory>
#include <list>

namespace thumbnailer
{

  // RateLimiter is a simple class to control the level of concurrency
  // of asynchronous jobs.  It performs no locking because it is only
  // intended to be run from the event loop thread.

  class RateLimiter
  {
  public:
    RateLimiter(int concurrency);
    ~RateLimiter();

    RateLimiter(RateLimiter const&) = delete;
    RateLimiter& operator=(RateLimiter const&) = delete;

    typedef std::function<bool() noexcept> CancelFunc;

    // Schedule a job to run.  If the concurrency limit has not been
    // reached, the job will be run immediately.  Otherwise it will be
    // added to the queue. Return value is a function that, when
    // called, cancels the job in the queue (if it's still in the queue).
    // The cancel function returns true if the request could be cancelled because
    // it was still waiting, false otherwise.
    CancelFunc schedule(std::function<void() > job);

    // Schedule a job to run immediately, regardless of the concurrency limit.
    CancelFunc schedule_now(std::function<void() > job);

    // Notify that a job has completed. If there are queued jobs,
    // start the one at the head of the queue. Every call to schedule()
    // and schedule_now() *must* be matched by exactly one call to done(),
    // unless the call is cancelled. If the call is cancelled, done() must
    // be called only if the cancel function returns false.
    void done();

  private:
    int const concurrency_; // Max number of outstanding requests.
    int running_; // Actual number of outstanding requests.
    // We store a shared_ptr so we can detect on cancellation
    // whether a job completed before it was cancelled.
    std::list<std::shared_ptr<std::function<void()>>> list_;
  };

} // namespace thumbnailer
#endif /* RATELIMITER_H */

