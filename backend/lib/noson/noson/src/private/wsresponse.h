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

#ifndef WSRESPONSE_H
#define	WSRESPONSE_H

#include <local_config.h>
#include "wscontent.h"
#include "wsrequest.h"

#include <cstddef>  // for size_t
#include <string>
#include <list>

namespace NSROOT
{

  class NetSocket;
  class TcpSocket;
  class Decompressor;
 
  class WSResponse
  {
  public:
    WSResponse(const WSRequest& request);
    ~WSResponse();

    bool IsSuccessful() const { return m_successful; }
    bool IsChunkedTransfer() const { return m_contentChunked; }
    size_t GetContentLength() const { return m_contentLength; }
    size_t ReadContent(char *buf, size_t buflen);
    size_t GetConsumed() const { return m_consumed; }
    int GetStatusCode() const { return m_statusCode; }
    const std::string& Redirection() const { return m_location; }

    bool GetHeaderValue(const std::string& header, std::string& value);

    static bool ReadHeaderLine(NetSocket *socket, const char *eol, std::string& line, size_t *len);

  private:
    TcpSocket *m_socket;
    bool m_successful;
    int m_statusCode;
    std::string m_serverInfo;
    std::string m_etag;
    std::string m_location;
    CT_t m_contentType;
    CE_t m_contentEncoding;
    bool m_contentChunked;
    size_t m_contentLength;
    size_t m_consumed;
    char* m_chunkBuffer;      ///< The chunk data buffer
    char* m_chunkPtr;         ///< The next position to read data from the chunk
    char* m_chunkEOR;         ///< The end of received data in the chunk
    char* m_chunkEnd;         ///< The end of the chunk buffer
    Decompressor *m_decoder;

    typedef std::list<std::pair<std::string, std::string> > HeaderList;
    HeaderList m_headers;

    // prevent copy
    WSResponse(const WSResponse&);
    WSResponse& operator=(const WSResponse&);

    bool SendRequest(const WSRequest& request);
    bool GetResponse();
    size_t ReadChunk(void *buf, size_t buflen);
    static int SocketStreamReader(void *hdl, void *buf, int sz);
    static int ChunkStreamReader(void *hdl, void *buf, int sz);
  };

}

#endif	/* WSRESPONSE_H */
