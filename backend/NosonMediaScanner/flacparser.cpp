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
#include "flacparser.h"
#include "mediafile.h"
#include "mediainfo.h"
#include "byteorder.h"

#include <cstdio>
#include <string>
#include <QDebug>

#define FLAC_CODEC        "flac"
#define FLAC_BLOCK_SIZE   20


using namespace mediascanner;

static const char * _exts[] = { "FLAC" };
static int _exts_len = sizeof(_exts) / sizeof(const char*);

bool FLACParser::match(const QFileInfo& fileInfo)
{
  QString path = fileInfo.suffix().toUpper();
  for (int i = 0; i < _exts_len; ++i)
  {
    if (path == _exts[i])
      return true;
  }
  return false;
}

bool FLACParser::parse(MediaFile * file, MediaInfo * info, bool debug)
{
  std::string path(file->filePath.toUtf8().constData());
  unsigned char buf[FLAC_BLOCK_SIZE];
  bool isLast = false;
  bool isInfoValid = false;
  FILE * fp = fopen(path.c_str(), "rb");
  if (!fp)
    return false;

  // check the magic file header, else close and return a null payload
  if (fread(buf, 1, 4, fp) != 4 || memcmp(buf, "fLaC", 4) != 0)
  {
    qWarning("%s: ERROR: bad magic header in file %s", __FUNCTION__, path.c_str());
    fclose(fp);
    return false;
  }
  // loop over metadata blocks until one match with requirements
  while (!isLast && fread(buf, 1, 4, fp) == 4)
  {
    // get last block flag. if true next loop will stop
    isLast = ((*buf & 0x80) != 0);
    unsigned block = *buf & 0x7f;
    // get the current block size
    unsigned offset = (read_b32be(buf) & 0xffffff);
    if (debug)
      qDebug("%s: on block type %02x size %u", __FUNCTION__, block, offset);

    /*
     * STREAMINFO
     */
    if (block == 0x00)
    {
      if (isInfoValid)
      {
        isInfoValid = false;
        break; // only one STREAMINFO block is allowed
      }
      if (fread(buf, 1, FLAC_BLOCK_SIZE, fp) != FLAC_BLOCK_SIZE)
        break;
      offset -= FLAC_BLOCK_SIZE;
      unsigned stream = read_b32be(buf + 10) >> 4;
      unsigned sampleRate = (stream & 0xffffff00) >> 8;
      unsigned channels = ((stream & 0xe0) >> 5) + 1;
      unsigned bitsPerSample = (stream & 0x1f) + 1;
      uint64_t samples = ((uint64_t)(buf[13] & 0x0f) << 32) + (uint32_t)read_b32be(buf + 14);
      if (debug)
        qDebug("%s: sr:%u ch:%u bps:%u", __FUNCTION__, sampleRate, channels, bitsPerSample);
      if (sampleRate == 0)
        break; // invalid sample rate
      info->container = QString::fromUtf8(FLAC_CODEC);
      info->codec = info->container;
      info->sampleRate = (int)sampleRate;
      info->channels = (int)channels;
      info->bitRate = (int)(sampleRate * channels * bitsPerSample);
      info->duration = (int)(samples / sampleRate);
      isInfoValid = true;
    }
    /*
     * VORBIS_COMMENT
     */
    else if (block == 0x04)
    {
      unsigned char * vorbis = new unsigned char [offset];
      unsigned char * ve = vorbis + offset;

      if (fread(vorbis, 1, offset, fp) != offset)
      {
        delete [] vorbis;
        break;
      }
      offset = 0;

      unsigned char * vp = vorbis;
      vp += read_b32le(vp) + 4; // pass vendor string
      unsigned count = read_b32le(vp); // comment list length;
      vp += 4;
      while (count > 0)
      {
        unsigned len = read_b32le(vp);
        vp += 4;
        if ((vp + len) > ve)
          break; // buffer overflow
        QString str = QString::fromUtf8(reinterpret_cast<char*>(vp), len);
        if (str.startsWith("TITLE=", Qt::CaseInsensitive))
          info->title = str.mid(6);
        else if (str.startsWith("ARTIST=", Qt::CaseInsensitive))
          info->artist = str.mid(7);
        else if (str.startsWith("ALBUM=", Qt::CaseInsensitive))
          info->album = str.mid(6);
        else if (str.startsWith("ALBUMARTIST=", Qt::CaseInsensitive))
          info->albumArtist = str.mid(12);
        else if (str.startsWith("GENRE=", Qt::CaseInsensitive))
          info->genre = str.mid(6);
        else if (str.startsWith("COMPOSER=", Qt::CaseInsensitive))
          info->composer = str.mid(9);
        else if (str.startsWith("TRACKNUMBER=", Qt::CaseInsensitive))
          info->trackNo = str.mid(12).toInt();
        else if (str.startsWith("DATE=", Qt::CaseInsensitive))
          info->year = str.mid(5,4).toInt(); // format should be 'yyyy-mm-dd'
        else if (str.startsWith("DISCNUMBER=", Qt::CaseInsensitive))
          info->discNo = str.mid(11).toInt();
        vp += len;
        --count;
        if (debug)
          qDebug("%s", str.toUtf8().constData());
      }

      delete [] vorbis;

      if (count)
      {
        isInfoValid = false;
        break; // parsing vorbis comments failed
      }
      if (info->title.isEmpty())
        info->title = file->baseName; // default title
    }
    /*
     * PICTURE
     */
    else if (block == 0x06)
    {
      if (fread(buf, 1, 4, fp) != 4)
        break;
      offset -= 4;
      if (debug)
        qDebug("%s: picture type %02x", __FUNCTION__, buf[3]);
      info->hasArt = true;
    }

    // first block MUST be STREAMINFO, else return an error
    if (!isInfoValid || fseek(fp, offset, SEEK_CUR) != 0)
      break;
  }
  fclose(fp);
  if (debug)
      qDebug("%s: info:%s complete:%s", __FUNCTION__, isInfoValid ? "true" : "false", isLast ? "true" : "false");
  // parsing is completed if all blocks have been parsed and info is valid
  return (isInfoValid && isLast);
}
