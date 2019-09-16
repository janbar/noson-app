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
#ifndef OGGPARSER_H
#define OGGPARSER_H

#include "mediaparser.h"

namespace mediascanner
{

class OGGParser : public MediaParser
{
public:
  const char * commonName() override { return "OGG"; }
  bool match(const QFileInfo& fileInfo) override;
  bool parse(MediaFile * file, MediaInfo * info, bool debug) override;

private:
  typedef struct
  {
    unsigned char * buf;
    uint32_t size;
    unsigned char * data;
    uint32_t datalen;
  } packet_t;
  static bool resize_packet(packet_t * packet, uint32_t size);
  static bool fill_packet(packet_t * packet, uint32_t len, FILE * fp);
  static bool parse_identification(packet_t * packet, MediaInfo * info, bool debug);
  static bool parse_comment(packet_t * packet, MediaInfo * info, bool debug);
};

}

#endif /* OGGPARSER_H */

