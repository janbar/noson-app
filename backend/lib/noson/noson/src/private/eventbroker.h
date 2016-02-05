/*
 *      Copyright (C) 2015 Jean-Luc Barriere
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

#ifndef EVENTBROKER_H
#define	EVENTBROKER_H

#include <local_config.h>
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
    char* m_buffer;

  };
}


#endif	/* EVENTBROKER_H */

