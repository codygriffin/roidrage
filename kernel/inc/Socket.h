//------------------------------------------------------------------------------
//
// Copyright (C) 2013 Cody Griffin (cody.m.griffin@gmail.com)
// 
//------------------------------------------------------------------------------

#ifndef INCLUDED_SOCKET_H
#define INCLUDED_SOCKET_H

#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sstream>

//------------------------------------------------------------------------------

struct ErrnoException : public std::exception {
  ErrnoException(std::string context, int fd, int err) 
    : context_  (context) 
#include "Socket.h"
    , fd_       (fd) 
    , errno_    (err) 
    , message_  () {
    std::stringstream msg;
    msg << context_;
    if (fd >= 0) {
      msg << "[fd = " << fd_ << "] ";
    }
    msg <<  std::string(strerror(errno_));
    message_ = msg.str();
  }

  virtual const char* what() const noexcept (true)  {
    return message_.c_str();
  }

  std::string context_;
  int         fd_;
  int         errno_;
  std::string message_;
};

//------------------------------------------------------------------------------

#define SOCKET_THROW   throw ErrnoException(std::string("Socket::") + __FUNCTION__, *this, errno);
#define ERRNO_THROW(c) throw ErrnoException(std::string( #c "::")   + __FUNCTION__, -1,    errno);

//------------------------------------------------------------------------------

namespace corvid {

// Socket Domains, Types and Protocols
struct Unix         { static const int domain = AF_UNIX;       };
struct Inet         { static const int domain = AF_INET;       };
struct Inet6        { static const int domain = AF_INET6;      };

struct Stream       { static const int type   = SOCK_STREAM;   };
struct Dgram        { static const int type   = SOCK_DGRAM;    };
struct Raw          { static const int type   = SOCK_RAW;      };

struct NullProtocol { static const int proto  = 0;             };

//------------------------------------------------------------------------------

template <typename Domain>
struct Addr;

//------------------------------------------------------------------------------

template <>
struct Addr<Inet> {
  Addr(const Addr& other) 
    : addr  (other.addr)
    , length(other.length) {
  }

  Addr(const std::string& hostname, uint16_t port) {
    addr.sin_family = Inet::domain;
    addr.sin_port   = htons(port);
    length = sizeof(addr);
    if(::inet_pton(Inet::domain, hostname.c_str(), &addr.sin_addr) < 0) {
      ERRNO_THROW("Addr<Inet>");
    }
  }

  sockaddr_in addr;
  socklen_t   length;
};

//------------------------------------------------------------------------------

struct FileDescriptor {
  FileDescriptor(int fd) 
    : fd_ (fd) {
  }

  virtual ~FileDescriptor() {
    close(fd_);
  }
  
  operator int() {
    return fd_;
  }

private:
  int fd_;  
};

//------------------------------------------------------------------------------

template <typename Domain, typename Type, typename Protocol = NullProtocol>
struct Socket : public FileDescriptor {
  typedef std::vector<uint8_t> Buffer;
  Socket() 
  : FileDescriptor(socket(Domain::domain, Type::type | O_NONBLOCK, Protocol::proto)) {
    if (*this < 0) {
      SOCKET_THROW;
    }
  }

  Socket(const Socket& other) 
  : FileDescriptor(other) {
  }

  Socket accept() {
    int fd = ::accept(*this, 0, 0);
    if (fd < 0) {
      SOCKET_THROW;
    }
    return Socket(fd);
  }

  void bind(const Addr<Domain>& addr, bool reuse = false) {
    const int reuseFlag = reuse?1:0;
    if (::setsockopt(*this, SOL_SOCKET, SO_REUSEADDR, &reuseFlag, sizeof(reuseFlag)) < 0) { 
      SOCKET_THROW;
    }

    if (::bind(*this, reinterpret_cast<const sockaddr*>(&addr.addr), addr.length) < 0) {
      SOCKET_THROW;
    }
  }

  void connect(const Addr<Domain>& addr) {
    if (::connect(*this, reinterpret_cast<const sockaddr*>(&addr.addr), addr.length) < 0) {
      SOCKET_THROW;
    }
  }

  void listen(int backlog = 0) {
    if (::listen(*this, backlog) < 0) {
      SOCKET_THROW;
    }
  }

  bool 
  send(Buffer::iterator& begin,  
       Buffer::iterator& end) {
    ssize_t bytesSent = ::send(*this, &(*begin), end-begin, MSG_NOSIGNAL);
    bool again = errno == EAGAIN;
    if (bytesSent < 0 && !again) {
      SOCKET_THROW;
    }
    std::advance(begin, bytesSent);

    return bytesSent == 0 && !again;
  }

  bool 
  recv(Buffer::iterator& begin,  
       Buffer::iterator& end) {
    ssize_t bytesRcvd = ::recv(*this, &(*begin), end-begin, 0);
    bool again = errno == EAGAIN;
    if (bytesRcvd < 0 && !again) {
      SOCKET_THROW;
    }

    std::advance(begin, bytesRcvd);

    return bytesRcvd == 0 && !again;
  }

  ssize_t send(const std::string& buffer) {
    ssize_t bytesSent = ::send(*this, &buffer[0], buffer.length(), MSG_NOSIGNAL);
    if (bytesSent < 0 && errno != EAGAIN) {
      SOCKET_THROW;
    }

    return bytesSent;
  }

  ssize_t recv(std::string& buffer) {
    ssize_t bytesRcvd = ::recv(*this, &buffer[0], buffer.length(), 0);
    if (bytesRcvd < 0 && errno != EAGAIN) {
      SOCKET_THROW;
    }

    return bytesRcvd;
  }

private:
  Socket(int fd) 
  : FileDescriptor(fd) {
    int flags;
    flags = fcntl (*this, F_GETFL, 0);
    if (flags == -1) {
      SOCKET_THROW;
    }

    flags |= O_NONBLOCK;
    if (fcntl (*this, F_SETFL, flags) == -1) {
      SOCKET_THROW;
    }
  }
};

// Socket types
typedef Socket<Inet, Stream> TcpSocket;
typedef Socket<Inet, Dgram>  UdpSocket;

} // namespace corvid

//------------------------------------------------------------------------------

#endif
