/*
 *      Copyright (C) 2014-2019 Jean-Luc Barriere
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

#include "socket.h"
#include "debug.h"

#include <errno.h>
#include <cstdio>
#include <cstring>

#ifdef __WINDOWS__
#include <WS2tcpip.h>
#define SHUT_RDWR SD_BOTH
#define SHUT_WR   SD_SEND
#define LASTERROR WSAGetLastError()
#define ERRNO_INTR WSAEINTR
typedef int socklen_t;
typedef IN_ADDR in_addr_t;

#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#define closesocket(a) close(a)
#define LASTERROR errno
#define ERRNO_INTR EINTR
#endif /* __WINDOWS__ */

#include <signal.h>

using namespace NSROOT;

static int __addressFamily(SOCKET_AF_t saf)
{
  switch(saf)
  {
    case SOCKET_AF_INET4:
      return AF_INET;
    case SOCKET_AF_INET6:
      return AF_INET6;
    default:
      return AF_UNSPEC;
  }
}

namespace NSROOT
{

  struct SocketAddress
  {
    sockaddr_storage data;
    socklen_t sa_len;

    SocketAddress() { Clear(AF_UNSPEC); }
    SocketAddress(int family) { Clear(family); }
    inline sockaddr* sa() { return (sockaddr*)&data; }
    inline int sa_family() { return data.ss_family; }
    void Clear(int family)
    {
      memset(&data, 0, sizeof(data));
      data.ss_family = family;
      sa_len = (family == AF_INET ? sizeof(sockaddr_in) : sizeof(sockaddr_in6));
    }
  };

}

///////////////////////////////////////////////////////////////////////////////
////
//// TCP socket
////

static char my_hostname[SOCKET_HOSTNAME_MAXSIZE];
static volatile net_socket_t my_socket;

static void __sigHandler(int sig)
{
  closesocket(my_socket);
  my_socket = INVALID_SOCKET_VALUE;
  (void)sig;
}

TcpSocket::TcpSocket()
: NetSocket()
, m_socket(INVALID_SOCKET_VALUE)
, m_rcvbuf(SOCKET_RCVBUF_MINSIZE)
, m_errno(0)
, m_attempt(SOCKET_READ_ATTEMPT)
, m_buffer(NULL)
, m_bufptr(NULL)
, m_buflen(SOCKET_BUFFER_SIZE)
, m_rcvlen(0)
{
}

TcpSocket::~TcpSocket()
{
  if (IsValid())
    Disconnect();
  if (m_buffer)
    delete[] m_buffer;
}

static int __connectAddr(struct addrinfo *addr, net_socket_t *s, int rcvbuf)
{
#ifndef __WINDOWS__
  void (*old_sighandler)(int);
  int old_alarm;
#endif
  socklen_t size;
  int err = 0, opt_rcvbuf;

  if ((my_hostname[0] == '\0') && (gethostname(my_hostname, sizeof (my_hostname)) < 0))
  {
    err = LASTERROR;
    DBG(DBG_ERROR, "%s: gethostname failed (%d)\n", __FUNCTION__, err);
    return err;
  }

  *s = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
  if (*s == INVALID_SOCKET_VALUE)
  {
    err = LASTERROR;
    DBG(DBG_ERROR, "%s: invalid socket (%d)\n", __FUNCTION__, err);
    return err;
  }

  opt_rcvbuf = (rcvbuf < SOCKET_RCVBUF_MINSIZE ? SOCKET_RCVBUF_MINSIZE : rcvbuf);
  size = sizeof (opt_rcvbuf);
  if (setsockopt(*s, SOL_SOCKET, SO_RCVBUF, (char *)&opt_rcvbuf, size))
    DBG(DBG_WARN, "%s: could not set rcvbuf from socket (%d)\n", __FUNCTION__, LASTERROR);
  if (getsockopt(*s, SOL_SOCKET, SO_RCVBUF, (char *)&opt_rcvbuf, &size))
    DBG(DBG_WARN, "%s: could not get rcvbuf from socket (%d)\n", __FUNCTION__, LASTERROR);

#ifdef SO_NOSIGPIPE
  int opt_set = 1;
  if (setsockopt(*s, SOL_SOCKET, SO_NOSIGPIPE, (void *)&opt_set, sizeof(int)))
    DBG(DBG_WARN, "%s: could not set nosigpipe from socket (%d)\n", __FUNCTION__, LASTERROR);
#endif

#ifndef __WINDOWS__
  old_sighandler = signal(SIGALRM, __sigHandler);
  old_alarm = alarm(5);
  my_socket = *s;
#endif
  if (connect(*s, addr->ai_addr, addr->ai_addrlen) < 0)
  {
    err = LASTERROR;
    DBG(DBG_ERROR, "%s: failed to connect (%d)\n", __FUNCTION__, err);
    closesocket(*s);
    *s = INVALID_SOCKET_VALUE;
#ifndef __WINDOWS__
    signal(SIGALRM, old_sighandler);
    alarm(old_alarm);
#endif
    return err;
  }
#ifndef __WINDOWS__
  my_socket = INVALID_SOCKET_VALUE;
  signal(SIGALRM, old_sighandler);
  alarm(old_alarm);
#endif
  DBG(DBG_PROTO, "%s: connected to socket(%p)\n", __FUNCTION__, s);
  return err;
}

bool TcpSocket::Connect(const char *server, unsigned port, int rcvbuf)
{
  struct addrinfo hints;
  struct addrinfo *result, *addr;
  char service[33];
  int err;

  if (IsValid())
    Disconnect();

  if (rcvbuf > SOCKET_RCVBUF_MINSIZE)
    m_rcvbuf = rcvbuf;

  memset(&hints, 0, sizeof (hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;
  snprintf(service, sizeof(service), "%u", port);

  err = getaddrinfo(server, service, &hints, &result);
  if (err)
  {
    switch (err)
    {
      case EAI_NONAME:
        DBG(DBG_ERROR, "%s: the specified host is unknown\n", __FUNCTION__);
        break;
      case EAI_FAIL:
        DBG(DBG_ERROR, "%s: a non-recoverable failure in name resolution occurred\n", __FUNCTION__);
        break;
      case EAI_MEMORY:
        DBG(DBG_ERROR, "%s: a memory allocation failure occurred\n", __FUNCTION__);
        break;
      case EAI_AGAIN:
        DBG(DBG_ERROR, "%s: a temporary error occurred on an authoritative name server\n", __FUNCTION__);
        break;
      default:
        DBG(DBG_ERROR, "%s: unknown error %d\n", __FUNCTION__, err);
        break;
    }
    m_errno = err;
    return false;
  }

  for (addr = result; addr; addr = addr->ai_next)
  {
    err = __connectAddr(addr, &m_socket, m_rcvbuf);
    if (!err)
      break;
  }
  freeaddrinfo(result);
  m_errno = err;
  return (err ? false : true);
}

bool TcpSocket::SendData(const char *msg, size_t size)
{
  if (IsValid())
  {

#if defined(__WINDOWS__) || defined(SO_NOSIGPIPE)
    size_t s = send(m_socket, msg, size, 0);
    if (s != size)
    {
      m_errno = LASTERROR;
      return false;
    }
#elif defined(MSG_NOSIGNAL)
    size_t s = send(m_socket, msg, size, MSG_NOSIGNAL);
    if (s != size)
    {
      m_errno = LASTERROR;
      return false;
    }
#else
    sigset_t sig_block, sig_restore, sig_pending;
    sigemptyset(&sig_block);
    sigaddset(&sig_block, SIGPIPE);
    /*
     * Block SIGPIPE for this thread.
     * This works since kernel sends SIGPIPE to the thread that called write(),
     * not to the whole process.
     */
    if (pthread_sigmask(SIG_BLOCK, &sig_block, &sig_restore) != 0)
      return false;
    /*
     * Check if SIGPIPE is already pending.
     */
    int sigpipe_pending = -1;
    if (sigpending(&sig_pending) != -1)
      sigpipe_pending = sigismember(&sig_pending, SIGPIPE);
    if (sigpipe_pending == -1)
    {
      pthread_sigmask(SIG_SETMASK, &sig_restore, NULL);
      return false;
    }

    size_t s = send(m_socket, msg, size, 0);
    if (s != size)
    {
      m_errno = LASTERROR;
      /*
       * Fetch generated SIGPIPE if write() failed with EPIPE.
       * However, if SIGPIPE was already pending before calling write(), it was
       * also generated and blocked by caller, and caller may expect that it can
       * fetch it later. Since signals are not queued, we don't fetch it in this
       * case.
       */
      if (m_errno == EPIPE && sigpipe_pending == 0)
      {
        struct timespec ts;
        ts.tv_sec = 0;
        ts.tv_nsec = 0;
        int sig;
        while ((sig = sigtimedwait(&sig_block, 0, &ts)) == -1 && LASTERROR == EINTR);
      }
      pthread_sigmask(SIG_SETMASK, &sig_restore, NULL);
      return false;
    }
    pthread_sigmask(SIG_SETMASK, &sig_restore, NULL);
#endif

    m_errno = 0;
    return true;
  }
  m_errno = ENOTCONN;
  return false;
}

size_t TcpSocket::ReceiveData(void *buf, size_t n)
{
  if (IsValid())
  {
    m_errno = 0;
    size_t rcvlen = 0;
    char *p = (char*)buf;

    // Check for data remaining in buffer
    if (m_buffer)
    {
      if (m_bufptr < m_buffer + m_rcvlen)
      {
        rcvlen = m_rcvlen - (m_bufptr - m_buffer);
        if (rcvlen > n)
          rcvlen = n;
        memcpy(p, m_bufptr, rcvlen);
        m_bufptr += rcvlen;
        p += rcvlen;
        n -= rcvlen;
        if (n == 0)
          return rcvlen;
      }
    }
    else if ((m_buffer = new char[m_buflen]) == NULL)
    {
      m_errno = ENOMEM;
      DBG(DBG_ERROR, "%s: cannot allocate %u bytes for buffer\n", __FUNCTION__, m_buflen);
      return 0;
    }
    // Reset buffer
    m_bufptr = m_buffer;
    m_rcvlen = 0;

    struct timeval tv;
    fd_set fds;
    int r = 0, hangcount = 0;

    while (n > 0)
    {
      tv = m_timeout;
      FD_ZERO(&fds);
      FD_SET(m_socket, &fds);
      r = select(m_socket + 1, &fds, NULL, NULL, &tv);
      if (r > 0)
      {
        // Under threshold use buffering
        if (n < m_buflen)
        {
          if ((r = recv(m_socket, m_buffer, m_buflen, 0)) > 0)
          {
            m_rcvlen = r;
            size_t s = r;
            if (s > n)
              s = n;
            memcpy(p, m_buffer, s);
            m_bufptr = m_buffer + s;
            p += s;
            n -= s;
            rcvlen += s;
          }
        }
        // No buffering
        else
        {
          if ((r = recv(m_socket, p, n, 0)) > 0)
          {
            p += r;
            n -= r;
            rcvlen += r;
          }
        }
      }
      if (r == 0)
      {
        DBG(DBG_WARN, "%s: socket(%p) timed out (%d)\n", __FUNCTION__, &m_socket, hangcount);
        m_errno = ETIMEDOUT;
        if (++hangcount >= m_attempt)
          break;
      }
      if (r < 0)
      {
        m_errno = LASTERROR;
        if (m_errno != ERRNO_INTR)
          break;
      }
    }
    return rcvlen;
  }
  m_errno = ENOTCONN;
  return 0;
}

void TcpSocket::Disconnect()
{
  if (IsValid())
  {
    char buf[256];
    struct timeval tv;
    fd_set fds;
    int r = 0;

    shutdown(m_socket, SHUT_RDWR);

    tv.tv_sec = 5;
    tv.tv_usec = 0;
    do
    {
      FD_ZERO(&fds);
      FD_SET(m_socket, &fds);
      r = select(m_socket + 1, &fds, NULL, NULL, &tv);
      if (r > 0)
        r = recv(m_socket, buf, sizeof(buf), 0);
    } while (r > 0);

    closesocket(m_socket);
    m_socket = INVALID_SOCKET_VALUE;
    m_rcvlen = 0;
  }
}

bool TcpSocket::IsValid() const
{
  return (m_socket == INVALID_SOCKET_VALUE ? false : true);
}

int TcpSocket::Listen(timeval *timeout)
{
  if (IsValid())
  {
    fd_set fds;
    int r;

    FD_ZERO(&fds);
    FD_SET(m_socket, &fds);
    r = select(m_socket + 1, &fds, NULL, NULL, timeout);
    if (r < 0)
      m_errno = LASTERROR;
    return r;
  }
  m_errno = ENOTCONN;
  return -1;
}

std::string TcpSocket::GetHostAddrInfo()
{
  char host[INET6_ADDRSTRLEN];
  memset(host, 0, INET6_ADDRSTRLEN);

  if (!IsValid())
    return host;

  char addr[sizeof(sockaddr_in6)];
  socklen_t addr_len = sizeof(addr);

  if (getsockname(m_socket, (sockaddr*)&addr, &addr_len) == 0)
    getnameinfo((sockaddr*)&addr, addr_len, host, sizeof(host), NULL, 0, NI_NUMERICHOST);
  else
    m_errno = LASTERROR;

  return host;
}

const char* TcpSocket::GetMyHostName()
{
  return my_hostname;
}

///////////////////////////////////////////////////////////////////////////////
////
//// TCP server socket
////

TcpServerSocket::TcpServerSocket()
: m_socket(INVALID_SOCKET_VALUE)
, m_errno(0)
, m_requestQueueSize(0)
{
  m_addr = new(SocketAddress);
}

TcpServerSocket::~TcpServerSocket()
{
  Close();
  if (m_addr)
  {
    delete(m_addr);
    m_addr = NULL;
  }
}

bool TcpServerSocket::Create(SOCKET_AF_t af)
{
  if (IsValid())
    return false;

  m_addr->Clear(__addressFamily(af));
  m_socket = socket(m_addr->sa_family(), SOCK_STREAM, 0);
  if (!IsValid())
  {
    m_errno = LASTERROR;
    DBG(DBG_ERROR, "%s: invalid socket (%d)\n", __FUNCTION__, m_errno);
    return false;
  }

#ifdef __WINDOWS__
  // The bind will succeed even an other socket is currently listening on the
  // same address. So enable the option SO_EXCLUSIVEADDRUSE will fix the issue.
  int opt_exclusive = 1;
  if (setsockopt(m_socket, SOL_SOCKET, SO_EXCLUSIVEADDRUSE, (char*)&opt_exclusive, sizeof(opt_exclusive)))
  {
    m_errno = LASTERROR;
    DBG(DBG_ERROR, "%s: could not set exclusiveaddruse from socket (%d)\n", __FUNCTION__, m_errno);
    return false;
  }
#else
  // Reuse address. The bind will fail only if an other socket is currently
  // listening on the same address.
  int opt_reuseaddr = 1;
  if (setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, (char*)&opt_reuseaddr, sizeof(opt_reuseaddr)))
  {
    m_errno = LASTERROR;
    DBG(DBG_ERROR, "%s: could not set reuseaddr from socket (%d)\n", __FUNCTION__, m_errno);
    return false;
  }
#endif

  return true;
}

bool TcpServerSocket::IsValid() const
{
  return (m_socket == INVALID_SOCKET_VALUE ? false : true);
}

bool TcpServerSocket::Bind(unsigned port)
{
  if (!IsValid())
    return false;
  int r = 0;

  m_addr->Clear(m_addr->sa_family());
  switch (m_addr->sa_family())
  {
    case AF_INET:
    {
      sockaddr_in* sa = (sockaddr_in*)m_addr->sa();
      sa->sin_addr.s_addr = htonl(INADDR_ANY);
      sa->sin_port = htons(port);
      r = bind(m_socket, m_addr->sa(), m_addr->sa_len);
      break;
    }
    case AF_INET6:
    {
      sockaddr_in6* sa = (sockaddr_in6*)m_addr->sa();
      sa->sin6_addr = in6addr_any;
      sa->sin6_port = htons(port);
      r = bind(m_socket, m_addr->sa(), m_addr->sa_len);
      break;
    }
  }

  if (r)
  {
    m_errno = LASTERROR;
    DBG(DBG_ERROR, "%s: could not bind to address (%d)\n", __FUNCTION__, m_errno);
    return false;
  }
  return true;
}

bool TcpServerSocket::ListenConnection(int queueSize /*= SOCKET_CONNECTION_REQUESTS*/)
{
  if (!IsValid())
    return false;

  if (listen(m_socket, queueSize))
  {
    m_errno = LASTERROR;
    DBG(DBG_ERROR, "%s: listen failed (%d)\n", __FUNCTION__, m_errno);
    return false;
  }
  m_requestQueueSize = queueSize;
  return true;
}

bool TcpServerSocket::AcceptConnection(TcpSocket& socket)
{
  socket.m_socket = accept(m_socket, m_addr->sa(), &m_addr->sa_len);
  if (!socket.IsValid())
  {
    m_errno = LASTERROR;
    DBG(DBG_ERROR, "%s: accept failed (%d)\n", __FUNCTION__, m_errno);
    return false;
  }

#ifdef SO_NOSIGPIPE
  int opt_set = 1;
  if (setsockopt(socket.m_socket, SOL_SOCKET, SO_NOSIGPIPE, (void *)&opt_set, sizeof(int)))
    DBG(DBG_WARN, "%s: could not set nosigpipe from socket (%d)\n", __FUNCTION__, LASTERROR);
#endif

  socket.SetReadAttempt(0);
  return true;
}

void TcpServerSocket::Close()
{
  if (IsValid())
  {
    closesocket(m_socket);
    m_socket = INVALID_SOCKET_VALUE;
  }
}

///////////////////////////////////////////////////////////////////////////////
////
//// UDP socket
////

UdpSocket::UdpSocket()
: m_socket(INVALID_SOCKET_VALUE)
, m_errno(0)
, m_buffer(NULL)
, m_bufptr(NULL)
, m_buflen(SOCKET_BUFFER_SIZE)
, m_rcvlen(0)
{
  m_addr = new SocketAddress;
  m_from = new SocketAddress;
}

UdpSocket::UdpSocket(size_t bufferSize)
: m_socket(INVALID_SOCKET_VALUE)
, m_errno(0)
, m_buffer(NULL)
, m_bufptr(NULL)
, m_buflen(bufferSize)
, m_rcvlen(0)
{
  m_addr = new SocketAddress;
  m_from = new SocketAddress;
}

UdpSocket::~UdpSocket()
{
  if (IsValid())
  {
    closesocket(m_socket);
    m_socket = INVALID_SOCKET_VALUE;
  }
  if (m_addr)
  {
    delete m_addr;
    m_addr = NULL;
  }
  if (m_from)
  {
    delete m_from;
    m_from = NULL;
  }
  if (m_buffer)
  {
    delete[] m_buffer;
    m_buffer = m_bufptr = NULL;
  }
}

bool UdpSocket::Open(SOCKET_AF_t af, const char* target, unsigned port)
{
  return Open(af) && SetAddress(target, port);
}

bool UdpSocket::Open(SOCKET_AF_t af, bool broadcast /*= false*/)
{
  if (IsValid() && m_addr->sa_family() != __addressFamily(af))
  {
    closesocket(m_socket);
    m_socket = INVALID_SOCKET_VALUE;
  }
  if (m_socket == INVALID_SOCKET_VALUE)
  {
    m_addr->Clear(__addressFamily(af));
    m_from->Clear(AF_UNSPEC);
    if ((m_socket = socket(m_addr->sa_family(), SOCK_DGRAM, IPPROTO_UDP)) == INVALID_SOCKET_VALUE)
    {
      m_errno = LASTERROR;
      DBG(DBG_ERROR, "%s: create socket failed (%d)\n", __FUNCTION__, m_errno);
      return false;
    }
    if (broadcast && af == SOCKET_AF_INET4)
    {
      // set broadcast permission
      int _broadcast = 1;
      if (setsockopt(m_socket, SOL_SOCKET, SO_BROADCAST, (char*)&_broadcast, sizeof(_broadcast)))
      {
        m_errno = LASTERROR;
        DBG(DBG_ERROR, "%s: could not set SO_BROADCAST from socket (%d)\n", __FUNCTION__, m_errno);
        return false;
      }
    }
  }
  m_errno = 0;
  return true;
}

bool UdpSocket::SetAddress(const char* target, unsigned port)
{
  if (!IsValid())
  {
    DBG(DBG_ERROR, "%s: invalid socket\n", __FUNCTION__);
    return false;
  }

  unsigned char _addr[sizeof(struct in6_addr)];
  if (inet_pton(m_addr->sa_family(), target, &_addr) == 0)
  {
    m_errno = LASTERROR;
    DBG(DBG_ERROR, "%s: invalid address (%s)\n", __FUNCTION__, target);
    return false;
  }

  m_addr->Clear(m_addr->sa_family());
  switch(m_addr->sa_family())
  {
    case AF_INET:
    {
      sockaddr_in* sa = (sockaddr_in*)m_addr->sa();
      memcpy(&(sa->sin_addr.s_addr), _addr, sizeof(in_addr_t));
      sa->sin_port = htons(port);
      break;
    }
    case AF_INET6:
    {
      sockaddr_in6* sa = (sockaddr_in6*)m_addr->sa();
      memcpy(&(sa->sin6_addr), _addr, sizeof(struct in6_addr));
      sa->sin6_port = htons(port);
      break;
    }
    default:
      m_errno = EINVAL;
      DBG(DBG_ERROR, "%s: address familly unknown (%d)\n", __FUNCTION__, m_addr->sa_family());
      return false;
  }
  m_errno = 0;
  return true;
}

bool UdpSocket::SetMulticastTTL(int multicastTTL)
{
  if (!IsValid())
    return false;

  switch(m_addr->sa_family())
  {
    case AF_INET:
    {
      // The v4 multicast TTL socket option requires that the value be passed in an unsigned char
      unsigned char _ttl = (unsigned char) multicastTTL;
      if (setsockopt(m_socket, IPPROTO_IP, IP_MULTICAST_TTL, (char*)&_ttl, sizeof(_ttl)))
      {
        m_errno = LASTERROR;
        DBG(DBG_ERROR, "%s: could not set IP_MULTICAST_TTL from socket (%d)\n", __FUNCTION__, m_errno);
        return false;
      }
      break;
    }
    case AF_INET6:
    {
      // The v6 multicast TTL socket option requires that the value be passed in as an integer
      if (setsockopt(m_socket, IPPROTO_IPV6, IPV6_MULTICAST_HOPS, (char*)&multicastTTL, sizeof(multicastTTL)))
      {
        m_errno = LASTERROR;
        DBG(DBG_ERROR, "%s: could not set IPV6_MULTICAST_HOPS from socket (%d)\n", __FUNCTION__, m_errno);
        return false;
      }
      break;
    }
    default:
      m_errno = EINVAL;
      DBG(DBG_ERROR, "%s: address familly unknown (%d)\n", __FUNCTION__, m_addr->sa_family());
      return false;
  }
  m_errno = 0;
  return true;
}

bool UdpSocket::SendData(const char* buf, size_t size)
{
  if (IsValid())
  {
    size_t s = sendto(m_socket, buf, size, 0, m_addr->sa(), m_addr->sa_len);
    if (s != size)
    {
      m_errno = LASTERROR;
      return false;
    }
    m_errno = 0;
    return true;
  }
  m_errno = ENOTSOCK;
  return false;
}

size_t UdpSocket::ReceiveData(void* buf, size_t n)
{
  if (IsValid())
  {
    m_errno = 0;
    size_t len = 0;

    // fill rest of data from buffer
    if (m_buffer)
    {
      if (m_bufptr < m_buffer + m_rcvlen)
      {
        len = m_rcvlen - (m_bufptr - m_buffer);
        if (len > n)
          len = n;
        memcpy(buf, m_bufptr, len);
        m_bufptr += len;
        return len;
      }
    }
    else if ((m_buffer = new char[m_buflen]) == NULL)
    {
      m_errno = ENOMEM;
      DBG(DBG_ERROR, "%s: cannot allocate %u bytes for buffer\n", __FUNCTION__, m_buflen);
      return 0;
    }
    // fill buffer with the next incoming datagram
    m_bufptr = m_buffer;
    m_rcvlen = 0;

    struct timeval tv;
    fd_set fds;
    int r = 0;

    tv = m_timeout;
    FD_ZERO(&fds);
    FD_SET(m_socket, &fds);
    r = select(m_socket + 1, &fds, NULL, NULL, &tv);
    if (r > 0)
    {
      if ((r = recvfrom(m_socket, m_buffer, m_buflen, 0, m_from->sa(), &m_from->sa_len)) > 0)
      {
        m_rcvlen = len = r;
        if (m_rcvlen == m_buflen)
          DBG(DBG_WARN, "%s: datagram have been truncated (%d)\n", __FUNCTION__, r);
        if (len > n)
          len = n;
        memcpy(buf, m_buffer, len);
        m_bufptr += len;
      }
    }
    if (r == 0)
    {
      m_errno = ETIMEDOUT;
      DBG(DBG_DEBUG, "%s: socket(%p) timed out\n", __FUNCTION__, &m_socket);
    }
    if (r < 0)
    {
      m_errno = LASTERROR;
      DBG(DBG_ERROR, "%s: socket(%p) read error (%d)\n", __FUNCTION__, &m_socket, m_errno);
    }
    return len;
  }
  m_errno = ENOTSOCK;
  return 0;
}

bool UdpSocket::IsValid() const
{
  return (m_socket == INVALID_SOCKET_VALUE ? false : true);
}

std::string UdpSocket::GetRemoteAddrInfo() const
{
  char host[INET6_ADDRSTRLEN];
  memset(host, 0, INET6_ADDRSTRLEN);
  getnameinfo(m_from->sa(), m_from->sa_len, host, sizeof(host), NULL, 0, NI_NUMERICHOST);
  return host;
}

///////////////////////////////////////////////////////////////////////////////
////
//// UDP server socket
////

UdpServerSocket::UdpServerSocket()
: m_socket(INVALID_SOCKET_VALUE)
, m_errno(0)
, m_buffer(NULL)
, m_bufptr(NULL)
, m_buflen(SOCKET_BUFFER_SIZE)
, m_rcvlen(0)
{
  m_addr = new SocketAddress;
  m_from = new SocketAddress;
  m_timeout.tv_sec = SOCKET_READ_TIMEOUT_SEC;
  m_timeout.tv_usec = SOCKET_READ_TIMEOUT_USEC;

}

UdpServerSocket::UdpServerSocket(size_t bufferSize)
: m_socket(INVALID_SOCKET_VALUE)
, m_errno(0)
, m_buffer(NULL)
, m_bufptr(NULL)
, m_buflen(bufferSize)
, m_rcvlen(0)
{
  m_addr = new SocketAddress;
  m_from = new SocketAddress;
  m_timeout.tv_sec = SOCKET_READ_TIMEOUT_SEC;
  m_timeout.tv_usec = SOCKET_READ_TIMEOUT_USEC;
}

UdpServerSocket::~UdpServerSocket()
{
  if (IsValid())
  {
    closesocket(m_socket);
    m_socket = INVALID_SOCKET_VALUE;
  }
  if (m_addr)
  {
    delete m_addr;
    m_addr = NULL;
  }
  if (m_from)
  {
    delete m_from;
    m_from = NULL;
  }
  if (m_buffer)
  {
    delete[] m_buffer;
    m_buffer = m_bufptr = NULL;
  }
}

bool UdpServerSocket::Create(SOCKET_AF_t af)
{
  if (IsValid())
    return false;

  m_addr->Clear(__addressFamily(af));
  m_socket = socket(m_addr->sa_family(), SOCK_DGRAM, 0);
  if (!IsValid())
  {
    m_errno = LASTERROR;
    DBG(DBG_ERROR, "%s: invalid socket (%d)\n", __FUNCTION__, m_errno);
    return false;
  }

  int opt_reuseaddr = 1;
  if (setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, (char*)&opt_reuseaddr, sizeof(opt_reuseaddr)))
  {
    m_errno = LASTERROR;
    DBG(DBG_ERROR, "%s: could not set reuseaddr from socket (%d)\n", __FUNCTION__, m_errno);
    return false;
  }
  return true;
}

bool UdpServerSocket::IsValid() const
{
  return (m_socket == INVALID_SOCKET_VALUE ? false : true);
}

bool UdpServerSocket::Bind(unsigned port)
{
  if (!IsValid())
    return false;

  m_addr->Clear(m_addr->sa_family());
  switch (m_addr->sa_family())
  {
    case AF_INET:
    {
      sockaddr_in* sa = (sockaddr_in*)m_addr->sa();
      sa->sin_addr.s_addr = htonl(INADDR_ANY);
      sa->sin_port = htons(port);
      break;
    }
    case AF_INET6:
    {
      sockaddr_in6* sa = (sockaddr_in6*)m_addr->sa();
      sa->sin6_addr = in6addr_any;
      sa->sin6_port = htons(port);
      break;
    }
    default:
      m_errno = EINVAL;
      DBG(DBG_ERROR, "%s: address familly unknown (%d)\n", __FUNCTION__, m_addr->sa_family());
      return false;
  }

  if (bind(m_socket, m_addr->sa(), m_addr->sa_len) != 0)
  {
    m_errno = LASTERROR;
    DBG(DBG_ERROR, "%s: could not bind to address (%d)\n", __FUNCTION__, m_errno);
    return false;
  }
  m_errno = 0;
  return true;
}

bool UdpServerSocket::SetMulticastTTL(int multicastTTL)
{
  if (!IsValid())
    return false;

  switch(m_addr->sa_family())
  {
    case AF_INET:
    {
      // The v4 multicast TTL socket option requires that the value be passed in an unsigned char
      unsigned char _ttl = (unsigned char) multicastTTL;
      if (setsockopt(m_socket, IPPROTO_IP, IP_MULTICAST_TTL, (char*)&_ttl, sizeof(_ttl)))
      {
        m_errno = LASTERROR;
        DBG(DBG_ERROR, "%s: could not set IP_MULTICAST_TTL from socket (%d)\n", __FUNCTION__, m_errno);
        return false;
      }
      break;
    }
    case AF_INET6:
    {
      // The v6 multicast TTL socket option requires that the value be passed in as an integer
      if (setsockopt(m_socket, IPPROTO_IPV6, IPV6_MULTICAST_HOPS, (char*)&multicastTTL, sizeof(multicastTTL)))
      {
        m_errno = LASTERROR;
        DBG(DBG_ERROR, "%s: could not set IPV6_MULTICAST_HOPS from socket (%d)\n", __FUNCTION__, m_errno);
        return false;
      }
      break;
    }
    default:
      m_errno = EINVAL;
      DBG(DBG_ERROR, "%s: address familly unknown (%d)\n", __FUNCTION__, m_addr->sa_family());
      return false;
  }
  m_errno = 0;
  return true;
}

bool UdpServerSocket::SetMulticastMembership(const char* group, bool join)
{
  if (!IsValid())
    return false;

  switch(m_addr->sa_family())
  {
    case AF_INET:
    {
      struct ip_mreq mreq;
      if (inet_pton(AF_INET, group, &mreq.imr_multiaddr) == 0)
      {
        m_errno = LASTERROR;
        DBG(DBG_ERROR, "%s: invalid address (%d)\n", __FUNCTION__, m_errno);
        return false;
      }
      mreq.imr_interface.s_addr = htonl(INADDR_ANY);
      if (setsockopt(m_socket, IPPROTO_IP, join ? IP_ADD_MEMBERSHIP : IP_DROP_MEMBERSHIP, reinterpret_cast<const char*>(&mreq), sizeof(mreq)))
      {
        m_errno = LASTERROR;
        DBG(DBG_ERROR, "%s: could not set multicast membership from socket (%d)\n", __FUNCTION__, m_errno);
        return false;
      }
      break;
    }
    case AF_INET6:
    {
      struct ipv6_mreq mreq;
      if (inet_pton(AF_INET6, group, &mreq.ipv6mr_multiaddr) == 0)
      {
        m_errno = LASTERROR;
        DBG(DBG_ERROR, "%s: invalid address (%d)\n", __FUNCTION__, m_errno);
        return false;
      }
      mreq.ipv6mr_interface = 0;
      if (setsockopt(m_socket, IPPROTO_IPV6, join ? IPV6_JOIN_GROUP : IPV6_LEAVE_GROUP, reinterpret_cast<const char*>(&mreq), sizeof(mreq)))
      {
        m_errno = LASTERROR;
        DBG(DBG_ERROR, "%s: could not set multicast membership from socket (%d)\n", __FUNCTION__, m_errno);
        return false;
      }
      break;
    }
    default:
      m_errno = EINVAL;
      DBG(DBG_ERROR, "%s: address familly unknown (%d)\n", __FUNCTION__, m_addr->sa_family());
      return false;
  }
  m_errno = 0;
  return true;
}

size_t UdpServerSocket::AwaitIncoming(timeval timeout)
{
  if (IsValid())
  {
    m_errno = 0;

    if (!m_buffer && (m_buffer = new char[m_buflen]) == NULL)
    {
      m_errno = ENOMEM;
      DBG(DBG_ERROR, "%s: cannot allocate %u bytes for buffer\n", __FUNCTION__, m_buflen);
      return 0;
    }
    // reset buffer
    m_bufptr = m_buffer;
    m_rcvlen = 0;

    struct timeval tv = timeout;
    fd_set fds;
    int r = 0;

    FD_ZERO(&fds);
    FD_SET(m_socket, &fds);
    r = select(m_socket + 1, &fds, NULL, NULL, &tv);
    if (r > 0)
    {
      if ((r = recvfrom(m_socket, m_buffer, m_buflen, 0, m_from->sa(), &m_from->sa_len)) > 0)
      {
        m_rcvlen = r;
        if (m_rcvlen == m_buflen)
          DBG(DBG_WARN, "%s: datagram have been truncated (%d)\n", __FUNCTION__, r);
      }
    }
    if (r == 0)
    {
      m_errno = ETIMEDOUT;
      DBG(DBG_DEBUG, "%s: socket(%p) timed out\n", __FUNCTION__, &m_socket);
    }
    if (r < 0)
    {
      m_errno = LASTERROR;
      DBG(DBG_ERROR, "%s: socket(%p) read error (%d)\n", __FUNCTION__, &m_socket, m_errno);
    }
    return m_rcvlen;
  }
  m_errno = ENOTSOCK;
  return 0;
}

size_t UdpServerSocket::AwaitIncoming()
{
  return AwaitIncoming(m_timeout);
}

std::string UdpServerSocket::GetRemoteAddrInfo() const
{
  char host[INET6_ADDRSTRLEN];
  memset(host, 0, INET6_ADDRSTRLEN);
  getnameinfo(m_from->sa(), m_from->sa_len, host, sizeof(host), NULL, 0, NI_NUMERICHOST);
  return host;
}

size_t UdpServerSocket::ReadData(void* buf, size_t n)
{
  if (IsValid())
  {
    m_errno = 0;
    size_t len = 0;

    if (m_buffer && m_bufptr < m_buffer + m_rcvlen)
    {
      len = m_rcvlen - (m_bufptr - m_buffer);
      if (len > n)
        len = n;
      memcpy(buf, m_bufptr, len);
      m_bufptr += len;
    }
    return len;
  }
  m_errno = ENOTSOCK;
  return 0;
}
