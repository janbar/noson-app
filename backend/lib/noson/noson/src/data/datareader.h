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
#ifndef DATAREADER_H
#define DATAREADER_H

#include "local_config.h"

#include "../streamreader.h"

namespace NSROOT
{

class DataReader : public StreamReader
{
  typedef struct
  {
    const char * uri;
    const char * mime;
    const unsigned char * data;
    unsigned size;
  } Resource;
  static Resource _uris[];
  static unsigned _uris_len;
  static DataReader _instance;
  DataReader();

public:
  virtual ~DataReader() override { }
  static DataReader * Instance();

  STREAM * OpenStream(const std::string& streamUrl) override;
  int ReadStream(STREAM * stream) override;
  void CloseStream(STREAM * stream) override;
};

}

#endif // DATAREADER_H

