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

#ifndef SOCKET_H
#define	SOCKET_H

#include "local_config.h"
#include "os/os.h"

#include <cstddef>  // for size_t
#include <string>

#define SOCKET_HOSTNAME_MAXSIZE       256
#define SOCKET_RCVBUF_MINSIZE         16384
#define SOCKET_READ_TIMEOUT_SEC       10
#define SOCKET_READ_TIMEOUT_USEC      0
#define SOCKET_READ_ATTEMPT           3
#define SOCKET_BUFFER_SIZE            1472
#define SOCKET_CONNECTION_REQUESTS    5

namespace NSROOT
{

  typedef enum {
    SOCKET_AF_INET4,
    SOCKET_AF_INET6,
  } SOCKET_AF_t;

  struct SocketAddress;

  class NetSocket
  {
  public:
    NetSocket()
    {
      m_timeout.tv_sec = SOCKET_READ_TIMEOUT_SEC;
      m_timeout.tv_usec = SOCKET_READ_TIMEOUT_USEC;
    }
    virtual ~NetSocket() { }
    virtual bool SendData(const char* buf, size_t size) = 0;
    virtual size_t ReceiveData(void* buf, size_t n) = 0;
    void SetTimeout(timeval timeout) { m_timeout = timeout; }

  protected:
    struct timeval m_timeout;
  };

  class TcpSocket : public NetSocket
  {
    friend class TcpServerSocket;
  public:
    TcpSocket();
    virtual ~TcpSocket();

    /**
     * @return the last error occuring on call
     */
    int GetErrNo() const { return m_errno; }

    /**
     * Configure the number of timed out attempt reading the socket before
     * returning.
     * @param n the number of attempt
     */
    void SetReadAttempt(int n) { m_attempt = n; }

    /**
     * Try to connect the socket to an address name, port.
     * @param server the destination address name
     * @param port the destination port
     * @param rcvbuf the size of read buffer, else 0 for SOCKET_RCVBUF_MINSIZE
     * @return true on success, else false
     */
    virtual bool Connect(const char *server, unsigned port, int rcvbuf);

    /**
     * Send data into the socket.
     * @param buf the pointer to data
     * @param size the number of byte to send
     * @return true when succeeded, else false
     */
    virtual bool SendData(const char* buf, size_t size);

    /**
     * Read data from the socket.
     * @param buf the pointer to write received data
     * @param n the number of byte to read
     * @return the number of received byte
     */
    virtual size_t ReceiveData(void* buf, size_t n);

    /**
     * Gracefully disconnect the socket.
     */
    virtual void Disconnect();

    /**
     * @return true when socket is connected, else false
     */
    virtual bool IsValid() const;

    /**
     * Check for read readiness. It returns -1 for errors, 0 for occurred
     * timeout, and >0 when incoming data are ready to read.
     * @param timeout
     * @return an int for status
     */
    int Listen(timeval *timeout);

    /**
     * @return the socket handle
     */
    net_socket_t GetHandle() const { return m_socket; }

    /**
     * @return the address string of this host
     */
    std::string GetHostAddrInfo();

    /**
     * @return this host name
     */
    static const char* GetMyHostName();

  protected:
    net_socket_t m_socket;
    int m_rcvbuf;
    int m_errno;
    int m_attempt;

  private:
    char* m_buffer;
    char* m_bufptr;
    size_t m_buflen;
    size_t m_rcvlen;

    // prevent copy
    TcpSocket(const TcpSocket&);
    TcpSocket& operator=(const TcpSocket&);
  };

  class TcpServerSocket
  {
  public:
    TcpServerSocket();
    ~TcpServerSocket();

    /**
     * @return the last error occuring on call
     */
    int GetErrNo() const { return m_errno; }

    /**
     * Initialize the socket for the given protocol.
     * @return true on success, else false
     */
    bool Create(SOCKET_AF_t af);

    /**
     * @return true when socket is created, else false
     */
    bool IsValid() const;

    /**
     * Bind the socket to the given port on any local addresses.
     * @param port
     * @return true on success, else false
     */
    bool Bind(unsigned port);

    /**
     * Prepare to accept connections on the socket.
     * @param queueSize the maximum length for the queue of pending connections
     * @return true on success, else false
     */
    bool ListenConnection(int queueSize = SOCKET_CONNECTION_REQUESTS);

    /**
     * Await a connection.
     * @param socket the tcp socket to connect on new request
     * @return true on success, else false
     */
    bool AcceptConnection(TcpSocket& socket);

    /**
     * Close the socket.
     */
    void Close();

    /**
     * @return the socket handle
     */
    net_socket_t GetHandle() const { return m_socket; }

  private:
    SocketAddress* m_addr;
    net_socket_t m_socket;
    int m_errno;
    unsigned m_requestQueueSize;

    // prevent copy
    TcpServerSocket(const TcpServerSocket&);
    TcpServerSocket& operator=(const TcpServerSocket&);
  };

  class UdpSocket : public NetSocket
  {
  public:
    UdpSocket();
    UdpSocket(size_t bufferSize);
    virtual ~UdpSocket();

    /**
     * @return the last error occuring on call
     */
    int GetErrNo() const { return m_errno; }

    /**
     * Send data into the socket.
     * @param buf the pointer to data
     * @param size the number of byte to send
     * @return true when succeeded, else false
     */
    bool SendData(const char* buf, size_t size);

    /**
     * Read data from the socket.
     * @param buf the pointer to write received data
     * @param n the number of byte to read
     * @return the number of received byte
     */
    size_t ReceiveData(void* buf, size_t n);

    /**
     * @return true when socket is opened, else false
     */
    bool IsValid() const;

    /**
     * Open the socket and configure the given destination.
     * @param af the protocol
     * @param target the address name of destination
     * @param port
     * @return true on success, else false
     */
    bool Open(SOCKET_AF_t af, const char* target, unsigned port);

    /**
     * Open the socket.
     * @param af the protocol
     * @param broadcast enable broadcast permission
     * @return true on success, else false
     */
    bool Open(SOCKET_AF_t af, bool broadcast = false);

    /**
     * Configure the destination.
     * @param target the address name of destination
     * @param port
     * @return true on success, else false
     */
    bool SetAddress(const char* target, unsigned port);

    /**
     * Configure hop limit value to be used for multicast packets on the opened
     * socket.
     * @param multicastTTL
     * @return true on success, else false
     */
    bool SetMulticastTTL(int multicastTTL);

    /**
     * @return the address string of the remote host
     */
    std::string GetRemoteAddrInfo() const;

    /**
     * @return the socket handle
     */
    net_socket_t GetHandle() const { return m_socket; }

  private:
    SocketAddress* m_addr;
    SocketAddress* m_from;
    net_socket_t m_socket;
    int m_errno;
    char* m_buffer;
    char* m_bufptr;
    size_t m_buflen;
    size_t m_rcvlen;

    // prevent copy
    UdpSocket(const UdpSocket&);
    UdpSocket& operator=(const UdpSocket&);
  };

  class UdpServerSocket
  {
  public:
    UdpServerSocket();
    UdpServerSocket(size_t bufferSize);
    ~UdpServerSocket();

    /**
     * @return the last error occuring on call
     */
    int GetErrNo() const { return m_errno; }

    /**
     * Initialize the socket for the given protocol.
     * @return true on success, else false
     */
    bool Create(SOCKET_AF_t af);

    /**
     * @return true when socket is created, else false
     */
    bool IsValid() const;

    /**
     * Bind the socket to the given port on any local addresses.
     * @param port
     * @return true on success, else false
     */
    bool Bind(unsigned port);

    /**
     * Configure hop limit value to be used for multicast packets on the opened
     * socket.
     * @param multicastTTL
     * @return true on success, else false
     */
    bool SetMulticastTTL(int multicastTTL);

    /**
     * Configure socket to join/leave the given multicast group.
     * @param group address name of the group
     * @param join add/drop membership
     * @return true on success, else false
     */
    bool SetMulticastMembership(const char *group, bool join);

    /**
     * Wait for incoming data.
     * @return the size of datagram else 0 when timeout occurred
     */
    size_t AwaitIncoming(timeval timeout);
    size_t AwaitIncoming();

    /**
     * @return the address string of the remote host that provided the incoming
     * datagram
     */
    std::string GetRemoteAddrInfo() const;

    /**
     * Read remaining data from the datagram buffer.
     * @param buf the pointer to write received data
     * @param n the number of byte to read
     * @return the number of received byte
     */
    size_t ReadData(void* buf, size_t n);

    /**
     * @return the socket handle
     */
    net_socket_t GetHandle() const { return m_socket; }

  private:
    SocketAddress* m_addr;
    SocketAddress* m_from;
    net_socket_t m_socket;
    int m_errno;
    char* m_buffer;
    char* m_bufptr;
    size_t m_buflen;
    size_t m_rcvlen;
    struct timeval m_timeout;

    // prevent copy
    UdpServerSocket(const UdpServerSocket&);
    UdpServerSocket& operator=(const UdpServerSocket&);
  };

  class UdpMessageReader : public NetSocket
  {
  public:
    UdpMessageReader(UdpServerSocket& boundSocket)
    : NetSocket()
    , m_bound(boundSocket) { }

    bool SendData(const char* buf, size_t size) { (void)buf; (void)size; return false; };

    size_t ReceiveData(void* buf, size_t n)
    {
      size_t r = 0;
      if (m_bound.IsValid())
      {
        if ((r = m_bound.ReadData(buf, n)) > 0)
          return r;
        if (m_bound.AwaitIncoming(m_timeout) > 0)
          return m_bound.ReadData(buf, n);
      }
      return r;
    }

  private:
    UdpServerSocket& m_bound;
  };

}

#endif	/* SOCKET_H */
