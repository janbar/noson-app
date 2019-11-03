/*
 *      Copyright (C) 2019 Jean-Luc Barriere
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef PCMBLANKKILLER_H
#define PCMBLANKKILLER_H

#include "local_config.h"

namespace NSROOT
{

typedef void(*PCMBlankKiller)(void*, int, int);

void PCMBlankKillerNull(void * buf, int channels, int frames);

void PCMBlankKillerU8(void * buf, int channels, int frames);

void PCMBlankKillerS16LE(void * buf, int channels, int frames);
void PCMBlankKillerU16LE(void * buf, int channels, int frames);
void PCMBlankKillerS24LE(void * buf, int channels, int frames);
void PCMBlankKillerU24LE(void * buf, int channels, int frames);
void PCMBlankKillerS32LE(void * buf, int channels, int frames);
void PCMBlankKillerU32LE(void * buf, int channels, int frames);

void PCMBlankKillerS16BE(void * buf, int channels, int frames);
void PCMBlankKillerU16BE(void * buf, int channels, int frames);
void PCMBlankKillerS24BE(void * buf, int channels, int frames);
void PCMBlankKillerU24BE(void * buf, int channels, int frames);
void PCMBlankKillerS32BE(void * buf, int channels, int frames);
void PCMBlankKillerU32BE(void * buf, int channels, int frames);

}

#endif /* PCMBLANKKILLER_H */

