/*
 *      Copyright (C) 2016 Jean-Luc Barriere
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published
 *  by the Free Software Foundation; either version 3, or (at your option)
 *  any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this library; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 51 Franklin Street, Fifth Floor, Boston,
 *  MA 02110-1301 USA
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#include "compressor.h"
#include "cppdef.h"

#if HAVE_ZLIB
#include <zlib.h>
#else
#define Z_NO_FLUSH      0
#define Z_OK            0
#define Z_STREAM_END    1
#define Z_STREAM_ERROR (-2)
#define Z_BUF_ERROR    (-5)
#endif

#define GZIP_WINDOWS_BIT  15 + 16
#define GZIP_CHUNK_SIZE   16384
#define MIN(a,b)          (a > b ? b : a)
#define MAX(a,b)          (a > b ? a : b)

using namespace NSROOT;

Compressor::Compressor(const char *input, size_t len, int level /*= -1*/)
: m_status(Z_STREAM_ERROR)
, m_flush(Z_NO_FLUSH)
, m_stop(true)
, m_chunk_size(GZIP_CHUNK_SIZE)
, m_type_in(MEM_BUFFER)
, m_input_len(len)
, m_input(input)
, m_rstream(0)
, m_rstream_hdl(0)
, m_rstream_buf(0)
, m_output(0)
, m_output_pos(0)
, m_output_len(0)
, _opaque(0)
{
#if HAVE_ZLIB
  m_output = new char[m_chunk_size];
  _opaque = new z_stream;
  m_status = _init(_opaque, m_output, m_chunk_size, level);
  m_stop = (m_status != Z_OK);
#endif
}

Compressor::Compressor(STREAM_READER reader, void *handle, int level /*= -1*/)
: m_status(Z_STREAM_ERROR)
, m_flush(Z_NO_FLUSH)
, m_stop(true)
, m_chunk_size(GZIP_CHUNK_SIZE)
, m_type_in(FCB_READER)
, m_input_len(0)
, m_input(0)
, m_rstream(reader)
, m_rstream_hdl(handle)
, m_rstream_buf(0)
, m_output(0)
, m_output_pos(0)
, m_output_len(0)
, _opaque(0)
{
#if HAVE_ZLIB
  m_rstream_buf = new char[m_chunk_size];
  m_output = new char[m_chunk_size];
  _opaque = new z_stream;
  m_status = _init(_opaque, m_output, m_chunk_size, level);
  m_stop = (m_status != Z_OK);
#endif
}

Compressor::~Compressor()
{
#if HAVE_ZLIB
  z_stream *strm = static_cast<z_stream*>(_opaque);
  deflateEnd(strm);
  SAFE_DELETE(strm);
  SAFE_DELETE_ARRAY(m_output);
  SAFE_DELETE_ARRAY(m_rstream_buf);
#endif
}

bool Compressor::IsCompleted()
{
  return (m_status == Z_STREAM_END);
}

bool Compressor::HasBufferError()
{
  return (m_status == Z_BUF_ERROR);
}

bool Compressor::HasStreamError()
{
  switch(m_status)
  {
  case Z_OK:
  case Z_STREAM_END:
  case Z_BUF_ERROR:
    return false;
  default:
    return true;
  }
}

size_t Compressor::ReadOutput(char *buf, size_t len)
{
  size_t out = 0;
#if HAVE_ZLIB
  while (len)
  {
    if (m_output_len)
    {
      size_t sz = MIN(m_output_len, len);
      memcpy(buf, m_output + m_output_pos, sz);
      out += sz;
      buf += sz;
      len -= sz;
      m_output_pos += sz;
      m_output_len -= sz;
    }
    else if (m_status != Z_STREAM_END)
    {
      z_stream *strm = static_cast<z_stream*>(_opaque);
      if (!strm->avail_in)
        NextChunk();
      if (!strm->avail_out)
      {
        strm->next_out = (unsigned char*)m_output;
        strm->avail_out = m_chunk_size;
        m_output_pos = 0;
      }
      // Try to deflate chunk
      m_status = deflate(strm, m_flush);
      if (m_status < 0)
      {
        m_stop = true;
        return 0;
      }
      m_output_len = m_chunk_size - m_output_pos - strm->avail_out;
      m_stop = false;
    }
    else
    {
      m_stop = true;
      break;
    }
  }
#endif
  return out;
}

size_t Compressor::FetchOutput(const char **data)
{
  *data = 0;
#if HAVE_ZLIB
  size_t len = 0;
  while (!m_stop)
  {
    if (m_output_len)
    {
      *data = m_output + m_output_pos;
      len = m_output_len;
      m_output_pos += m_output_len;
      m_output_len = 0;
      return len;
    }
    else if (m_status != Z_STREAM_END)
    {
      z_stream *strm = static_cast<z_stream*>(_opaque);
      if (!strm->avail_in)
        NextChunk();
      if (!strm->avail_out)
      {
        strm->next_out = (unsigned char*)m_output;
        strm->avail_out = m_chunk_size;
        m_output_pos = 0;
      }
      // Try to inflate chunk
      m_status = deflate(strm, m_flush);
      if (m_status < 0)
      {
        m_stop = true;
        return 0;
      }
      m_output_len = m_chunk_size - m_output_pos - strm->avail_out;
      m_stop = false;
    }
    else
    {
      m_stop = true;
      break;
    }
  }
#endif
  return 0;
}

int Compressor::_init(void *zp, void *out, size_t len, int level)
{
#if HAVE_ZLIB
  z_stream *strm = static_cast<z_stream*>(zp);
  // Prepare inflater status
  strm->zalloc = Z_NULL;
  strm->zfree = Z_NULL;
  strm->opaque = Z_NULL;
  strm->avail_in = 0;
  strm->next_in = Z_NULL;
  strm->avail_out = len;
  strm->next_out = (unsigned char*)out;
  return deflateInit2(strm, MAX(-1, MIN(level, 9)), Z_DEFLATED, GZIP_WINDOWS_BIT, 8, Z_DEFAULT_STRATEGY);
#else
  return Z_STREAM_ERROR;
#endif
}

size_t Compressor::NextChunk()
{
  size_t sz = 0;
#if HAVE_ZLIB
  if (m_flush != Z_FINISH)
  {
    z_stream *strm = static_cast<z_stream*>(_opaque);
    switch(m_type_in)
    {
    case MEM_BUFFER:
      // Determine current chunk size
      sz = MIN(m_chunk_size, m_input_len);
      // Check for end of data
      if (sz)
      {
        // Set stream
        strm->next_in = (unsigned char*)m_input;
        strm->avail_in = (unsigned)sz;
        // Update next chunk
        m_input += sz;
        m_input_len -= sz;
        // flush on last chunk
        m_flush = (m_input_len > 0 ? Z_NO_FLUSH : Z_FINISH);
      }
      break;
    case FCB_READER:
      {
        int ret = (*m_rstream)(m_rstream_hdl, (void*)m_rstream_buf, m_chunk_size);
        if (ret < 0)
          sz = 0;
        else
        {
          m_flush = (ret > 0 ? Z_NO_FLUSH : Z_FINISH);
          sz = ret;
        }
        // Set stream
        strm->next_in = (unsigned char*)m_rstream_buf;
        strm->avail_in = (unsigned)sz;
      }
      break;
    default:
      break;
    }
  }
#endif
  return sz;
}

Decompressor::Decompressor(const char *input, size_t len)
: m_status(Z_STREAM_ERROR)
, m_stop(true)
, m_chunk_size(GZIP_CHUNK_SIZE)
, m_type_in(MEM_BUFFER)
, m_input_len(len)
, m_input(input)
, m_rstream(0)
, m_rstream_hdl(0)
, m_rstream_buf(0)
, m_output(0)
, m_output_pos(0)
, m_output_len(0)
, _opaque(0)
{
#if HAVE_ZLIB
  m_output = new char[m_chunk_size];
  _opaque = new z_stream;
  m_status = _init(_opaque, m_output, m_chunk_size);
  m_stop = (m_status != Z_OK);
#endif
}

Decompressor::Decompressor(STREAM_READER reader, void *handle)
: m_status(Z_STREAM_ERROR)
, m_stop(true)
, m_chunk_size(GZIP_CHUNK_SIZE)
, m_type_in(FCB_READER)
, m_input_len(0)
, m_input(0)
, m_rstream(reader)
, m_rstream_hdl(handle)
, m_rstream_buf(0)
, m_output(0)
, m_output_pos(0)
, m_output_len(0)
, _opaque(0)
{
#if HAVE_ZLIB
  m_rstream_buf = new char[m_chunk_size];
  m_output = new char[m_chunk_size];
  _opaque = new z_stream;
  m_status = _init(_opaque, m_output, m_chunk_size);
  m_stop = (m_status != Z_OK);
#endif
}

Decompressor::~Decompressor()
{
#if HAVE_ZLIB
  z_stream *strm = static_cast<z_stream*>(_opaque);
  inflateEnd(strm);
  SAFE_DELETE(strm);
  SAFE_DELETE_ARRAY(m_output);
  SAFE_DELETE_ARRAY(m_rstream_buf);
#endif
}

bool Decompressor::IsCompleted()
{
  return (m_status == Z_STREAM_END);
}

bool Decompressor::HasBufferError()
{
  return (m_status == Z_BUF_ERROR);
}

bool Decompressor::HasStreamError()
{
  switch(m_status)
  {
  case Z_OK:
  case Z_STREAM_END:
  case Z_BUF_ERROR:
    return false;
  default:
    return true;
  }
}

size_t Decompressor::ReadOutput(char *buf, size_t len)
{
  size_t out = 0;
#if HAVE_ZLIB
  while (len)
  {
    if (m_output_len)
    {
      size_t sz = MIN(m_output_len, len);
      memcpy(buf, m_output + m_output_pos, sz);
      out += sz;
      buf += sz;
      len -= sz;
      m_output_pos += sz;
      m_output_len -= sz;
    }
    else if (m_status != Z_STREAM_END)
    {
      z_stream *strm = static_cast<z_stream*>(_opaque);
      if (!strm->avail_in)
        NextChunk();
      if (!strm->avail_out)
      {
        strm->next_out = (unsigned char*)m_output;
        strm->avail_out = m_chunk_size;
        m_output_pos = 0;
      }
      // Try to inflate chunk
      m_status = inflate(strm, Z_NO_FLUSH);
      if (m_status < 0)
      {
        m_stop = true;
        return 0;
      }
      m_output_len = m_chunk_size - m_output_pos - strm->avail_out;
      m_stop = false;
    }
    else
    {
      m_stop = true;
      break;
    }
  }
#endif
  return out;
}

size_t Decompressor::FetchOutput(const char **data)
{
  *data = 0;
#if HAVE_ZLIB
  size_t len = 0;
  do
  {
    if (m_output_len)
    {
      *data = m_output + m_output_pos;
      len = m_output_len;
      m_output_pos += m_output_len;
      m_output_len = 0;
      return len;
    }
    else if (m_status != Z_STREAM_END)
    {
      z_stream *strm = static_cast<z_stream*>(_opaque);
      if (!strm->avail_in)
        NextChunk();
      if (!strm->avail_out)
      {
        strm->next_out = (unsigned char*)m_output;
        strm->avail_out = m_chunk_size;
        m_output_pos = 0;
      }
      // Try to inflate chunk
      m_status = inflate(strm, Z_NO_FLUSH);
      if (m_status < 0)
      {
        m_stop = true;
        return 0;
      }
      m_output_len = m_chunk_size - m_output_pos - strm->avail_out;
      m_stop = false;
    }
    else
    {
      m_stop = true;
      break;
    }
  } while (!m_stop);
#endif
  return 0;
}

int Decompressor::_init(void *zp, void *out, size_t len)
{
#if HAVE_ZLIB
  z_stream *strm = static_cast<z_stream*>(zp);
  // Prepare inflater status
  strm->zalloc = Z_NULL;
  strm->zfree = Z_NULL;
  strm->opaque = Z_NULL;
  strm->avail_in = 0;
  strm->next_in = Z_NULL;
  strm->avail_out = len;
  strm->next_out = (unsigned char*)out;
  return inflateInit2(strm, GZIP_WINDOWS_BIT);
#else
  return Z_STREAM_ERROR;
#endif
}

size_t Decompressor::NextChunk()
{
  size_t sz = 0;
#if HAVE_ZLIB
  z_stream *strm = static_cast<z_stream*>(_opaque);
  switch(m_type_in)
  {
  case MEM_BUFFER:
    // Determine current chunk size
    sz = MIN(m_chunk_size, m_input_len);
    // Check for end of data
    if (sz)
    {
      // Set stream
      strm->next_in = (unsigned char*)m_input;
      strm->avail_in = (unsigned)sz;
      // Update next chunk
      m_input += sz;
      m_input_len -= sz;
    }
    break;
  case FCB_READER:
    {
      int ret = (*m_rstream)(m_rstream_hdl, (void*)m_rstream_buf, m_chunk_size);
      if (ret > 0)
        sz = ret;
      else
        sz = 0;
      // Set stream
      strm->next_in = (unsigned char*)m_rstream_buf;
      strm->avail_in = (unsigned)sz;
    }
    break;
  default:
    break;
  }
#endif
  return sz;
}
