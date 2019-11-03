/*
 *      Copyright (C) 2018-2019 Jean-Luc Barriere
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
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "filepicreader.h"
#include "private/debug.h"
#include "private/tokenizer.h"
#include "private/urlencoder.h"
#include "private/byteorder.h"
#include "private/base64.h"

#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <ctype.h>

using namespace NSROOT;

#define MAX_PICTURE_SIZE  0x1fffff

FilePicReader::Picture::Picture()
: payload(nullptr), free(nullptr), mime(nullptr), data(nullptr), size(0)
{
}

FilePicReader::Picture::~Picture()
{
  if (free) free(payload);
}

FilePicReader FilePicReader::_instance;

FilePicReader::FilePicReader()
{
}

FilePicReader* FilePicReader::Instance()
{
  return &_instance;
}

StreamReader::STREAM * FilePicReader::OpenStream(const std::string& streamUrl)
{
  std::vector<std::string> params;
  readParameters(streamUrl, params);
  std::string filePath = getParamValue(params, FILEPICREADER_PARAM_PATH);
  std::string typeString = getParamValue(params, FILEPICREADER_PARAM_TYPE);
  DBG(DBG_DEBUG, "%s: path (%s) type (%s)\n", __FUNCTION__, filePath.c_str(), typeString.c_str());

  PictureType picType = PictureType::CoverFront;
  if (!typeString.empty())
    picType = (PictureType) atoi(typeString.c_str());

  size_t dot = filePath.find_last_of('.');
  if (dot == std::string::npos)
    return nullptr;
  std::string _suffix = filePath.substr(dot + 1);
  std::string suffix;
  size_t p = 0;
  while (p < _suffix.length())
    suffix.push_back(tolower(_suffix.at(p++)));

  bool error = true;

  if (suffix.compare("flac") == 0)
  {
    Picture * picture = ExtractFLACPicture(filePath, picType, error);
    if (!picture && !error)
      picture = ExtractFLACPicture(filePath, PictureType::Any, error);
    if (picture)
    {
      STREAM * stream = new STREAM();
      stream->opaque = picture;
      stream->contentType = picture->mime;
      stream->contentLength = picture->size;
      stream->data = nullptr;
      stream->size = 0;
      return stream;
    }
  }
  if (suffix.compare("mp3") == 0)
  {
    Picture * picture = ExtractID3Picture(filePath, picType, error);
    if (!picture && !error)
      picture = ExtractID3Picture(filePath, PictureType::Any, error);
    if (picture)
    {
      STREAM * stream = new STREAM();
      stream->opaque = picture;
      stream->contentType = picture->mime;
      stream->contentLength = picture->size;
      stream->data = nullptr;
      stream->size = 0;
      return stream;
    }
  }
  if (suffix.compare("ogg") == 0)
  {
    Picture * picture = ExtractOGGSPicture(filePath, picType, error);
    if (!picture && !error)
      picture = ExtractOGGSPicture(filePath, PictureType::Any, error);
    if (picture)
    {
      STREAM * stream = new STREAM();
      stream->opaque = picture;
      stream->contentType = picture->mime;
      stream->contentLength = picture->size;
      stream->data = nullptr;
      stream->size = 0;
      return stream;
    }
  }

  // error flag has been cleaned but no picture has been found
  // return null stream
  if (!error)
  {
    STREAM * stream = new STREAM();
    stream->opaque = nullptr;
    stream->contentType = nullptr;
    stream->contentLength = 0;
    stream->data = nullptr;
    stream->size = 0;
    return stream;
  }

  return nullptr;
}

int FilePicReader::ReadStream(STREAM* stream)
{
  if (!stream)
    return -1;
  if (stream->opaque)
  {
    Picture * picture = static_cast<Picture*>(stream->opaque);
    if (!stream->data)
      stream->data = picture->data;
    else
      stream->data += stream->size;
    int r = static_cast<int>(picture->size - (stream->data - picture->data));
    stream->size = r > 0 ? static_cast<unsigned>(r) : 0;
    return r;
  }
  return 0;
}

void FilePicReader::CloseStream(STREAM* stream)
{
  if (stream)
  {
    stream->data = nullptr;
    stream->contentType = nullptr;
    stream->size = 0;
    if (stream->opaque)
    {
      delete static_cast<Picture*>(stream->opaque);
      stream->opaque = nullptr;
    }
    delete stream;
  }
}

void FilePicReader::readParameters(const std::string& streamUrl, std::vector<std::string>& params)
{
  size_t s = streamUrl.find('?');
  if (s != std::string::npos)
    tokenize(streamUrl.substr(s + 1), "&", params, true);
}

std::string FilePicReader::getParamValue(const std::vector<std::string>& params, const std::string& name)
{
  size_t lval = name.length() + 1;
  for (const std::string& str : params)
  {
    if (str.length() > lval && str.at(name.length()) == '=' && str.compare(0, name.length(), name) == 0)
      return urldecode(str.substr(lval));
  }
  return std::string();
}


/////////////////////////////////////////////////////////////////////////////
//// Media file FLAC/Vorbis
/////////////////////////////////////////////////////////////////////////////

FilePicReader::Picture * FilePicReader::ExtractFLACPicture(const std::string& filePath, PictureType pictureType, bool& error)
{
  unsigned char buf[8];
  bool isLast = false;
  Picture * pic = nullptr;
  FILE * file = fopen(filePath.c_str(), "rb");
  if (!file)
  {
    DBG(DBG_INFO, "%s: file not found (%s)\n", __FUNCTION__, filePath.c_str());
    error = true;
    return pic;
  }
  // check the magic file header, else close and return a null payload
  if (fread(buf, 1, 4, file) != 4 || memcmp(buf, "fLaC", 4) != 0)
  {
    DBG(DBG_INFO, "%s: bad magic header (%s)\n", __FUNCTION__, filePath.c_str());
    fclose(file);
    error = true;
    return pic;
  }
  // loop over metadata blocks until one match with requirements
  while (!isLast && fread(buf, 1, 8, file) == 8)
  {
    // get last block flag. if true next loop will stop
    isLast = ((*buf & 0x80) != 0);
    // get the current block size
    unsigned v = (read32be(buf) & 0xffffff) - 4;
    if (v > MAX_PICTURE_SIZE)
      break;

    DBG(DBG_PROTO, "%s: block type (%.2x) size (%u)\n", __FUNCTION__, buf[0], v);
    // check block type as PICTURE
    if ((*buf & 0x7f) == 0x06)
    {
      DBG(DBG_PROTO, "%s: picture type (%.2x)\n", __FUNCTION__, buf[7]);
      // check picture type matches with requirement
      if (read32be(buf + 4) == pictureType || pictureType == -1)
      {
        unsigned mime_type_len;
        unsigned desc_len;
        unsigned data_len;
        char * picbuf = new char [v];
        // read data block and check for sanity
        if (fread(picbuf, 1, v, file) != v ||
                (mime_type_len = read32be(picbuf)) > v - 4 ||
                (desc_len = read32be(picbuf + 4 + mime_type_len)) > v - 8 - mime_type_len ||
                (data_len = read32be(picbuf + mime_type_len + desc_len + 24)) > v - 28 - desc_len - mime_type_len)
        {
          delete [] picbuf;
          break;
        }
        pic = new Picture();
        pic->payload = picbuf; // allocated pointer
        pic->free = &FilePicReader::FreeFLACPicture; // handler to free the payload
        pic->mime = picbuf + 4; // mime type string
        picbuf[mime_type_len + 4] = 0; // terminate the mime type string with zero
        pic->data = picbuf + mime_type_len + desc_len + 28; // image data
        pic->size = data_len; // image data length
        DBG(DBG_PROTO, "%s: found picture (%s) size (%u)\n", __FUNCTION__, pic->mime, pic->size);
        break;
      }
    }
    if (fseek(file, v, SEEK_CUR) != 0)
      break;
  }
  fclose(file);
  error = (!isLast && pic == nullptr);
  return pic;
}

void FilePicReader::FreeFLACPicture(void * payload)
{
  assert(payload);
  char * pic = static_cast<char*>(payload);
  delete [] pic;
}


/////////////////////////////////////////////////////////////////////////////
//// Media file MPEG/ID3
/////////////////////////////////////////////////////////////////////////////

FilePicReader::Picture * FilePicReader::ExtractID3Picture(const std::string& filePath, PictureType pictureType, bool& error)
{
  long id3v2_offset;
  off_t sync_offset = 0;
  Picture * pic = nullptr;
  FILE * file = fopen(filePath.c_str(), "rb");
  if (!file)
  {
    DBG(DBG_INFO, "%s: file not found (%s)\n", __FUNCTION__, filePath.c_str());
    error = true;
    return pic;
  }

  id3v2_offset = find_id3v2(file, &sync_offset);
  if (id3v2_offset < 0)
    error = true;
  else
  {
    off_t id3v2_size = 3;
    sync_offset = id3v2_offset;
    error = (parse_id3v2(file, id3v2_offset, &pic, &id3v2_size, pictureType) != 0);
  }

  fclose(file);
  return pic;
}

void FilePicReader::FreeID3Picture(void * payload)
{
  assert(payload);
  char * pic = static_cast<char*>(payload);
  delete [] pic;
}

static inline unsigned int _to_uint_max7b(const char * data, int data_size)
{
  unsigned int sum = 0;
  unsigned int last, i;

  last = data_size > 4 ? 3 : data_size - 1;

  for (i = 0; i <= last; i++)
    sum |= ((unsigned char) data[i]) << ((last - i) * 7);

  return sum;
}

static inline int _is_id3v2_second_synch_byte(unsigned char byte)
{
  if (byte == 0xff)
    return 0;
  if ((byte & 0xe0) == 0xe0)
    return 1;
  return 0;
}

static inline unsigned int _get_id3v2_frame_header_size(unsigned int version)
{
  switch (version)
  {
  case 0:
  case 1:
  case 2:
    return 6;
  case 3:
  case 4:
  default:
    return 10;
  }
}

struct ID3v2FrameHeader
{
  char frame_id[4];
  unsigned int frame_size;
  int compression;
  int data_length_indicator;
};

static inline void _parse_id3v2_frame_header(char * data, unsigned int version, struct ID3v2FrameHeader * fh)
{
  switch (version)
  {
  case 0:
  case 1:
  case 2:
    memcpy(fh->frame_id, data, 3);
    fh->frame_id[3] = 0;
    fh->frame_size = (unsigned)read24be(data + 3);
    fh->compression = 0;
    fh->data_length_indicator = 0;
    break;
  case 3:
    memcpy(fh->frame_id, data, 4);
    fh->frame_size = (unsigned)read32be(data + 4);
    fh->compression = data[9] & 0x40;
    fh->data_length_indicator = 0;
    break;
  case 4:
  default:
    memcpy(fh->frame_id, data, 4);
    fh->frame_size = (unsigned)read32be(data + 4);
    fh->compression = data[9] & 0x4;
    fh->data_length_indicator = data[9] & 0x1;
    break;
  }
}

#define ID3V2_HEADER_SIZE     10
#define ID3V2_FOOTER_SIZE     10
#define ID3V2_STACK_BUFFER    40

long FilePicReader::find_id3v2(FILE * file, off_t * sync_offset)
{
  static const char pattern[3] = {'I', 'D', '3'};
  char buffer[3];
  unsigned int prev_part_match, prev_part_match_sync = 0;
  long buffer_offset;

  if (fread(buffer, 1, sizeof(buffer), file) != sizeof(buffer))
    return -1;

  if (memcmp(buffer, pattern, sizeof(pattern)) == 0)
    return 0;

  /* This loop is the crux of the find method.  There are three cases that we
   * want to account for:
   * (1) The previously searched buffer contained a partial match of the
   * search pattern and we want to see if the next one starts with the
   * remainder of that pattern.
   *
   * (2) The search pattern is wholly contained within the current buffer.
   *
   * (3) The current buffer ends with a partial match of the pattern.  We will
   * note this for use in the next iteration, where we will check for the rest
   * of the pattern.
   */
  buffer_offset = 0;
  prev_part_match_sync = 0;
  prev_part_match = 0;
  for(;;)
  {
    const char *p, *p_end;

    /* (1) previous partial match */
    if (prev_part_match_sync)
    {
      if (_is_id3v2_second_synch_byte(buffer[0]))
      {
        *sync_offset = buffer_offset - 1;
        return -1;
      }
      prev_part_match_sync = 0;
    }

    if (prev_part_match)
    {
      const int size = sizeof(buffer) - prev_part_match;
      const char *part_pattern = pattern + prev_part_match;

      if (memcmp(buffer, part_pattern, size) == 0)
        return buffer_offset - prev_part_match;

      prev_part_match = 0;
    }

    p_end = buffer + sizeof(buffer);
    for (p = buffer; p < p_end; p++)
    {
      if (*p == pattern[0])
      {
        /* Try to match pattern, possible partial contents */
        const char *q;
        int todo;

        q = p + 1;
        todo = p_end - q;
        if (todo == 0 || memcmp(q, pattern + 1, todo) == 0)
        {
          todo++;
          if (todo == sizeof(buffer))
            /* (2) pattern contained in current buffer */
            return buffer_offset;

          /* (3) partial match */
          prev_part_match = todo;
          break;
        }
      }
      else if ((unsigned char) *p == 0xff)
      {
        /* Try to match synch pattern, possible partial contents */
        const char *q;

        q = p + 1;
        if (q < p_end)
        {
          if (_is_id3v2_second_synch_byte(*q))
          {
            /* (2) synch pattern contained in current buffer */
            *sync_offset = buffer_offset + (p - buffer);
            return -1;
          }
        }
        else
          /* (3) partial match */
          prev_part_match_sync = 1;
      }
    }

    if (fread(buffer, 1, sizeof(buffer), file) != sizeof(buffer))
      return -1;
    buffer_offset += sizeof(buffer);
  }

  return -1;
}

int FilePicReader::parse_id3v2_pic_v2(FILE * file, unsigned frame_size, Picture ** pic, PictureType pictureType)
{
  static const char * mime_types[2] = { "image/png" , "image/jpeg" };
  const char * mime_type = nullptr;
  char buffer[ID3V2_STACK_BUFFER];
  unsigned remain_size = frame_size;
  if (fread(buffer, 1, sizeof(buffer), file) != sizeof(buffer))
    return -1;
  remain_size -= sizeof(buffer);

  if (buffer[1] == 'P')
    mime_type = mime_types[0];
  else if (buffer[1] == 'J')
    mime_type = mime_types[1];

  if (mime_type &&
          (buffer[4] == (int)pictureType || pictureType == PictureType::Any))
  {
    static const char csend[2] = { '\0', '\0' };
    unsigned csz;
    unsigned desc_len = 0;
    unsigned data_len;
    char * picbuf = new char [frame_size];
    memcpy(picbuf, buffer, sizeof(buffer));
    if (fread(picbuf + sizeof(buffer), 1, remain_size, file) != remain_size)
    {
      delete [] picbuf;
      return -1;
    }

    switch(picbuf[0]) // text encoding
    {
    case 0x1: // UTF16
    case 0x2: // UTF16BE
    case 0x4: // UTF16LE
      csz = 2;
      break;
    default:
      csz = 1;
    }

    while (memcmp(&picbuf[5 + desc_len], csend, csz) != 0 && desc_len < (frame_size - 5 - csz))
      desc_len += csz;

    data_len = frame_size - 5 - csz - desc_len;
    Picture * p = new Picture();
    p->payload = picbuf; // allocated pointer
    p->free = &FilePicReader::FreeID3Picture; // handler to free the payload
    p->mime = mime_type;
    p->data = picbuf + desc_len + csz + 5; // image data
    p->size = data_len; // image data length
    DBG(DBG_PROTO, "%s: found picture (%s) size (%u)\n", __FUNCTION__, p->mime, p->size);
    *pic = p;
    return 0;
  }
  else
  {
    fseek(file, remain_size, SEEK_CUR);
  }
  return 0;
}

int FilePicReader::parse_id3v2_pic_v3(FILE * file, unsigned frame_size, Picture ** pic, PictureType pictureType)
{
  char buffer[ID3V2_STACK_BUFFER];
  unsigned remain_size = frame_size;
  unsigned mime_type_len = 0;
  if (fread(buffer, 1, sizeof(buffer), file) != sizeof(buffer))
    return -1;
  remain_size -= sizeof(buffer);

  while (buffer[mime_type_len + 1] != 0 && mime_type_len < sizeof(buffer) - 3)
    ++mime_type_len;

  if (buffer[mime_type_len + 1] == 0 &&
          (buffer[mime_type_len + 2] == (int)pictureType || pictureType == PictureType::Any))
  {
    static const char csend[2] = { '\0', '\0' };
    unsigned csz;
    unsigned desc_len = 0;
    unsigned data_len;
    char * picbuf = new char [frame_size];
    memcpy(picbuf, buffer, sizeof(buffer));
    if (fread(picbuf + sizeof(buffer), 1, remain_size, file) != remain_size)
    {
      delete [] picbuf;
      return -1;
    }

    switch(picbuf[0]) // text encoding
    {
    case 0x1: // UTF16
    case 0x2: // UTF16BE
    case 0x4: // UTF16LE
      csz = 2;
      break;
    default:
      csz = 1;
    }

    while (memcmp(&picbuf[mime_type_len + 3 + desc_len], csend, csz) != 0 && desc_len < (frame_size - 3 - csz - mime_type_len))
      desc_len += csz;

    data_len = frame_size - 3 - csz - mime_type_len - desc_len;
    Picture * p = new Picture();
    p->payload = picbuf; // allocated pointer
    p->free = &FilePicReader::FreeID3Picture; // handler to free the payload
    p->mime = picbuf + 1; // mime type string
    picbuf[mime_type_len + 1] = 0; // terminate the mime type string with zero
    p->data = picbuf + mime_type_len + desc_len + csz + 3; // image data
    p->size = data_len; // image data length
    DBG(DBG_PROTO, "%s: found picture (%s) size (%u)\n", __FUNCTION__, p->mime, p->size);
    *pic = p;
    return 0;
  }
  else
  {
    fseek(file, remain_size, SEEK_CUR);
  }
  return 0;
}

int FilePicReader::parse_id3v2(FILE * file, long id3v2_offset, Picture ** pic, off_t * ptag_size, PictureType pictureType)
{
  char header_data[ID3V2_HEADER_SIZE], frame_header_data[10];
  unsigned int tag_size, major_version, frame_data_pos, frame_data_length, frame_header_size;
  int extended_header, footer_present;
  struct ID3v2FrameHeader fh;
  size_t nread;

  fseek(file, id3v2_offset, SEEK_SET);

  /* parse header */
  if (fread(header_data, 1, ID3V2_HEADER_SIZE, file) != ID3V2_HEADER_SIZE)
    return -1;

  tag_size = _to_uint_max7b(header_data + 6, 4);
  if (tag_size == 0)
    return -1;

  *ptag_size = tag_size + ID3V2_HEADER_SIZE;

  /* parse frames */
  major_version = header_data[3];

  frame_data_pos = 0;
  frame_data_length = tag_size;

  /* check for extended header */
  extended_header = header_data[5] & 0x20; /* bit 6 */
  if (extended_header)
  {
    /* skip extended header */
    unsigned int extended_header_size;
    char extended_header_data[6];
    bool crc;

    if (fread(extended_header_data, 1, 4, file) != 4)
      return -1;

    extended_header_size = (unsigned)read32be(extended_header_data);
    crc = extended_header_data[5] & 0x8000;

    *ptag_size += extended_header_size + (crc * 4);

    fseek(file, extended_header_size - 6, SEEK_CUR);
    frame_data_pos += extended_header_size;
    frame_data_length -= extended_header_size;
  }

  footer_present = header_data[5] & 0x8; /* bit 4 */
  if (footer_present && frame_data_length > ID3V2_FOOTER_SIZE)
    frame_data_length -= ID3V2_FOOTER_SIZE;

  frame_header_size = _get_id3v2_frame_header_size(major_version);
  while (frame_data_pos < frame_data_length - frame_header_size)
  {
    nread = fread(frame_header_data, 1, frame_header_size, file);
    if (nread == 0)
      break;

    if (nread != frame_header_size)
      return -1;

    if (frame_header_data[0] == 0)
      break;

    _parse_id3v2_frame_header(frame_header_data, major_version, &fh);

    if (fh.data_length_indicator)
      fseek(file, 4, SEEK_CUR);

    DBG(DBG_PROTO, "%s: version (%u) frame (%c%c%c%c) size (%u)\n", __FUNCTION__,
            major_version, fh.frame_id[0],fh.frame_id[1],fh.frame_id[2],fh.frame_id[3],
            fh.frame_size);

    if (fh.frame_size > MAX_PICTURE_SIZE || fh.frame_size < ID3V2_STACK_BUFFER || fh.compression)
    {
      fseek(file, fh.frame_size, SEEK_CUR);
    }
    else if (major_version < 0x3 && memcmp(fh.frame_id, "PIC", 3) == 0)
    {
      if (parse_id3v2_pic_v2(file, fh.frame_size, pic, pictureType) != 0)
        return -1;
      if (*pic)
        return 0;
    }
    else if (major_version > 0x2 && major_version < 0x5 && memcmp(fh.frame_id, "APIC", 4) == 0)
    {
      if (parse_id3v2_pic_v3(file, fh.frame_size, pic, pictureType) != 0)
        return -1;
      if (*pic)
        return 0;
    }
    else
    {
      fseek(file, fh.frame_size, SEEK_CUR);
    }

    frame_data_pos += fh.frame_size + frame_header_size;
  }

  return 0;
}


/////////////////////////////////////////////////////////////////////////////
//// Media file OGG/Vorbis
/////////////////////////////////////////////////////////////////////////////

#define OGG_BLOCK_SIZE   27
#define OGG_PACKET_RSVSIZE 200*1024
#define OGG_PACKET_MAXSIZE 500*1024

FilePicReader::Picture * FilePicReader::ExtractOGGSPicture(const std::string& filePath, PictureType pictureType, bool& error)
{
  unsigned char buf[OGG_BLOCK_SIZE];
  unsigned char lacing[255];
  bool isLast = false;
  packet_t packet = { nullptr, 0, nullptr, 0 };
  Picture * pic = nullptr;
  FILE * file = fopen(filePath.c_str(), "rb");
  if (!file)
  {
    DBG(DBG_INFO, "%s: file not found (%s)\n", __FUNCTION__, filePath.c_str());
    error = true;
    return pic;
  }

  for (;;)
  {
    // check the magic file header, else close and return a null payload
    if (fread(buf, 1, OGG_BLOCK_SIZE, file) != OGG_BLOCK_SIZE || memcmp(buf, "OggS", 4) != 0)
    {
      DBG(DBG_INFO, "%s: bad magic header (%s)\n", __FUNCTION__, filePath.c_str());
      break;
    }
    //char stream_structure_version = read8(buf + 4);
    unsigned char header_type_flag = (unsigned char)read8(buf + 5);
    unsigned char number_page_segments = (unsigned char)read8(buf + 26);

    uint32_t segment_table = 0;
    if (fread(lacing, 1, number_page_segments, file) != number_page_segments)
    {
      DBG(DBG_INFO, "%s: file read error (%s)\n", __FUNCTION__, filePath.c_str());
      break;
    }

    for (int i = 0; i < number_page_segments; ++i)
      segment_table += (unsigned char)read8(lacing + i);

    // bit 0x04: this is the last page of a logical bitstream (eos)
    if ((header_type_flag & 0x04) == 0x04)
    {
      // append data and process the packet
      isLast = true;
      resize_packet(&packet, packet.datalen + segment_table);
      if (!fill_packet(&packet,segment_table, file))
      {
        DBG(DBG_INFO, "%s: file read error (%s)\n", __FUNCTION__, filePath.c_str());
        break;
      }
    }
    // bit 0x02: this is the first page of a logical bitstream (bos)
    else if ((header_type_flag & 0x02) == 0x02)
    {
      // fill fresh data and read next page
      packet.datalen = 0;
      resize_packet(&packet, OGG_PACKET_RSVSIZE);
      if (!fill_packet(&packet, segment_table, file))
      {
        DBG(DBG_INFO, "%s: file read error (%s)\n", __FUNCTION__, filePath.c_str());
        break;
      }
      continue;
    }
    // bit 0x01: page contains data of a packet continued from the previous page
    else if ((header_type_flag & 0x01) == 0x01)
    {
      // append data and read next page
      resize_packet(&packet, packet.datalen + segment_table);
      if (!fill_packet(&packet,segment_table, file))
      {
        DBG(DBG_INFO, "%s: file read error (%s)\n", __FUNCTION__, filePath.c_str());
        break;
      }
      continue;
    }

    /////////////////////////////////////////////////////////////////////////////
    //// Process the packet
    /////////////////////////////////////////////////////////////////////////////

    if (packet.datalen == 0)
    {
      DBG(DBG_INFO, "%s: missing BOS packet (%s)\n", __FUNCTION__, filePath.c_str());
      break;
    }

    // check for vorbis comment header
    unsigned char block = (unsigned char)read8(packet.data);
    if (block == 0x03 && packet.datalen > 7 &&
        memcmp(packet.data + 1, "vorbis", 6) == 0)
    {
      // parse comment header
      parse_comment(&packet, &pic, pictureType);
      break;
    }

    if (isLast)
      break; // finish

    // fill fresh data and read next page
    packet.datalen = 0;
    if (!fill_packet(&packet, segment_table, file))
    {
      DBG(DBG_INFO, "%s: file read error (%s)\n", __FUNCTION__, filePath.c_str());
      break;
    }
  }

  if (packet.buf != nullptr)
    delete [] packet.buf;
  fclose(file);
  error = (!isLast && pic == nullptr);
  return pic;
}

void FilePicReader::FreeOGGSPicture(void *payload)
{
  assert(payload);
  char * pic = static_cast<char*>(payload);
  delete [] pic;
}

bool FilePicReader::resize_packet(packet_t * packet, uint32_t size)
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

bool FilePicReader::fill_packet(packet_t * packet, uint32_t len, FILE * fp)
{
  if (!resize_packet(packet, packet->datalen + len) ||
      fread(packet->buf + packet->datalen, 1, len, fp) != len)
    return false;
  packet->data = packet->buf;
  packet->datalen += len;
  return true;
}

bool FilePicReader::parse_comment(packet_t * packet, Picture ** pic, PictureType pictureType)
{
  bool gotoLast = false;
  unsigned char * ve = packet->data + packet->datalen;
  unsigned char * vp = packet->data + 7; // pass magic string
  vp += read32le(vp) + 4; // pass vendor string
  int count = read32le(vp); // comment list length;
  vp += 4;
  while (count > 0)
  {
    int len = read32le(vp);
    vp += 4;
    if ((vp + len) > ve)
      break; // buffer overflow
    if (gotoLast)
      continue; // bypass to last
    if (len > 23 && memcmp(vp, "METADATA_BLOCK_PICTURE=", 23) == 0)
    {
      char * picbuf = nullptr;
      size_t lenbuf = Base64::b64decode(vp + 23, len - 23, &picbuf);
      // check picture type matches with requirement
      if (lenbuf > 8 && (read32be(picbuf) == pictureType || pictureType == -1))
      {
        unsigned mime_type_len;
        unsigned desc_len;
        unsigned data_len;
        // check for sanity
        if ((mime_type_len = read32be(picbuf + 4)) > lenbuf - 8 ||
                (desc_len = read32be(picbuf + 8 + mime_type_len)) > lenbuf - 12 - mime_type_len ||
                (data_len = read32be(picbuf + mime_type_len + desc_len + 28)) > lenbuf - 32 - desc_len - mime_type_len)
        {
          delete [] picbuf;
          break;
        }
        Picture * p = new Picture();
        p->payload = picbuf; // allocated pointer
        p->free = &FilePicReader::FreeOGGSPicture; // handler to free the payload
        p->mime = picbuf + 8; // mime type string
        picbuf[mime_type_len + 8] = 0; // terminate the mime type string with zero
        p->data = picbuf + mime_type_len + desc_len + 32; // image data
        p->size = data_len; // image data length
        DBG(DBG_PROTO, "%s: found picture (%s) size (%u)\n", __FUNCTION__, p->mime, p->size);
        *pic = p;
        gotoLast = true;
      }
    }
    vp += len;
    --count;
  }
  packet->data = vp + *vp;
  packet->datalen -= ve - vp - *vp;
  return (count == 0);
}
