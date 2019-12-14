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
#include "m4aparser.h"
#include "mediafile.h"
#include "mediainfo.h"
#include "byteorder.h"

#include <cstdio>
#include <string>
#include <QDebug>

#define M4A_CONTAINER    "mp4"
#define M4A_HEADER_SIZE  8


using namespace mediascanner;

static const char * _exts[] = { "M4A", "M4B", "MP4" };
static int _exts_len = sizeof(_exts) / sizeof(const char*);

bool M4AParser::match(const QFileInfo& fileInfo)
{
  QString path = fileInfo.suffix().toUpper();
  for (int i = 0; i < _exts_len; ++i)
  {
    if (path == _exts[i])
      return true;
  }
  return false;
}

bool M4AParser::parse(MediaFile * file, MediaInfo * info, bool debug)
{
  std::string path(file->filePath.toUtf8().constData());
  FILE * fp = fopen(path.c_str(), "rb");
  if (!fp)
    return false;

  bool isValid = false;
  bool isLast = false;
  info->container = M4A_CONTAINER;

  // loop over chunks until one match with requirements
  unsigned char buf[M4A_HEADER_SIZE];
  unsigned chunk;
  uint64_t size, remaining = M4A_HEADER_SIZE;
  int r;
  while (!isLast && (r = nextChild(buf, &remaining, fp, &chunk, &size)) > 0)
  {
    if (debug)
      qDebug("%s: found chunk %08x size %lu", __FUNCTION__, chunk, (unsigned long)size);
    if (chunk == 0x66747970) // ftyp
    {
      if (debug)
        qDebug("%s: processing chunk ftyp", __FUNCTION__);
      if (size < 4 || fread(buf, 1, 4, fp) != 4)
        break;
      size -= 4;
      isValid = true;
      if (memcmp(buf, "M4A ", 4) == 0)
        info->codec = "mp4a";
      else if (memcmp(buf, "M4B ", 4) == 0)
        info->codec = "mp4b";
      else
        isValid = false;
    }
    else if (chunk == 0x6d6f6f76) // moov
    {
      if (debug)
        qDebug("%s: processing chunk moov", __FUNCTION__);
      parse_moov(&size, fp, info);
      isLast = true;
      // do sanity check before exit
      if (!info->duration)
        isValid = false;
    }
    //else if (chunk == 0x6d646174) // mdat
    //{
    //  if (debug)
    //    qDebug("%s: processing chunk mdat", __FUNCTION__);
    //}

    // first chunk MUST be ftyp, else return an error
    if (!isValid || (size && fseek(fp, size, SEEK_CUR) != 0))
      break;
    // refill remaining
    remaining = M4A_HEADER_SIZE;
  }
  fclose(fp);
  if (debug)
      qDebug("%s: info:%s complete:%s", __FUNCTION__, isValid ? "true" : "false", isLast ? "true" : "false");
  // parsing is completed if all blocks have been parsed and info is valid
  if (isValid && isLast)
    return true;
  qWarning("%s: file %s failed", __FUNCTION__, path.c_str());
  return false;
}

int M4AParser::nextChild(unsigned char * buf, uint64_t * remaining, FILE * fp, unsigned * child, uint64_t * childSize)
{
  if (*remaining < M4A_HEADER_SIZE)
    return 0; // end of chunk
  if (fread(buf, 1, M4A_HEADER_SIZE, fp) == M4A_HEADER_SIZE)
  {
    *remaining -= M4A_HEADER_SIZE;
    *child = (unsigned)read32be(buf + 4);
    *childSize = (uint32_t)read32be(buf);
    if (*childSize == 1)
    {
      // size of 1 means the real size follows the header in next 8 bytes (64bits)
      if (*remaining < 8 || fread(buf, 1, 8, fp) != 8)
        return -1; // error
      *remaining -= 8;
      *childSize = (((uint64_t)read32be(buf) << 32) | (uint32_t)read32be(buf + 4)) - M4A_HEADER_SIZE - 8;
    }
    else
    {
      *childSize -= M4A_HEADER_SIZE;
    }
    if (*child > 0x20202020)
      return 1;
  }
  return -1; // error
}

int M4AParser::loadDataValue(uint64_t * remaining, FILE * fp, char ** alloc, unsigned * allocSize)
{
  unsigned char buf[M4A_HEADER_SIZE];
  unsigned child;
  uint64_t size;
  int r;
  if ((r = nextChild(buf, remaining, fp, &child, &size)) > 0)
  {
    if (*remaining < size || child != 0x64617461) // data
      return -1;
    //qDebug("%s: found chunk data size %lu", __FUNCTION__, (unsigned long)size);
    char * _alloc = new char [size];
    if (fread(_alloc, 1, size, fp) != size)
    {
      delete [] _alloc;
      return -1;
    }
    *remaining -= size;
    *allocSize = size;
    *alloc = _alloc;
    return (read32be(_alloc) & 0x00ffffff); // return datatype
  }
  return r;
}

int M4AParser::loadUtf8Value(uint64_t * remaining, FILE * fp, QString& str)
{
  char * alloc = nullptr;
  unsigned allocSize = 0;
  int r;
  if ((r = loadDataValue(remaining, fp, &alloc, &allocSize) == 1)) // 1 = datatype utf8 string
  {
    str = QString::fromUtf8(alloc + 8, allocSize - 8);
    //qDebug("%s: %s", __FUNCTION__, str.toUtf8().constData());
  }
  if (alloc)
    delete [] alloc;
  return r;
}

int M4AParser::parse_ilst(uint64_t * remaining, FILE * fp, MediaInfo * info)
{
  unsigned char buf[M4A_HEADER_SIZE];
  unsigned child;
  uint64_t size;
  int r;
  while ((r = nextChild(buf, remaining, fp, &child, &size)) > 0)
  {
    uint64_t rest = size;
    if (child == 0xa96e616d) // _nam
      loadUtf8Value(&rest, fp, info->title);
    else if (child == 0xa9616c62) // _alb
      loadUtf8Value(&rest, fp, info->album);
    else if (child == 0xa9415254) // _ART
      loadUtf8Value(&rest, fp, info->artist);
    else if (child == 0x61415254) // aART
      loadUtf8Value(&rest, fp, info->artist);
    else if (child == 0xa967656e) // _gen
      loadUtf8Value(&rest, fp, info->genre);
    else if (child == 0xa9646179) // _day
    {
      QString str;
      loadUtf8Value(&rest, fp, str);
      if (str.length() > 3)
        info->year = str.mid(0, 4).toInt();
    }
    else if (child == 0x74726b6e) // trkn
    {
      QString str;
      loadUtf8Value(&rest, fp, str);
      info->trackNo = str.toInt();
    }
    else if (child == 0x636f7672) // covr
      info->hasArt = (rest > M4A_HEADER_SIZE);

    // move to the end of child
    if (rest && fseek(fp, rest, SEEK_CUR) != 0)
      return -1;
    *remaining -= size;
  }
  return 1;
}

int M4AParser::parse_meta(uint64_t * remaining, FILE * fp, MediaInfo * info)
{
  bool exit = false;
  unsigned char buf[M4A_HEADER_SIZE];
  unsigned child;
  uint64_t size;
  int r;
  // skip flag bytes before reading children atoms
  if (*remaining < 4 || fread(buf, 1, 4, fp) != 4)
    return -1;
  *remaining -= 4;
  while (!exit && (r = nextChild(buf, remaining, fp, &child, &size)) > 0)
  {
    uint64_t rest = size;
    if (child == 0x696c7374) // ilst
    {
      parse_ilst(&rest, fp, info);
      exit = true;
    }
    // move to the end of child
    if (rest && fseek(fp, rest, SEEK_CUR) != 0)
      break;
    *remaining -= size;
  }
  return 1;
}

int M4AParser::parse_udta(uint64_t * remaining, FILE * fp, MediaInfo * info)
{
  bool exit = false;
  unsigned char buf[M4A_HEADER_SIZE];
  unsigned child;
  uint64_t size;
  int r;
  while (!exit && (r = nextChild(buf, remaining, fp, &child, &size)) > 0)
  {
    uint64_t rest = size;
    if (child == 0x6d657461) // meta
    {
      parse_meta(&rest, fp, info);
      exit = true;
    }
    // move to the end of child
    if (rest && fseek(fp, rest, SEEK_CUR) != 0)
      return -1;
    *remaining -= size;
  }
  return 1;
}

int M4AParser::parse_mvhd(uint64_t * remaining, FILE * fp, MediaInfo * info)
{
#define MVHD_SIZE 20
  unsigned char buf[MVHD_SIZE];
  if (*remaining < MVHD_SIZE || fread(buf, 1, MVHD_SIZE, fp) != MVHD_SIZE)
    return -1;
  *remaining -= MVHD_SIZE;
  unsigned scale = read32be(buf + 12);
  unsigned duration = read32be(buf + 16);
  info->duration = (int) (duration / scale);
  return 1;
}

int M4AParser::parse_moov(uint64_t * remaining, FILE * fp, MediaInfo * info)
{
  unsigned char buf[M4A_HEADER_SIZE];
  unsigned child;
  uint64_t size = 0;
  int r;
  while ((r = nextChild(buf, remaining, fp, &child, &size)) > 0)
  {
    uint64_t rest = size;
    if (child == 0x6d766864) // mvhd
    {
      parse_mvhd(&rest, fp, info);
    }
    else if (child == 0x75647461) // udta
    {
      parse_udta(&rest, fp, info);
    }
    // move to the end of child
    if (rest && fseek(fp, rest, SEEK_CUR) != 0)
      return -1;
    *remaining -= size;
  }
  return 1;
}
