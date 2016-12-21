/*
 *      Copyright (C) 2014-2015 Jean-Luc Barriere
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

#include "wsresponse.h"
#include "securesocket.h"
#include "debug.h"
#include "cppdef.h"
#include "compressor.h"

#include <cstdlib>  // for atol
#include <cstdio>
#include <cstring>

#define HTTP_TOKEN_MAXSIZE    20
#define HTTP_HEADER_MAXSIZE   4000
#define RESPONSE_BUFFER_SIZE  4000

using namespace NSROOT;

bool WSResponse::ReadHeaderLine(NetSocket *socket, const char *eol, std::string& line, size_t *len)
{
  char buf[RESPONSE_BUFFER_SIZE];
  const char *s_eol;
  int p = 0, p_eol = 0, l_eol;
  size_t l = 0;

  if (eol != NULL)
    s_eol = eol;
  else
    s_eol = "\n";
  l_eol = strlen(s_eol);

  line.clear();
  do
  {
    if (socket->ReceiveData(&buf[p], 1) > 0)
    {
      if (buf[p++] == s_eol[p_eol])
      {
        if (++p_eol >= l_eol)
        {
          buf[p - l_eol] = '\0';
          line.append(buf);
          l += p - l_eol;
          break;
        }
      }
      else
      {
        p_eol = 0;
        if (p > (RESPONSE_BUFFER_SIZE - 2 - l_eol))
        {
          buf[p] = '\0';
          line.append(buf);
          l += p;
          p = 0;
        }
      }
    }
    else
    {
      /* No EOL found until end of data */
      *len = l;
      return false;
    }
  }
  while (l < HTTP_HEADER_MAXSIZE);

  *len = l;
  return true;
}

WSResponse::WSResponse(const WSRequest &request)
: m_socket(NULL)
, m_successful(false)
, m_statusCode(0)
, m_serverInfo()
, m_etag()
, m_location()
, m_contentType(CT_NONE)
, m_contentEncoding(CE_NONE)
, m_contentChunked(false)
, m_contentLength(0)
, m_consumed(0)
, m_chunkBuffer(NULL)
, m_chunkPtr(NULL)
, m_chunkEOR(NULL)
, m_chunkEnd(NULL)
, m_decoder(NULL)
{
  if (request.IsSecureURI())
    m_socket = SSLSessionFactory::Instance().NewSocket();
  else
    m_socket = new TcpSocket();
  if (!m_socket)
    DBG(DBG_ERROR, "%s: create socket failed\n", __FUNCTION__);
  else if (m_socket->Connect(request.GetServer().c_str(), request.GetPort(), SOCKET_RCVBUF_MINSIZE))
  {
    m_socket->SetReadAttempt(6); // 60 sec to hang up
    if (SendRequest(request) && GetResponse())
    {
      if (m_statusCode < 200)
        DBG(DBG_WARN, "%s: status %d\n", __FUNCTION__, m_statusCode);
      else if (m_statusCode < 300)
        m_successful = true;
      else if (m_statusCode < 400)
        m_successful = false;
      else if (m_statusCode < 500)
        DBG(DBG_ERROR, "%s: bad request (%d)\n", __FUNCTION__, m_statusCode);
      else
        DBG(DBG_ERROR, "%s: server error (%d)\n", __FUNCTION__, m_statusCode);
    }
    else
      DBG(DBG_ERROR, "%s: invalid response\n", __FUNCTION__);
  }
}

WSResponse::~WSResponse()
{
  SAFE_DELETE(m_decoder);
  SAFE_DELETE_ARRAY(m_chunkBuffer);
  SAFE_DELETE(m_socket);
}

bool WSResponse::SendRequest(const WSRequest &request)
{
  std::string msg;

  request.MakeMessage(msg);
  DBG(DBG_PROTO, "%s: %s\n", __FUNCTION__, msg.c_str());
  if (!m_socket->SendData(msg.c_str(), msg.size()))
  {
    DBG(DBG_ERROR, "%s: failed (%d)\n", __FUNCTION__, m_socket->GetErrNo());
    return false;
  }
  return true;
}

bool WSResponse::GetResponse()
{
  size_t len;
  std::string strread;
  char token[HTTP_TOKEN_MAXSIZE + 1];
  int n = 0, token_len = 0;
  bool ret = false;

  token[0] = 0;
  while (ReadHeaderLine(m_socket, "\r\n", strread, &len))
  {
    const char *line = strread.c_str(), *val = NULL;
    int value_len = 0;

    DBG(DBG_PROTO, "%s: %s\n", __FUNCTION__, line);
    /*
     * The first line of a Response message is the Status-Line, consisting of
     * the protocol version followed by a numeric status code and its associated
     * textual phrase, with each element separated by SP characters.
     */
    if (++n == 1)
    {
      int status;
      if (len > 5 && 0 == memcmp(line, "HTTP", 4) && 1 == sscanf(line, "%*s %d", &status))
      {
        /* We have received a valid feedback */
        m_statusCode = status;
        ret = true;
      }
      else
      {
        /* Not a response header */
        return false;
      }
    }

    if (len == 0)
    {
      /* End of header */
      break;
    }

    /*
     * Header fields can be extended over multiple lines by preceding each
     * extra line with at least one SP or HT.
     */
    if ((line[0] == ' ' || line[0] == '\t') && token_len)
    {
      /* Append value of previous token */
      val = line;
    }
      /*
       * Each header field consists of a name followed by a colon (":") and the
       * field value. Field names are case-insensitive. The field value MAY be
       * preceded by any amount of LWS, though a single SP is preferred.
       */
    else if ((val = strchr(line, ':')))
    {
      int p;
      if ((token_len = val - line) > HTTP_TOKEN_MAXSIZE)
        token_len = HTTP_TOKEN_MAXSIZE;
      for (p = 0; p < token_len; ++p)
        token[p] = toupper(line[p]);
      token[token_len] = 0;
      value_len = len - (val - line + 1);
      while (*(++val) == ' ' && value_len > 0) --value_len;
      m_headers.push_front(std::make_pair(token, ""));
    }
    else
    {
      /* Unknown syntax! Close previous token */
      token_len = 0;
      token[token_len] = 0;
    }

    if (token_len)
    {
      m_headers.front().second.append(val);
      switch (token_len)
      {
        case 4:
          if (memcmp(token, "ETAG", token_len) == 0)
            m_etag.append(val);
          break;
        case 6:
          if (memcmp(token, "SERVER", token_len) == 0)
            m_serverInfo.append(val);
          break;
        case 8:
          if (memcmp(token, "LOCATION", token_len) == 0)
            m_location.append(val);
          break;
        case 12:
          if (memcmp(token, "CONTENT-TYPE", token_len) == 0)
            m_contentType = ContentTypeFromMime(val);
          break;
        case 14:
          if (memcmp(token, "CONTENT-LENGTH", token_len) == 0)
            m_contentLength = atol(val);
          break;
        case 16:
          if (memcmp(token, "CONTENT-ENCODING", token_len) == 0)
          {
            if (value_len > 6 && memcmp(val, "deflate", 7) == 0)
              m_contentEncoding = CE_DEFLATE;
            else if (value_len > 3 && memcmp(val, "gzip", 4) == 0)
              m_contentEncoding = CE_GZIP;
            else
            {
              m_contentEncoding = CE_UNKNOWN;
              DBG(DBG_ERROR, "%s: unsupported content encoding (%s) %d\n", __FUNCTION__, val, value_len);
            }
          }
          break;
        case 17:
          if (memcmp(token, "TRANSFER-ENCODING", token_len) == 0)
          {
            if (value_len > 6 && memcmp(val, "chunked", 7) == 0)
              m_contentChunked = true;
          }
          break;
        default:
          break;
      }
    }
  }

  return ret;
}

size_t WSResponse::ReadChunk(void *buf, size_t buflen)
{
  size_t s = 0;
  if (m_contentChunked)
  {
    // no more pending byte in chunk buffer
    if (m_chunkPtr == NULL || m_chunkPtr >= m_chunkEOR)
    {
      // process next chunk if all bytes have been read from the chunk buffer
      SAFE_DELETE_ARRAY(m_chunkBuffer);
      m_chunkBuffer = m_chunkPtr = m_chunkEOR = m_chunkEnd = NULL;
      std::string strread;
      size_t len = 0;
      while (ReadHeaderLine(m_socket, "\r\n", strread, &len) && len == 0);
      DBG(DBG_PROTO, "%s: chunked data (%s)\n", __FUNCTION__, strread.c_str());
      std::string chunkStr("0x0");
      uint32_t chunkSize;
      if (!strread.empty() && sscanf(chunkStr.append(strread).c_str(), "%x", &chunkSize) == 1 && chunkSize > 0)
      {
        if (!(m_chunkBuffer = new char[chunkSize]))
          return 0;
        m_chunkPtr = m_chunkEOR = m_chunkBuffer;
        m_chunkEnd = m_chunkBuffer + chunkSize;
      }
      else
        return 0; // that's the end of chunks
      // ask for new data to fill in the chunk buffer
      // fill at last read position and until to the end
      m_chunkEOR += m_socket->ReceiveData(m_chunkEOR, m_chunkEnd - m_chunkEOR);
    }
    if ((s = m_chunkEOR - m_chunkPtr) > buflen)
      s = buflen;
    memcpy(buf, m_chunkPtr, s);
    m_chunkPtr += s;
    m_consumed += s;
  }
  return s;
}

int WSResponse::SocketStreamReader(void *hdl, void *buf, int sz)
{
  WSResponse *resp = static_cast<WSResponse*>(hdl);
  if (resp == NULL)
    return 0;
  size_t s = 0;
  // let read on unknown length
  if (!resp->m_contentLength)
    s = resp->m_socket->ReceiveData(buf, sz);
  else if (resp->m_contentLength > resp->m_consumed)
  {
    size_t len = resp->m_contentLength - resp->m_consumed;
    s = resp->m_socket->ReceiveData(buf, len > (size_t)sz ? (size_t)sz : len);
  }
  resp->m_consumed += s;
  return s;
}

int WSResponse::ChunkStreamReader(void *hdl, void *buf, int sz)
{
  WSResponse *resp = static_cast<WSResponse*>(hdl);
  return (resp == NULL ? 0 : resp->ReadChunk(buf, sz));
}

size_t WSResponse::ReadContent(char* buf, size_t buflen)
{
  size_t s = 0;
  if (!m_contentChunked)
  {
    if (m_contentEncoding == CE_NONE)
    {
      // let read on unknown length
      if (!m_contentLength)
        s = m_socket->ReceiveData(buf, buflen);
      else if (m_contentLength > m_consumed)
      {
        size_t len = m_contentLength - m_consumed;
        s = m_socket->ReceiveData(buf, len > buflen ? buflen : len);
      }
      m_consumed += s;
    }
    else if (m_contentEncoding == CE_GZIP || m_contentEncoding == CE_DEFLATE)
    {
      if (m_decoder == NULL)
        m_decoder = new Decompressor(&SocketStreamReader, this);
      if (m_decoder->HasOutputData())
        s = m_decoder->ReadOutput(buf, buflen);
      if (s == 0 && !m_decoder->IsCompleted())
      {
        if (m_decoder->HasStreamError())
          DBG(DBG_ERROR, "%s: decoding failed: stream error\n", __FUNCTION__);
        else if (m_decoder->HasBufferError())
          DBG(DBG_ERROR, "%s: decoding failed: buffer error\n", __FUNCTION__);
        else
          DBG(DBG_ERROR, "%s: decoding failed\n", __FUNCTION__);
      }
    }
  }
  else
  {
    if (m_contentEncoding == CE_NONE)
    {
      s = ReadChunk(buf, buflen);
    }
    else if (m_contentEncoding == CE_GZIP || m_contentEncoding == CE_DEFLATE)
    {
      if (m_decoder == NULL)
        m_decoder = new Decompressor(&ChunkStreamReader, this);
      if (m_decoder->HasOutputData())
        s = m_decoder->ReadOutput(buf, buflen);
      if (s == 0 && !m_decoder->IsCompleted())
      {
        if (m_decoder->HasStreamError())
          DBG(DBG_ERROR, "%s: decoding failed: stream error\n", __FUNCTION__);
        else if (m_decoder->HasBufferError())
          DBG(DBG_ERROR, "%s: decoding failed: buffer error\n", __FUNCTION__);
        else
          DBG(DBG_ERROR, "%s: decoding failed\n", __FUNCTION__);
      }
    }
  }
  return s;
}

bool WSResponse::GetHeaderValue(const std::string& header, std::string& value)
{
  for (HeaderList::const_iterator it = m_headers.begin(); it != m_headers.end(); ++it)
  {
    if (it->first != header)
      continue;
    value.assign(it->second);
    return true;
  }
  return false;
}
