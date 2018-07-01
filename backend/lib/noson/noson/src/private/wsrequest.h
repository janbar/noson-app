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

#ifndef WSREQUEST_H
#define	WSREQUEST_H

#include "local_config.h"
#include "wscontent.h"
#include "uriparser.h"

#include <cstddef>  // for size_t
#include <string>
#include <map>

#define REQUEST_PROTOCOL      "HTTP/1.1"
#define REQUEST_USER_AGENT    "libnoson/1.0"
#define REQUEST_CONNECTION    "close" // "keep-alive"
#define REQUEST_STD_CHARSET   "utf-8"

namespace NSROOT
{

  typedef enum
  {
    HRM_GET,
    HRM_POST,
    HRM_HEAD,
    HRM_SUBSCRIBE,
    HRM_UNSUBSCRIBE,
    HRM_NOTIFY,
  } HRM_t;

  class WSRequest
  {
  public:
    WSRequest(const std::string& server, unsigned port);
    WSRequest(const std::string& server, unsigned port, bool secureURI);
    WSRequest(const URIParser& uri, HRM_t method = HRM_GET);
    ~WSRequest();

    void RequestService(const std::string& url, HRM_t method = HRM_GET);
    void RequestAccept(CT_t contentType);
    void RequestAcceptEncoding(bool yesno);
    void SetUserAgent(const std::string& value);
    void SetContentParam(const std::string& param, const std::string& value);
    void SetContentCustom(CT_t contentType, const char *content);
    void SetHeader(const std::string& field, const std::string& value);
    const std::string& GetContent() const { return m_contentData; }
    void ClearContent();

    void MakeMessage(std::string& msg) const;

    const std::string& GetServer() const { return m_server; }
    unsigned GetPort() const { return m_port; }
    bool IsSecureURI() const { return m_secure_uri; }

  private:
    std::string m_server;
    unsigned m_port;
    bool m_secure_uri;
    std::string m_service_url;
    HRM_t m_service_method;
    std::string m_charset;
    CT_t m_accept;
    CT_t m_contentType;
    std::string m_contentData;
    std::map<std::string, std::string> m_headers;
    std::string m_userAgent;

    void MakeMessageGET(std::string& msg, const char* method = "GET") const;
    void MakeMessagePOST(std::string& msg, const char* method = "POST") const;
    void MakeMessageHEAD(std::string& msg, const char* method = "HEAD") const;
  };

}

#endif	/* WSREQUEST_H */
