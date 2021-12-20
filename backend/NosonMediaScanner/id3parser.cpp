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
 *  Strongly inspired by the source code of the parser from lightmediascanner.
 *  See https://github.com/profusion/lightmediascanner
 */
#include "id3parser.h"
#include "id3v1genres.h"
#include "mediafile.h"
#include "mediainfo.h"
#include "byteorder.h"
#include "packed.h"

#include <QDebug>
#include <cstdio>
#include <cassert>

#define ID3V2_HEADER_SIZE 10
#define ID3V2_FOOTER_SIZE 10

using namespace mediascanner;

static const char * _exts[] = {
  "MP3", "AAC",
#ifdef ENABLE_MEDIA_MP2
  "MP2",
#endif
};
static int _exts_len = sizeof(_exts) / sizeof(const char*);

bool ID3Parser::match(const QFileInfo& fileInfo)
{
  QString path = fileInfo.suffix().toUpper();
  for (int i = 0; i < _exts_len; ++i)
  {
    if (path == _exts[i])
      return true;
  }
  return false;
}

typedef QByteArray(*cs_conv_t)(const char * data, unsigned size);

static const unsigned id3_num_encodings = 5;

enum ID3Encoding
{
  Latin1 = 0,
  UTF16 = 1,
  UTF16BE = 2,
  UTF8 = 3,
  UTF16LE = 4,
};

struct ID3Iinfo
{
  QByteArray title;
  QByteArray album;
  QByteArray genre;
  QByteArray artist;
  QByteArray composer;
  int artist_priority;
  int track_no;
  bool has_art;
};

struct ID3v2FrameHeader
{
  char frame_id[4];
  unsigned int frame_size;
  int compression;
  int data_length_indicator;
};

PACK (
struct ID3v1Tag
{
  char title[30];
  char artist[30];
  char album[30];
  char year[4];
  char comments[30];
  unsigned char genre;
});

static QByteArray _cs_conv_latin1(const char * data, unsigned size);
static QByteArray _cs_conv_utf16(const char * data, unsigned size);
static QByteArray _cs_conv_utf16be(const char * data, unsigned size);
static QByteArray _cs_conv_utf8(const char * data, unsigned size);
static QByteArray _cs_conv_utf16le(const char * data, unsigned size);
static long _find_id3v2(FILE * fp, off_t * sync_offset);
static int _parse_id3v2(FILE * fp, long id3v2_offset, ID3Iinfo * info, off_t * ptag_size);
static int _parse_id3v1(FILE * fp, ID3Iinfo * info, cs_conv_t csconv);
static int _parse_mpeg_header(FILE * fp, off_t off, MediaInfo * audio_info, size_t size);

bool ID3Parser::parse(MediaFile * file, MediaInfo * info, bool debug)
{
  ID3Iinfo id3info;
  id3info.track_no = -1;
  id3info.artist_priority = 0;
  id3info.has_art = false;

  int r = 0;
  long id3v2_offset;
  off_t sync_offset = 0;

  std::string path(file->filePath.toUtf8().constData());
  FILE * fp = fopen(path.c_str(), "rb");
  if (!fp)
    return false;

  id3v2_offset = _find_id3v2(fp, &sync_offset);
  if (id3v2_offset >= 0)
  {
    off_t id3v2_size = 3;

    sync_offset = id3v2_offset;

    if (_parse_id3v2(fp, id3v2_offset, &id3info, &id3v2_size) != 0 ||
            id3info.title.isEmpty() ||
            id3info.artist.isEmpty() ||
            id3info.album.isEmpty() ||
            id3info.genre.isEmpty() ||
            id3info.track_no == -1)
    {
      id3v2_offset = -1;
    }

    /* Even if we later failed to parse the ID3, we want to look for sync
     * frame only after the tag */
    sync_offset += id3v2_size;
  }

  if (id3v2_offset < 0)
  {
    char tag[3];
    /* check for id3v1 tag */
    if (fseek(fp, -128, SEEK_END) != 0)
    {
      r = -3;
      goto done;
    }

    if (fread(&tag, 1, 3, fp) != 3)
    {
      r = -4;
      goto done;
    }

    if (memcmp(tag, "TAG", 3) == 0)
    {
      if (_parse_id3v1(fp, &id3info, _cs_conv_utf8) != 0)
      {
        r = -5;
        goto done;
      }
    }
  }

  info->container = file->suffix.toLower();
  info->title = id3info.title.isEmpty() ? file->baseName : id3info.title;
  info->album = id3info.album;
  info->genre = id3info.genre;
  info->artist = id3info.artist;
  info->trackNo = id3info.track_no > 0 ? id3info.track_no : 0;
  info->hasArt = id3info.has_art;

  r = _parse_mpeg_header(fp, sync_offset, info, file->size);

done:
  fclose(fp);
  return (r == 0);
}

static QByteArray _cs_conv_latin1(const char * data, unsigned size)
{
  return QString::fromLatin1(QByteArray(data, size)).toUtf8();
}

static QByteArray _cs_conv_utf16(const char * data, unsigned size)
{
  return QString::fromUtf16((const char16_t*) data, size / 2).toUtf8();
}

static QByteArray _cs_conv_utf16be(const char * data, unsigned size)
{
  static const char * _BOM = "\xfe\xff";
  if (memcmp(data, _BOM, 2) == 0)
    return _cs_conv_utf16(data, size);
  QByteArray buf;
  buf.append(_BOM).append(data, size);
  return _cs_conv_utf16(buf.constData(), buf.length());
}

static QByteArray _cs_conv_utf8(const char * data, unsigned size)
{
  return QByteArray(data, size);
}

static QByteArray _cs_conv_utf16le(const char * data, unsigned size)
{
  static const char * _BOM = "\xff\xfe";
  if (memcmp(data, _BOM, 2) == 0)
    return _cs_conv_utf16(data, size);
  QByteArray buf;
  buf.append(_BOM).append(data, size);
  return _cs_conv_utf16(buf.constData(), buf.length());
}

static cs_conv_t _cs_conv[] = {_cs_conv_latin1, _cs_conv_utf16, _cs_conv_utf16be, _cs_conv_utf8, _cs_conv_utf16le};

static unsigned int _to_uint(const char * data, int data_size)
{
  unsigned int sum = 0;
  unsigned int last, i;

  last = data_size > 4 ? 3 : data_size - 1;

  for (i = 0; i <= last; i++)
    sum |= ((unsigned char) data[i]) << ((last - i) * 8);

  return sum;
}

static unsigned int _to_uint_max7b(const char * data, int data_size)
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

/**
 * Returns the offset in fd to the position after the ID3 tag
 */
static long _find_id3v2(FILE * fp, off_t * sync_offset)
{
  static const char pattern[3] = {'I', 'D', '3'};
  char buffer[3];
  unsigned int prev_part_match, prev_part_match_sync = 0;
  long buffer_offset;

  if (fread(buffer, 1, sizeof(buffer), fp) != sizeof(buffer))
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

    if (fread(buffer, 1, sizeof(buffer), fp) != sizeof(buffer))
      return -1;
    buffer_offset += sizeof(buffer);
  }
}

static unsigned int _get_id3v2_frame_header_size(unsigned int version)
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

static void _parse_id3v2_frame_header(char * data, unsigned int version, struct ID3v2FrameHeader * fh)
{
  switch (version)
  {
  case 0:
  case 1:
  case 2:
    memcpy(fh->frame_id, data, 3);
    fh->frame_id[3] = 0;
    fh->frame_size = _to_uint(data + 3, 3);
    fh->compression = 0;
    fh->data_length_indicator = 0;
    break;
  case 3:
    memcpy(fh->frame_id, data, 4);
    fh->frame_size = _to_uint(data + 4, 4);
    fh->compression = data[9] & 0x40;
    fh->data_length_indicator = 0;
    break;
  case 4:
  default:
    memcpy(fh->frame_id, data, 4);
    fh->frame_size = _to_uint(data + 4, 4);
    fh->compression = data[9] & 0x4;
    fh->data_length_indicator = data[9] & 0x1;
    break;
  }
}

static inline void _get_id3v2_frame_info(const char * frame_data, unsigned int frame_size, QByteArray * str, cs_conv_t csconv, int strip)
{
  if (frame_size == 0)
    return;
  if (strip)
    *str = csconv(frame_data, frame_size).trimmed();
  else
    *str = csconv(frame_data, frame_size);
}

static int _get_id3v2_artist(unsigned int index, const char * frame_data, unsigned int frame_size, ID3Iinfo * info, cs_conv_t csconv)
{
  static const unsigned char artist_priorities[] = {3, 4, 2, 1};
  const unsigned int index_max = sizeof(artist_priorities) / sizeof(*artist_priorities);

  if (index >= index_max)
    return -1;

  if (artist_priorities[index] > info->artist_priority)
  {
    QByteArray artist;
    _get_id3v2_frame_info(frame_data, frame_size, &artist, csconv, 1);
    if (!artist.isEmpty())
    {
      info->artist = artist;
      info->artist_priority = artist_priorities[index];
    }
  }
  return 0;
}

static int _get_id3v1_genre(unsigned int genre, QByteArray * out)
{
  if (genre < id3v1_genres_len)
  {
    out->clear();
    out->append(id3v1_genres_str[genre]);
    return 0;
  }
  return -1;
}

static inline int _parse_id3v1_genre(const char * str_genre, QByteArray * out)
{
  return _get_id3v1_genre(atoi(str_genre), out);
}

static void _get_id3v2_genre(const char * frame_data, unsigned int frame_size, QByteArray * out, cs_conv_t csconv)
{
  unsigned int i, is_number;
  QByteArray genre;

  _get_id3v2_frame_info(frame_data, frame_size, &genre, csconv, 1);
  if (genre.isEmpty())
    return;

  is_number = (genre.length() != 0 && genre.at(0) != '(');
  if (is_number)
  {
    for (i = 0; i < genre.length(); ++i)
    {
      if (!isdigit(genre.at(i)))
      {
        is_number = 0;
        break;
      }
    }
  }

  if (is_number && _parse_id3v1_genre(genre.constData(), out) == 0)
  {
    /* id3v1 genre found */
    return;
  }

  /* ID3v2.3 "content type" can contain a ID3v1 genre number in parenthesis at
   * the beginning of the field. If this is all that the field contains, do a
   * translation from that number to the name and return that.  If there is a
   * string following the ID3v1 genre number, that is considered to be
   * authoritative and we return that instead. Or finally, the field may
   * simply be free text, in which case we just return the value. */

  if (genre.length() > 1 && genre.at(0) == '(')
  {
    int closing = 0;
    while (closing < genre.length() && genre.at(closing) != ')') ++closing;

    if (closing < genre.length())
    {
      if (closing == genre.length() - 1)
      {
        /* ) is the last character and only appears once in the
         * string get the id3v1 genre enclosed by parentheses
         */
        if (_parse_id3v1_genre(genre.constData() + 1, out) == 0)
        {
          return;
        }
      }
      else
      {
        ++closing;
        out->clear();
        out->append(genre.constData() + closing, genre.length() - closing);
        return;
      }
    }
  }

  /* pure text */
  *out = genre;
}

static void _get_id3v2_trackno(const char * frame_data, unsigned int frame_size, ID3Iinfo * info, cs_conv_t csconv)
{
  QByteArray buf;
  _get_id3v2_frame_info(frame_data, frame_size, &buf, csconv, 0);
  if (buf.isEmpty())
    return;
  info->track_no = atoi(buf.constData());
}

static void _parse_id3v2_frame(struct ID3v2FrameHeader * fh, const char * frame_data, ID3Iinfo * info)
{
  cs_conv_t csconv;
  unsigned int text_encoding, frame_size;
  const char *fid;

  /* ignore frames which contains just the encoding */
  if (fh->frame_size <= 1)
    return;

  /* All used frames start with 'T' */
  fid = fh->frame_id;
  if (fid[0] != 'T')
    return;

  text_encoding = frame_data[0];

  /* skip first byte - text encoding */
  frame_data += 1;
  frame_size = fh->frame_size - 1;

  if (text_encoding < id3_num_encodings)
    csconv = _cs_conv[text_encoding];
  else
    csconv = _cs_conv[0];

  /* ID3v2.2 used 3 bytes for the frame id, so let's check it */
  if ((fid[1] == 'T' && fid[2] == '2') ||
          (fid[1] == 'I' && fid[2] == 'T' && fid[3] == '2'))
    _get_id3v2_frame_info(frame_data, frame_size, &info->title, csconv, 1);
  else if (fid[1] == 'P')
  {
    if (fid[2] == 'E')
      _get_id3v2_artist(fid[3] - '1', frame_data, frame_size, info, csconv);
    else if (fid[2] >= '1' && fid[2] <= '4')
      _get_id3v2_artist(fid[2] - '1', frame_data, frame_size, info, csconv);
  }
    /* TALB, TAL */
  else if (fid[1] == 'A' && fid[2] == 'L')
    _get_id3v2_frame_info(frame_data, frame_size, &info->album, csconv, 1);
    /* TCOM (Composer) */
  else if (fid[1] == 'C' && fid[2] == 'O' && fid[3] == 'M')
    _get_id3v2_frame_info(frame_data, frame_size, &info->composer, csconv, 1);
    /* TCON (Content/Genre) */
  else if (fid[1] == 'C' && fid[2] == 'O' && fid[3] == 'N')
    _get_id3v2_genre(frame_data, frame_size, &info->genre, csconv);
  else if (fid[1] == 'R' && (fid[2] == 'K' ||
          (fid[2] == 'C' && fid[3] == 'K')))
    _get_id3v2_trackno(frame_data, frame_size, info, csconv);
}

static int _parse_id3v2(FILE * fp, long id3v2_offset, ID3Iinfo * info, off_t * ptag_size)
{
  char header_data[10], frame_header_data[10];
  unsigned int tag_size, major_version, frame_data_pos, frame_data_length, frame_header_size;
  int extended_header, footer_present;
  struct ID3v2FrameHeader fh;
  size_t nread;

  fseek(fp, id3v2_offset, SEEK_SET);

  /* parse header */
  if (fread(header_data, 1, ID3V2_HEADER_SIZE, fp) != ID3V2_HEADER_SIZE)
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

    if (fread(extended_header_data, 1, 4, fp) != 4)
      return -1;

    extended_header_size = _to_uint(extended_header_data, 4);
    crc = extended_header_data[5] & 0x8000;

    *ptag_size += extended_header_size + (crc * 4);

    fseek(fp, extended_header_size - 6, SEEK_CUR);
    frame_data_pos += extended_header_size;
    frame_data_length -= extended_header_size;
  }

  footer_present = header_data[5] & 0x8; /* bit 4 */
  if (footer_present && frame_data_length > ID3V2_FOOTER_SIZE)
    frame_data_length -= ID3V2_FOOTER_SIZE;

  frame_header_size = _get_id3v2_frame_header_size(major_version);
  while (frame_data_pos < frame_data_length - frame_header_size)
  {
    nread = fread(frame_header_data, 1, frame_header_size, fp);
    if (nread == 0)
      break;

    if (nread != frame_header_size)
      return -1;

    if (frame_header_data[0] == 0)
      break;

    _parse_id3v2_frame_header(frame_header_data, major_version, &fh);

    if (fh.frame_size > 0 &&
            !fh.compression &&
            fh.frame_id[0] == 'T' &&
            memcmp(fh.frame_id, "TXXX", 4) != 0)
    {
      char *frame_data;

      if (fh.data_length_indicator)
        fseek(fp, 4, SEEK_CUR);

      frame_data = (char*) malloc(sizeof(char) * fh.frame_size);
      if (fread(frame_data, 1, fh.frame_size, fp) != (int) fh.frame_size)
      {
        free(frame_data);
        return -1;
      }

      _parse_id3v2_frame(&fh, frame_data, info);
      free(frame_data);
    }
    else
    {
      if (major_version == 0x2 && memcmp(fh.frame_id, "PIC", 3) == 0)
        info->has_art = true;
      else if (major_version > 0x2 && major_version < 0x5 && memcmp(fh.frame_id, "APIC", 4) == 0)
        info->has_art = true;

      if (fh.data_length_indicator)
        fseek(fp, fh.frame_size + 4, SEEK_CUR);
      else
        fseek(fp, fh.frame_size, SEEK_CUR);
    }

    frame_data_pos += fh.frame_size + frame_header_size;
  }

  return 0;
}

static inline void _id3v1_str_get(QByteArray * str, const char * buf, int maxlen, cs_conv_t csconv)
{
  int start, len;
  const char *p, *p_end, *p_last;

  start = 0;
  p_last = NULL;
  p_end = buf + maxlen;
  for (p = buf; *p != '\0' && p < p_end; p++)
  {
    if (!isspace(*p))
      p_last = p;
    else if (!p_last)
      start++;
  }

  if (!p_last)
    return;

  len = (p_last - buf) - start;
  if (len < 1)
    return;

  ++len; /* p_last is not included yet */
  *str = csconv(buf + start, len);
}

static int _parse_id3v1(FILE * fp, ID3Iinfo * info, cs_conv_t csconv)
{
  struct ID3v1Tag tag;
  if (fread(&tag, 1, sizeof(struct ID3v1Tag), fp) != sizeof(struct ID3v1Tag))
    return -1;

  if (info->title.isEmpty())
    _id3v1_str_get(&info->title, tag.title, sizeof(tag.title), csconv);
  if (info->artist.isEmpty())
    _id3v1_str_get(&info->artist, tag.artist, sizeof(tag.artist), csconv);
  if (info->album.isEmpty())
    _id3v1_str_get(&info->album, tag.album, sizeof(tag.album), csconv);
  if (info->genre.isEmpty())
    _get_id3v1_genre(tag.genre, &info->genre);
  if (info->track_no == -1 &&
          tag.comments[28] == '\0' && tag.comments[29] != '\0')
    info->track_no = (unsigned char) tag.comments[29];

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
////
//// MPEG Audio info
////

#define MPEG_HEADER_SIZE 4
#define N_FRAMES_BITRATE_ESTIMATE 512
#define N_FRAMES_CBR_ESTIMATE 128

enum mpeg_audio_version
{
  MPEG_AUDIO_VERSION_1 = 0,
  MPEG_AUDIO_VERSION_2,
  MPEG_AUDIO_VERSION_2_5,
  MPEG_AUDIO_VERSION_4,
  MPEG_AUDIO_VERSION_COUNT
};

enum mpeg_audio_layer
{
  MPEG_AUDIO_LAYER_1 = 0,
  MPEG_AUDIO_LAYER_2,
  MPEG_AUDIO_LAYER_3,
  MPEG_AUDIO_LAYER_AAC,
  MPEG_AUDIO_LAYER_COUNT
};

struct mpeg_header
{
  enum mpeg_audio_version version;
  enum mpeg_audio_layer layer;
  bool crc;
  bool padding;
  uint8_t channels;
  uint8_t sampling_rate_idx;
  uint8_t codec_idx;
  unsigned int bitrate_idx;
  unsigned int bitrate;
  unsigned int length;
  bool cbr;
};

static const char * _codecs[] = {
  /* mp3 */
  "mpeg1layer1",
  "mpeg1layer2",
  "mpeg1layer3",
  "mpeg2layer1",
  "mpeg2layer2",
  "mpeg2layer3",
  "mpeg2.5layer1",
  "mpeg2.5layer2",
  "mpeg2.5layer3",

  /* aac */
#define MPEG_CODEC_AAC_START 9
  "mpeg2aac",  // mpeg2aac-main
  "mpeg2aac",  // mpeg2aac-lc
  "mpeg2aac",  // mpeg2aac-ssr
  "mpeg2aac",  // mpeg2aac-ltp
  "mpeg4aac",  // mpeg4aac-main
  "mpeg4aac",  // mpeg4aac-lc
  "mpeg4aac",  // mpeg4aac-ssr
  "mpeg4aac",  // mpeg4aac-ltp
  ""
};

/* Ordered according to AAC index, take care with mp3 */
static int _sample_rates[16] = {
  96000, 88200, 64000,

  /* Frequencies available on mp3, */
  48000, 44100, 32000,
  24000, 22050, 16000,
  12000, 11025, 8000,

  7350, /* reserved, zeroed */
};

static unsigned int
_bitrate_table[MPEG_AUDIO_VERSION_COUNT][MPEG_AUDIO_LAYER_COUNT][16] = {
  /*MPEG_AUDIO_VERSION_1*/
  0, 32, 64, 96, 128, 160, 192, 224, 256, 288, 320, 352, 384, 416, 448, 0,
  0, 32, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 384, 0,
  0, 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  /*MPEG_AUDIO_VERSION_2*/
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 32, 48, 56, 64, 80, 96, 112, 128, 144, 160, 176, 192, 224, 256, 0,
  0, 8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  /*MPEG_AUDIO_VERSION_2_5*/
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 32, 48, 56, 64, 80, 96, 112, 128, 144, 160, 176, 192, 224, 256, 0,
  0, 8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  /*MPEG_AUDIO_VERSION_4*/
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

static unsigned int
_samples_per_frame_table[MPEG_AUDIO_VERSION_COUNT][MPEG_AUDIO_LAYER_COUNT] = {
  384, 1152, 1152, 0,
  384, 1152, 576, 0,
  384, 1152, 576, 0,
  0, 0, 0, 0,
};

static int _fill_mpeg_header(struct mpeg_header * hdr, const uint8_t b[4]);
static int _fill_aac_header(struct mpeg_header * hdr, const uint8_t b[4]);
static int _fill_mp3_header(struct mpeg_header *hdr, const uint8_t b[4]);
static int _parse_vbr_headers(FILE * fp, off_t mpeg_offset, struct mpeg_header * hdr);
static int _estimate_mp3_bitrate_from_frames(FILE * fp, off_t mpeg_offset, struct mpeg_header * orig_hdr);

static int _parse_mpeg_header(FILE * fp, off_t off, MediaInfo * audio_info, size_t size)
{
  uint8_t buffer[32];
  const uint8_t *p, *p_end;
  unsigned int prev_read;
  struct mpeg_header hdr = {};
  int r;

  fseek(fp, off, SEEK_SET);

  /* Find sync word */
  prev_read = 0;
  do
  {
    int nread = fread(buffer + prev_read, 1, sizeof(buffer) - prev_read, fp);
    if (nread < MPEG_HEADER_SIZE)
      return -1;

    p = buffer;
    p_end = buffer + nread;
    off += nread - prev_read;
    while (p < p_end && (p = (const uint8_t*) memchr(p, 0xff, p_end - p)))
    {
      /* poor man's ring buffer since the needle is small (4 bytes) */
      if (p > p_end - MPEG_HEADER_SIZE)
      {
        memcpy(buffer, p, p_end - p);
        break;
      }

      if (_is_id3v2_second_synch_byte(*(p + 1)))
      {
        off -= (p_end - p);
        goto found;
      }

      p++;
    }
    prev_read = p ? p_end - p : 0;
  }
  while (1);

found:
  if (_fill_mpeg_header(&hdr, p) < 0)
  {
    qWarning("%s: Invalid field in file, ignoring.", __FUNCTION__);
    return 0;
  }

  if (hdr.layer == MPEG_AUDIO_LAYER_AAC)
    r = _fill_aac_header(&hdr, p);
  else
  {
    if ((r = _fill_mp3_header(&hdr, p) < 0) ||
            (r = _parse_vbr_headers(fp, off, &hdr) < 0))
      return r;

    if (hdr.cbr)
      hdr.bitrate = _bitrate_table[hdr.version][hdr.layer][hdr.bitrate_idx] * 1000;
    else if (!hdr.bitrate)
    {
      r = _estimate_mp3_bitrate_from_frames(fp, off, &hdr);
      if (r < 0)
        return r;
    }

    if (!hdr.length && hdr.bitrate > 0)
      hdr.length = (8 * (size - off)) / hdr.bitrate;
  }

  if (r < 0)
    return r;

  audio_info->codec.append(_codecs[hdr.codec_idx]);
  audio_info->sampleRate = _sample_rates[hdr.sampling_rate_idx];
  audio_info->channels = hdr.channels;
  audio_info->bitRate = hdr.bitrate;
  audio_info->duration = hdr.length;

  return 0;
}

static int _fill_mpeg_header(struct mpeg_header * hdr, const uint8_t b[4])
{
  unsigned int version = (b[1] & 0x18) >> 3;
  unsigned int layer = (b[1] & 0x06) >> 1;

  switch (layer)
  {
  case 0x0:
    if (version == 0x2 || version == 0x3)
      hdr->layer = MPEG_AUDIO_LAYER_AAC;
    else
      return -1;
    break;
  case 0x1:
    hdr->layer = MPEG_AUDIO_LAYER_3;
    break;
  case 0x2:
    hdr->layer = MPEG_AUDIO_LAYER_2;
    break;
  case 0x3:
    hdr->layer = MPEG_AUDIO_LAYER_1;
    break;
  }

  switch (version)
  {
  case 0x0:
    hdr->version = MPEG_AUDIO_VERSION_2_5;
    break;
  case 0x1:
    return -1;
  case 0x2:
    if (layer == 0x0)
      hdr->version = MPEG_AUDIO_VERSION_4;
    else
      hdr->version = MPEG_AUDIO_VERSION_2;
    break;
  case 0x3:
    if (layer == 0x0)
      hdr->version = MPEG_AUDIO_VERSION_2;
    else
      hdr->version = MPEG_AUDIO_VERSION_1;
  }

  hdr->crc = !(b[1] & 0x1);
  hdr->padding = b[2] & 0x2;

  return 0;
}

static int _fill_aac_header(struct mpeg_header * hdr, const uint8_t b[4])
{
  unsigned int profile;

  hdr->sampling_rate_idx = (b[2] & 0x3c) >> 2;

  profile = (b[2] & 0xc0) >> 6;
  hdr->codec_idx = MPEG_CODEC_AAC_START + profile;
  if (hdr->version == MPEG_AUDIO_VERSION_4)
    hdr->codec_idx += 4;

  hdr->channels = ((b[2] & 0x1) << 2) | ((b[3] & 0xc0) >> 6);
  return 0;
}

static inline int _fill_mp3_header(struct mpeg_header * hdr, const uint8_t b[4])
{
  unsigned int bitrate_idx = (b[2] & 0xf0) >> 4;

  hdr->sampling_rate_idx = (b[2] & 0x0c) >> 2;

  if (hdr->sampling_rate_idx == 0x3)
    return -1;
  /*
   * Sampling rate frequency index
   * bits     MPEG1           MPEG2           MPEG2.5
   * 00       44100 Hz        22050 Hz        11025 Hz
   * 01       48000 Hz        24000 Hz        12000 Hz
   * 10       32000 Hz        16000 Hz        8000 Hz
   * 11       reserv.         reserv.         reserv.
   */

  /* swap 0x1 and 0x0 */
  if (hdr->sampling_rate_idx < 0x2)
    hdr->sampling_rate_idx = !hdr->sampling_rate_idx;
  hdr->sampling_rate_idx += 3 * hdr->version + 3;

  hdr->codec_idx = hdr->version * 3 + hdr->layer;

  hdr->channels = (b[3] & 0xc0) >> 6;
  hdr->channels = hdr->channels == 0x3 ? 1 : 2;

  hdr->bitrate_idx = bitrate_idx;

  return 0;
}

static int _parse_vbr_headers(FILE * fp, off_t mpeg_offset, struct mpeg_header * hdr)
{
  unsigned int sampling_rate, samples_per_frame, flags, nframes = 0, size = 0;
  int xing_offset_table[2][2] = {/* [(version == 1)][channels == 1)] */
    { 17, 9},
    { 32, 17}
  };
  uint8_t buf[18];
  off_t xing_offset;

  /* Try Xing first since it's the most likely to be there */
  xing_offset = mpeg_offset + 4 + 2 * hdr->crc
          + xing_offset_table[(hdr->version == 1)][(hdr->channels == 1)];

  fseek(fp, xing_offset, SEEK_SET);
  if (fread(buf, 1, sizeof(buf), fp) != sizeof(buf))
    return -1;

  hdr->cbr = (memcmp(buf, "Info", 4) == 0);
  if (hdr->cbr || memcmp(buf, "Xing", 4) == 0)
  {
    flags = buf[7];

    if (flags & 1)
      nframes = read32be(&buf[8]);
    if (flags & 2)
      size = read32be(&buf[8 + !!(flags & 1) * 4]);

    goto proceed;
  }

  /* VBRI is found in files encoded by Fraunhofer Encoder. Fixed location: 32
   * bytes after the mpeg header */
  fseek(fp, mpeg_offset + 36, SEEK_SET);
  if (fread(buf, 1, sizeof(buf), fp) != sizeof(buf))
    return -1;

  if (memcmp(buf, "VBRI", 4) == 0 && read16be(buf) == 1)
  {
    size = read32be(&buf[10]);
    nframes = read32be(&buf[14]);

    goto proceed;
  }

  return 0;

proceed:
  samples_per_frame = _samples_per_frame_table[hdr->version][hdr->layer];
  sampling_rate = _sample_rates[hdr->sampling_rate_idx];

  assert(sampling_rate != 0);

  hdr->length = (nframes * samples_per_frame) / sampling_rate;

  if (hdr->length)
    hdr->bitrate = (8 * size) / (1000 * hdr->length);

  return 0;
}

static int _estimate_mp3_bitrate_from_frames(FILE * fp, off_t mpeg_offset, struct mpeg_header * orig_hdr)
{
  struct mpeg_header hdr = *orig_hdr;
  off_t offset = mpeg_offset;
  unsigned int sum = 0, i;
  int r;
  bool cbr = true;
  /* For Layer I slot is 32 bits long, for Layer II and Layer III slot is 8
   * bits long.
   * [layer == 1] */
  unsigned int padding_size_table[2] = {1, 4};
  unsigned int samples_per_frame, sampling_rate;

  samples_per_frame = _samples_per_frame_table[hdr.version][hdr.layer];
  sampling_rate = _sample_rates[hdr.sampling_rate_idx];
  assert(sampling_rate != 0);

  for (i = 0; i < N_FRAMES_BITRATE_ESTIMATE;)
  {
    unsigned int bitrate, padding_size;
    unsigned int framesize;
    uint8_t buf[4];

    bitrate = _bitrate_table[hdr.version][hdr.layer][hdr.bitrate_idx];
    if (cbr && bitrate == hdr.bitrate && i > N_FRAMES_CBR_ESTIMATE)
    {
      i = 1;
      sum = bitrate;
      break;
    }

    sum += bitrate;
    i++;

    padding_size = hdr.padding ? padding_size_table[hdr.layer == 1] : 0;

    framesize = 4; /* mpeg header */
    framesize += (samples_per_frame / 8) * bitrate * 1000;
    framesize /= sampling_rate;
    framesize += (padding_size * samples_per_frame);

    offset += framesize;

    fseek(fp, offset, SEEK_SET);
    r = fread(buf, 1, sizeof(buf), fp);

    if (r < 0)
    {
      qWarning("%s: ERROR reading frame header at %#x", __FUNCTION__, (unsigned int) offset);
      break;
    }
    if (!r)
      break;

    if (buf[0] != 0xff || !_is_id3v2_second_synch_byte(buf[1]) ||
            _fill_mpeg_header(&hdr, buf) < 0 ||
            _fill_mp3_header(&hdr, buf) < 0)
    {
      break;
    }
  }

  orig_hdr->bitrate = sum / i * 1000;

  return 0;
}
