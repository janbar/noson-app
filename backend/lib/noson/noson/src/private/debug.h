/*
 *      Copyright (C) 2014-2015 Jean-Luc Barriere
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

#ifndef DEBUG_H
#define	DEBUG_H

#include "local_config.h"

#define DBG_NONE  -1
#define DBG_ERROR  0
#define DBG_WARN   1
#define DBG_INFO   2
#define DBG_DEBUG  3
#define DBG_PROTO  4
#define DBG_ALL    6

namespace NSROOT
{
  void DBGLevel(int l);
  void DBGAll(void);
  void DBGNone(void);
  void DBG(int level, const char* fmt, ...);
  void SetDBGMsgCallback(void (*msgcb)(int level, char*));
}

#endif	/* DEBUG_H */

