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

#ifndef EVENTBROKER_H
#define	EVENTBROKER_H

#include "local_config.h"
#include "os/threads/threadpool.h"
#include "wsrequestbroker.h"
#include "socket.h"
#include "../eventhandler.h"
#include "../sharedptr.h"

namespace NSROOT
{

  class EventBroker : public OS::CWorker
  {
  public:
    EventBroker(EventHandler::EventHandlerThread* handler, SHARED_PTR<TcpSocket>& sockPtr);
    virtual ~EventBroker();
    virtual void Process();

  private:
    EventHandler::EventHandlerThread* m_handler;
    SHARED_PTR<TcpSocket> m_sockPtr;
  };
}


#endif	/* EVENTBROKER_H */

