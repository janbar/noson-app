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

#ifndef WSREQUESTBROKER_H
#define	WSREQUESTBROKER_H

#include <local_config.h>
#include "os/os.h"
#include "wsrequest.h"

#include <string>
#include <vector>
#include <map>

namespace NSROOT
{

  class NetSocket;

  class WSRequestBroker
  {
  public:
    WSRequestBroker(NetSocket* socket, timeval timeout);
    ~WSRequestBroker();

    // testing the broker
    //WSRequestBroker(const WSRequest& request);

    void SetTimeout(timeval timeout);
    bool IsParsed() const { return m_parsed; }
    HRM_t GetParsedMethod() const { return m_parsedMethod; }
    const std::string& GetParsedURI() const { return m_parsedURI; }
    const std::string& GetParsedQueryProtocol() const { return m_parsedQueryProtocol; }
    const std::string& GetParsedNamedEntry(const std::string& name);
    bool HasContent() const { return (m_contentLength > 0); }
    size_t GetContentLength() const { return m_contentLength; }
    size_t ReadContent(char *buf, size_t buflen);
    size_t GetConsumed() const { return m_consumed; }

    static bool ReadHeaderLine(NetSocket *socket, const char *eol, std::string& line, size_t *len);
    static void Tokenize(const std::string& str, const char *delimiters, std::vector<std::string>& tokens, bool trimnull = false);

  private:
    NetSocket* m_socket;
    bool m_parsed;
    HRM_t m_parsedMethod;
    std::string m_parsedURI;
    std::string m_parsedQueryProtocol;
    typedef std::map<std::string, std::string> entries_t;
    entries_t m_namedEntries;
    bool m_contentChunked;
    size_t m_contentLength;
    size_t m_consumed;
    char* m_chunkBuffer;
    char* m_chunkPtr;
    char* m_chunkEnd;

    bool ParseQuery();
  };

}

#endif	/* WSREQUESTBROKER_H */

