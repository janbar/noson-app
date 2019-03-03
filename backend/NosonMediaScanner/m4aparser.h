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
#ifndef M4APARSER_H
#define M4APARSER_H

#include "mediaparser.h"

namespace mediascanner
{

class M4AParser : public MediaParser
{
public:
  const char * commonName() override { return "M4A"; }
  bool match(const QFileInfo& fileInfo) override;
  bool parse(MediaFile * file, MediaInfo * info, bool debug) override;

private:
  static int nextChild(unsigned char * buf, uint64_t * remaining, FILE * fp, unsigned * child, uint64_t * childSize);
  static int loadDataValue(uint64_t * remaining, FILE * fp, char ** alloc, unsigned * allocSize);
  static int loadUtf8Value(uint64_t * remaining, FILE * fp, QString& str);
  static int parse_ilst(uint64_t * remaining, FILE * fp, MediaInfo * info);
  static int parse_meta(uint64_t * remaining, FILE * fp, MediaInfo * info);
  static int parse_udta(uint64_t * remaining, FILE * fp, MediaInfo * info);
  static int parse_mvhd(uint64_t * remaining, FILE * fp, MediaInfo * info);
  static int parse_moov(uint64_t * remaining, FILE * fp, MediaInfo * info);
};

}

#endif /* M4APARSER_H */

