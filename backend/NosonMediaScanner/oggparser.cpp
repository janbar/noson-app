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
#include "oggparser.h"
#include "mediafile.h"
#include "mediainfo.h"
#include "byteorder.h"

#include <cstdio>
#include <string>
#include <QDebug>

#define OGG_CODEC        "vorbis"
#define OGG_BLOCK_SIZE   27
#define OGG_PACKET_RSVSIZE 200*1024
#define OGG_PACKET_MAXSIZE 500*1024


using namespace mediascanner;

static const char * _exts[] = { "OGG" };
static int _exts_len = sizeof(_exts) / sizeof(const char*);

bool OGGParser::match(const QFileInfo& fileInfo)
{
  QString path = fileInfo.suffix().toUpper();
  for (int i = 0; i < _exts_len; ++i)
  {
    if (path == _exts[i])
      return true;
  }
  return false;
}

bool OGGParser::parse(MediaFile * file, MediaInfo * info, bool debug)
{
  std::string path(file->filePath.toUtf8().constData());
  unsigned char buf[OGG_BLOCK_SIZE];
  unsigned char lacing[255];
  bool isLast = false;
  bool isInfoValid = false;
  bool gotoLast = false;
  packet_t packet = { nullptr, 0, 0 };
  FILE * fp = fopen(path.c_str(), "rb");
  if (!fp)
    return false;

  while (!isLast)
  {
    // check the magic file header, else close and return a null payload
    if (fread(buf, 1, OGG_BLOCK_SIZE, fp) != OGG_BLOCK_SIZE || memcmp(buf, "OggS", 4) != 0)
    {
      qWarning("%s: ERROR: bad magic header in file %s", __FUNCTION__, path.c_str());
      fclose(fp);
      return false;
    }
    //char stream_structure_version = read8(buf + 4);
    unsigned char header_type_flag = (unsigned char)read8(buf + 5);
    uint64_t granule_position = ((uint64_t)read32le(buf + 6)) + ((uint64_t)read32le(buf + 10) << 32);
    //uint32_t bitstream_serial _number = read32le(buf + 14);
    //uint32_t page_sequence_number = (uint32_t)read32le(buf + 18);
    //uint32_t CRC_checksum = read32le(buf + 22);
    unsigned char number_page_segments = (unsigned char)read8(buf + 26);

    uint32_t segment_table = 0;
    if (fread(lacing, 1, number_page_segments, fp) != number_page_segments)
    {
      qWarning("%s: file read error %s", __FUNCTION__, path.c_str());
      break;
    }

    for (int i = 0; i < number_page_segments; ++i)
      segment_table += (unsigned char)read8(lacing + i);

    if ((header_type_flag & 0x04) == 0x04)
    // bit 0x04: this is the last page of a logical bitstream (eos)
    {
      isLast = true;
      // fill last page and process current packet
      if (!fill_packet(&packet, segment_table, fp))
      {
        qWarning("%s: file read error %s", __FUNCTION__, path.c_str());
        break;
      }
    }
    else if (gotoLast)
    // bypass the page until last
    {
      fseek(fp, segment_table, SEEK_CUR);
      continue;
    }
    else if ((header_type_flag & 0x01) == 0x01)
    // bit 0x01: page contains data of a packet continued from the previous page
    {
      if (packet.size == 0)
        break;
      resize_packet(&packet, packet.datalen + segment_table);
      if (!fill_packet(&packet,segment_table, fp))
      {
        qWarning("%s: file read error %s", __FUNCTION__, path.c_str());
        break;
      }
      continue;
    }
    else if ((header_type_flag & 0x02) == 0x02)
    // bit 0x02: this is the first page of a logical bitstream (bos)
    {
      resize_packet(&packet, OGG_PACKET_RSVSIZE);
      if (!fill_packet(&packet, segment_table, fp))
      {
        qWarning("%s: file read error %s", __FUNCTION__, path.c_str());
        break;
      }
      continue;
    }

    /////////////////////////////////////////////////////////////////////////////
    //// Process the packet
    /////////////////////////////////////////////////////////////////////////////

    if (packet.datalen == 0)
    {
      qWarning("%s: ERROR: missing BOS packet in file %s", __FUNCTION__, path.c_str());
      break;
    }

    unsigned char block = (unsigned char)read8(packet.buf);
    if (debug)
      qDebug("%s: on block type %02x size %u", __FUNCTION__, block, packet.datalen);

    if ((block & 0x01) == 0x01)
    {
      // check the magic packet header, else break
      if (packet.datalen < 7 || memcmp(packet.buf + 1, "vorbis", 6) != 0)
      {
        qWarning("%s: ERROR: bad packet header in file %s", __FUNCTION__, path.c_str());
        break;
      }

      if (block == 0x01)
      {
        if (isInfoValid)
        {
          isInfoValid = false;
          break; // only one identification block is allowed
        }
        // parse identification header
        isInfoValid = parse_identification(&packet, info, debug);
        info->container = file->suffix.toLower();
      }
      else if (block == 0x03)
      {
        // parse comment header
        isInfoValid = parse_comment(&packet, info, debug);
        if (!isInfoValid)
          break;
        if (info->title.isEmpty())
          info->title = file->baseName; // default title
        // required infos have been gathered
        // so now search the last page to compute the duration of the stream
        gotoLast = true;
        packet.datalen = 0;
      }
      else
      {
        // consume data
        packet.datalen = 0;
      }
    }
    else
    {
      // consume data
      packet.datalen = 0;
    }

    if (isLast)
    {
      if (debug)
        qDebug("%s: granule_position=%" PRIu64 " sample_rate=%d", __FUNCTION__, granule_position, info->sampleRate);
      if (info->sampleRate > 0)
        info->duration = granule_position / info->sampleRate;
    }
    else if (!fill_packet(&packet, segment_table, fp))
    {
      qWarning("%s: ERROR: reading file %s", __FUNCTION__, path.c_str());
      break;
    }
  }

  if (packet.buf != nullptr)
    delete [] packet.buf;
  fclose(fp);
  if (debug)
      qDebug("%s: info:%s complete:%s", __FUNCTION__, isInfoValid ? "true" : "false", isLast ? "true" : "false");
  // parsing is completed if all blocks have been parsed and info is valid
  if (isInfoValid && isLast)
    return true;
  qWarning("%s: file %s failed", __FUNCTION__, path.c_str());
  return false;
}

bool OGGParser::resize_packet(packet_t * packet, uint32_t size)
{
  if (size <= packet->size)
    return true;
  if (size > OGG_PACKET_MAXSIZE)
    return false;
  unsigned char * _buf = new unsigned char [size];
  if (packet->buf != nullptr)
  {
    memcpy(_buf, packet->buf, packet->datalen);
    delete [] packet->buf;
  }
  packet->buf = _buf;
  packet->size = size;
  return true;
}

bool OGGParser::fill_packet(packet_t * packet, uint32_t len, FILE * fp)
{
  if (!resize_packet(packet, packet->datalen + len) ||
      fread(packet->buf + packet->datalen, 1, len, fp) != len)
    return false;
  packet->datalen += len;
  return true;
}

bool OGGParser::parse_identification(packet_t * packet, MediaInfo *info, bool debug)
{
  unsigned char * vorbis = packet->buf;
  int channels = read8(vorbis + 11);
  int sampleRate = read32le(vorbis + 12);
  int bitRateMaximum = read32le(vorbis + 16);
  int bitRateNominal = read32le(vorbis + 20);
  //int bitRateMinimum = read32le(vorbis + 24);
  if (sampleRate == 0)
    return false; // invalid sample rate
  info->codec = QString::fromUtf8(OGG_CODEC);
  info->sampleRate = sampleRate;
  info->channels = channels;
  info->bitRate = (bitRateNominal > 0 ? bitRateNominal : bitRateMaximum);
  info->duration = 0; // not set
  packet->datalen = 0; // all is consumed
  return true;
}

bool OGGParser::parse_comment(packet_t * packet, MediaInfo *info, bool debug)
{
  unsigned char * vorbis = packet->buf;
  unsigned char * ve = vorbis + packet->datalen;
  unsigned char * vp = vorbis + 7; // pass magic string
  vp += read32le(vp) + 4; // pass vendor string
  int count = read32le(vp); // comment list length;
  vp += 4;
  while (count > 0)
  {
    int len = read32le(vp);
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
    else if (str.startsWith("GENRE=", Qt::CaseInsensitive))
      info->genre = str.mid(6);
    else if (str.startsWith("TRACKNUMBER=", Qt::CaseInsensitive))
      info->trackNo = str.mid(12).toInt();
    else if (str.startsWith("DATE=", Qt::CaseInsensitive))
      info->year = str.mid(5,4).toInt(); // format should be 'yyyy-mm-dd'
    else if (str.startsWith("METADATA_BLOCK_PICTURE=", Qt::CaseInsensitive))
      info->hasArt = true;
    vp += len;
    --count;
    if (debug)
      qDebug("%s", str.toUtf8().constData());
  }
  packet->datalen = (uint32_t)(ve - vp - *vp);
  memmove(packet->buf, vp + *vp, packet->datalen);
  return (count == 0);
}
