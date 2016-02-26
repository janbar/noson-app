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

#include "sonostypes.h"

using namespace NSROOT;

const char* NSROOT::ProtocolTable[Protocol_unknown + 1] = {
  "x-rincon-queue",
  "x-rincon-stream",
  "x-rincon-mp3radio",
  "x-sonos-htastream",
  "x-file-cifs",
  "x-rincon",
  "http-get",
  "aac",
  ""
};

const char* NSROOT::NetServiceDescTable[NetService_unknown + 1] = {
  "SA_RINCON65031_",
  ""
};
