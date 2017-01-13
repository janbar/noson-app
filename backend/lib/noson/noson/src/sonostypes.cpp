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

#include "sonostypes.h"

using namespace NSROOT;

const char* NSROOT::ProtocolTable[Protocol_unknown + 1] = {
  "x-rincon-queue",
  "x-rincon-stream",
  "x-rincon-mp3radio",
  "x-rincon-playlist",
  "x-rincon-cpcontainer",
  "x-sonos-htastream",
  "x-sonos-http",
  "x-sonos-spotify",
  "x-sonos-mms",
  "x-sonosapi-stream",
  "x-sonosapi-radio",
  "x-sonosapi-rtrecent",
  "x-file-cifs",
  "x-rincon",
  "http-get",
  "file",
  "aac",
  ""
};

const char* NSROOT::ServiceDescTable[ServiceDesc_unknown + 1] = {
  "RINCON_AssociatedZPUDN",
  ""
};
