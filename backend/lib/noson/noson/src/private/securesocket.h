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

#ifndef SECURESOCKET_H
#define SECURESOCKET_H

#include "socket.h"

#include <string>

namespace NSROOT
{
  class SecureSocket;

  class SSLSessionFactory
  {
  public:
    static SSLSessionFactory& Instance();
    static void Destroy();
    bool isEnabled() const { return m_enabled; }
    SecureSocket* NewSocket(bool disableSSLv2 = true);

  private:
    SSLSessionFactory();
    ~SSLSessionFactory();
    SSLSessionFactory(const SSLSessionFactory&);
    SSLSessionFactory& operator=(const SSLSessionFactory&);

    static SSLSessionFactory* m_instance;
    bool m_enabled;   ///< SSL feature status
    void* m_ctx;      ///< SSL default context for the application
  };

  class SecureSocket : public TcpSocket
  {
    friend class SSLSessionFactory;
  public:
    virtual ~SecureSocket();

    // Overrides TcpSocket
    bool Connect(const char *server, unsigned port, int rcvbuf);
    bool SendData(const char* buf, size_t size);
    size_t ReceiveData(void* buf, size_t n);
    void Disconnect();
    bool IsValid() const;

    bool IsCertificateValid(std::string& info);

  private:
    SecureSocket(void* ssl);

    void* m_ssl;      ///< SSL handle
    void* m_cert;     ///< X509 certificate
    bool m_connected; ///< SSL session state
    int m_ssl_error;  ///< SSL error code
  };
}

#endif /* SECURESOCKET_H */


