// Copyright (c) 2013 zhou chang yue. All rights reserved.

#include <unistd.h>
#include <sys/ioctl.h>
#include "commlib/net/inc/sock.h"

namespace lib {
  namespace net {
    INT32 Sock::Read(CHAR * in, INT32 max_size, INT32 timeout) {
      if (INVALID_SOCKET_HANDLE == handle_) {
        LIB_NET_LOG_WARN("sock is invalid");
        return Err::kERR_SOCKET_INVALID_HANDLE;
      }

      if (NO_WAIT != timeout) {
        if (0 >= WaitForReady(timeout, EVENT_READ)) {
          LIB_NET_LOG_WARN("sock read timeout ......");

          return -1;
        }
      }

      return ::recv(handle_, in, max_size, 0);
    }

    INT32 Sock::WaitForReady(INT32 timeout, INT32 flag) {
      fd_set fd_read;
      fd_set fd_write;
      fd_set fd_except;

      FD_ZERO(&fd_read);
      FD_SET(handle_, &fd_read);
      FD_ZERO(&fd_write);
      FD_SET(handle_, &fd_write);
      FD_ZERO(&fd_except);
      FD_SET(handle_, &fd_except);

      struct timeval tm;
      tm.tv_sec = timeout/1000;
      tm.tv_usec = (timeout%1000)*1000;

      struct timeval * ptm = &tm;
      if (INFINITE == timeout) {
        ptm = NULL;
      }

      if (EVENT_READ == flag) {
        return ::select(handle_ + 1, &fd_read, NULL, &fd_except, ptm);
      } else if (EVENT_WRITE == flag) {
        return ::select(handle_ + 1, NULL, &fd_write, &fd_except, ptm);
      }

      return ::select(handle_ + 1, &fd_read, &fd_write, &fd_except, ptm);
    }

    INT32 Sock::Write(const CHAR * out, INT32 size) {
      if (INVALID_SOCKET_HANDLE == handle_) {
        return Err::kERR_SOCKET_INVALID_HANDLE;
      }

      INT32 write_size = 0;
      INT32 write_total_size = 0;
      while (0 < (write_size = ::send(handle_, out + write_total_size, size - write_total_size, 0))) {
        write_total_size += write_size;
        if (write_total_size == size) {
          break;
        }

        if (write_total_size > size) {
          return -1;
        }
      }

      return write_total_size;
    }

    INT32 Sock::SetReused() {
      INT32 flag = 1;
      if (0 != ::setsockopt(handle_, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag))) {
        return Err::kERR_SOCKET_REUSED;
      }

      return 0;
    }

    INT32 Sock::SetNBlock() {
      INT32 flag;
      flag = ::fcntl(handle_, F_GETFL, 0);
      flag |= O_NONBLOCK;
      flag |= O_NDELAY;

      if (0 != ::fcntl(handle_, F_SETFL, flag)) {
        return Err::kERR_SOCKET_NBLOCK;
      }

      return 0;
    }

    INT32 Sock::IsNBlock() {
      INT32 flag;
      flag = ::fcntl(handle_, F_GETFL, 0);

      return (0 != (flag & O_NONBLOCK));
    }

    INT32 Sock::SetReceiveBufSize(INT32 size) {
      if (0 != ::setsockopt(handle_, SOL_SOCKET, SO_RCVBUF, &size, sizeof(size))) {
        return Err::kERR_SOCKET_SET_RECEIVE_BUF;
      }

      return 0;
    }

    INT32 Sock::SetSendBufSize(INT32 size) {
      if (0 != ::setsockopt(handle_, SOL_SOCKET, SO_SNDBUF, &size, sizeof(size))) {
        return Err::kERR_SOCKET_SET_RECEIVE_BUF;
      }

      return 0;
    }

    INT32 Sock::SetNBlock(SOCKET h) {
      INT32 flag;
      flag = ::fcntl(h, F_GETFL, 0);
      flag |= O_NONBLOCK;
      flag |= O_NDELAY;

      if (0 != ::fcntl(h, F_SETFL, flag)) {
        return Err::kERR_SOCKET_NBLOCK;
      }

      return 0;
    }

    VOID Sock::SetHandle(SOCKET h) {
      if (0 >= h) {
        return;
      }

      handle_ = h;
    }

    INT32 Sock::Close() {
      if (INVALID_SOCKET_HANDLE != handle_) {
        INT32 result = ::close(handle_);
        if (0 != result) {
          LIB_NET_LOG_WARN("sock close handle failed:" << ::strerror(errno));
        }
        handle_ = INVALID_SOCKET_HANDLE;
      }

      return 0;
    }

    INT32 Sock::GetReadableSize() {
      INT32 size = 0;
      if (INVALID_SOCKET_HANDLE == handle_) {
        return 0;
      }

      if (-1 == ::ioctl(handle_, FIONREAD, &size)) {
        return Err::kERR_SOCKET_GET_READABLE_SIZE;
      }

      return size;
    }

    BOOL Sock::IsAlive() {
      CHAR in[1024] = {0};
      INT32 size = sizeof(in);

      INT32 result = ::recv(handle_, in, size, MSG_PEEK);
      if (0 < result) {
        return TRUE;
      }

      if (0 == result) {
        if (11 == errno || 104 == errno || 9 == errno) {
          return FALSE;
        }

        return TRUE;
      }

      return FALSE;
    }

    INT32 Sock::Connect(const string & addr, INT32 timeout) {
      return 0;
    }

    INT32 Sock::ConnectAsync(const string & addr) {
      return 0;
    }

    VOID Sock::Destroy() {
      Close();
    }

    INT32 SockStream::Create() {
      if (INVALID_SOCKET_HANDLE == (handle_ = ::socket(AF_INET, SOCK_STREAM, 0))) {
        LIB_NET_LOG_ERROR("create socket stream failed " << ::strerror(errno));
        return INVALID_SOCKET_HANDLE;
      }

      return 0;
    }

    INT32 SockStream::Connect(const string & addr, INT32 timeout) {
      NetAddr n_addr;
      n_addr.Serialize(addr);

      struct sockaddr_in svr;
      ::memset(&svr, 0, sizeof(svr));
      svr.sin_family = AF_INET;
      svr.sin_port = htons(n_addr.GetPort());
      svr.sin_addr.s_addr = inet_addr(n_addr.GetIp().c_str());

      if (NO_WAIT != timeout) {
        if (0 != SetNBlock(handle_)) {
          LIB_NET_LOG_ERROR("sock set nonblocking failed");

          return -1;
        }
      }

      INT32 result = ::connect(handle_, (struct sockaddr *)&svr, sizeof(svr));
      if ( result < 0) {
        if (EINPROGRESS == errno) {
          if (0 >= WaitForReady(timeout, EVENT_WRITE)) {
            LIB_NET_LOG_WARN("sock  connect to ip:" << n_addr.GetIp().c_str() << " port: " << n_addr.GetPort() << " failed:" << ::strerror(errno));
            return -1;
          }
        } else {
          LIB_NET_LOG_WARN("sock  connect to ip:" << n_addr.GetIp().c_str() << " port: " << n_addr.GetPort() << " failed:" << ::strerror(errno));

          return -1;
        }
      }

      return 0;
    }

    INT32 SockStream::ConnectAsync(const string & addr) {
      if (0 != SetNBlock(handle_)) {
        LIB_NET_LOG_ERROR("sock set nonblocking failed");

        return -1;
      }

      NetAddr n_addr;
      n_addr.Serialize(addr);

      struct sockaddr_in svr;
      ::memset(&svr, 0, sizeof(svr));
      svr.sin_family = AF_INET;
      svr.sin_port = htons(n_addr.GetPort());
      svr.sin_addr.s_addr = inet_addr(n_addr.GetIp().c_str());

      INT32 result = ::connect(handle_, (struct sockaddr *) &svr, sizeof(svr));
      if (0 == result) {
        return 0;
      }

      if (0 > result) {
        if (EINPROGRESS == errno) {
          return Err::kERR_SOCKET_CONNECTING;
        }
      }

      return Err::kERR_SOCKET_CONNECT;
    }

    INT32 SockStream::Listen(const NetAddr & local, INT32 backlog) {
      INT32 result = Create();
      if (0 != result) {
        return result;
      }

      result = SetReused();
      if (0 != result) {
        return result;
      }

      result = SetNBlock();
      if (0 != result) {
        return result;
      }

      struct sockaddr_in svr_addr;
      ::memset(&svr_addr, 0x00, sizeof(svr_addr));
      svr_addr.sin_family = AF_INET;
      svr_addr.sin_port = htons(local.GetPort());
      svr_addr.sin_addr.s_addr = ::inet_addr(local.GetIp().c_str());
      if (0 != ::bind(handle_, reinterpret_cast<sockaddr *>(&svr_addr), sizeof(svr_addr))) {
        return Err::kERR_SOCKET_BIND;
      }

      if (0 != ::listen(handle_, backlog)) {
        return Err::kERR_SOCKET_LISTEN;
      }

      return 0;
    }

    INT32 SockStream::Accept(NetAddr * remote) {
      struct sockaddr_in addr;
      ::memset(&addr, 0x00, sizeof(addr));
      SOCKLEN_T addr_len = sizeof(addr);

      INT32 fd = ::accept(handle_, reinterpret_cast<sockaddr *>(&addr), &addr_len);
      if (0 >= fd) {
        return fd;
      }

      INT32 result = SetNBlock(fd);
      if (0 != result) {
        return result;
      }

      remote->SetIp(&addr.sin_addr);
      remote->SetPort(ntohs(addr.sin_port));

      return fd;
    }

    INT32 SockStream::WriteByAddr(const string & addr, const CHAR * out, INT32 size, CHAR * in, INT32 max_size, INT32 timeout) {
      INT32 result = 0;
      if (0 != (result = Create())) {
        LIB_NET_LOG_ERROR("sock write by addr:" << addr << " create socket failed");
        return result;
      }

      if (0 != (result = Connect(addr, timeout))) {
        LIB_NET_LOG_ERROR("sock write by addr:" << addr << " connect service failed");
        return result;
      }

      if (0 >= (result = Sock::Write(out, size))) {
        LIB_NET_LOG_ERROR("sock write by addr:" << addr << " write data failed size:" << size);
        return result;
      }

      result = Sock::Read(in, max_size, timeout);

      Sock::Close();

      return result;
    }

    INT32 UnixStream::Accept(NetAddr * remote) {
      struct sockaddr_un addr;
      ::memset(&addr, 0x00, sizeof(addr));
      SOCKLEN_T addr_len = sizeof(addr);

      INT32 fd = ::accept(handle_, reinterpret_cast<sockaddr *>(&addr), &addr_len);
      if (0 >= fd) {
        return fd;
      }

      INT32 result = SetNBlock(fd);
      if (0 != result) {
        return result;
      }

      remote->SetPath(addr.sun_path);

      return fd;
    }

    INT32 UnixStream::Listen(const NetAddr & local, INT32 backlog) {
      INT32 result = Create();
      if (0 != result) {
        return result;
      }

      result = SetNBlock();
      if (0 != result) {
        return result;
      }

      ::unlink(local.GetPath().c_str());

      struct sockaddr_un svr_addr;
      ::memset(&svr_addr, 0x00, sizeof(svr_addr));
      svr_addr.sun_family = AF_UNIX;
      ::snprintf(svr_addr.sun_path, sizeof(svr_addr.sun_path)-1, "%s", local.GetPath().c_str());
      if (0 != ::bind(handle_, reinterpret_cast<sockaddr *>(&svr_addr), sizeof(svr_addr))) {
        LIB_NET_LOG_ERROR("bind failed addr:" << local.GetPath() <<  " msg:" << ::strerror(errno));
        return Err::kERR_SOCKET_BIND;
      }

      if (0 != ::listen(handle_, backlog)) {
        return Err::kERR_SOCKET_LISTEN;
      }

      return 0;
    }

    INT32 SockDGram::Create() {
      if (INVALID_SOCKET_HANDLE == (handle_ = ::socket(AF_INET, SOCK_DGRAM, 0))) {
        LIB_NET_LOG_ERROR("create socket dgram failed " << ::strerror(errno));
        return INVALID_SOCKET_HANDLE;
      }

      return 0;
    }

    INT32 SockDGram::ReadFrom(CHAR * in, INT32 max_size, NetAddr * net_addr, INT32 timeout) {
      fd_set fd_read;

      FD_ZERO(&fd_read);
      FD_SET(handle_, &fd_read);

      struct timeval tm;
      tm.tv_sec = timeout/1000;
      tm.tv_usec = (timeout%1000)*1000;

      struct timeval * ptm = &tm;
      if (INFINITE == timeout) {
        ptm = NULL;
      }

      if (0 >= ::select(handle_ + 1, &fd_read, NULL, NULL, ptm)) {
        LIB_NET_LOG_WARN("sock dgram reading timeout:" << timeout);
        return Err::kERR_SOCKET_READ_TIMEOUT;
      }

      struct sockaddr_in addr;
      ::memset(&addr, 0x00, sizeof(addr));
      SOCKLEN_T addr_len = sizeof(addr);
      INT32 size = ::recvfrom(handle_, in, max_size, MSG_TRUNC, reinterpret_cast<sockaddr *>(&addr), &addr_len);

      net_addr->SetIp(&addr.sin_addr);
      net_addr->SetPort(ntohs(addr.sin_port));

      return size;
    }

    INT32 SockDGram::WriteTo(const CHAR * out, INT32 size, const NetAddr * net_addr) {
      struct sockaddr_in addr;
      addr.sin_family = AF_INET;
      addr.sin_port = htons(net_addr->GetPort());
      addr.sin_addr.s_addr = ::inet_addr(net_addr->GetIp().c_str());

      return ::sendto(handle_, out, size, 0, reinterpret_cast<sockaddr *>(&addr), sizeof(addr));;
    }

    INT32 SockDGram::Listen(const NetAddr & local) {
      INT32 result = Create();
      if (0 != result) {
        return result;
      }

      result = SetReused();
      if (0 != result) {
        return result;
      }

      result = SetNBlock();
      if (0 != result) {
        return result;
      }

      struct sockaddr_in svr_addr;
      ::memset(&svr_addr, 0x00, sizeof(svr_addr));
      svr_addr.sin_family = AF_INET;
      svr_addr.sin_port = htons(local.GetPort());
      svr_addr.sin_addr.s_addr = ::inet_addr(local.GetIp().c_str());
      if (0 != ::bind(handle_, reinterpret_cast<sockaddr *>(&svr_addr), sizeof(svr_addr))) {
        return Err::kERR_SOCKET_BIND;
      }

      return 0;
    }

    INT32 UnixStream::Create() {
      if (INVALID_SOCKET_HANDLE == (handle_ = ::socket(AF_UNIX, SOCK_STREAM, 0))) {
        LIB_NET_LOG_ERROR("create unix stream failed " << ::strerror(errno));
        return INVALID_SOCKET_HANDLE;
      }

      return 0;
    }

    INT32 UnixDGram::Create() {
      if (INVALID_SOCKET_HANDLE == (handle_ = ::socket(AF_UNIX, SOCK_DGRAM, 0))) {
        LIB_NET_LOG_ERROR("create unix dgram failed " << ::strerror(errno));
        return INVALID_SOCKET_HANDLE;
      }

      return 0;
    }

    INT32 UnixDGram::ReadFrom(CHAR * in, INT32 max_size, NetAddr * net_addr, INT32 timeout) {
      fd_set fd_read;

      FD_ZERO(&fd_read);
      FD_SET(handle_, &fd_read);

      struct timeval tm;
      tm.tv_sec = timeout/1000;
      tm.tv_usec = (timeout%1000)*1000;

      struct timeval * ptm = &tm;
      if (INFINITE == timeout) {
        ptm = NULL;
      }

      if (0 >= ::select(handle_ + 1, &fd_read, NULL, NULL, ptm)) {
        LIB_NET_LOG_WARN("unix dgram reading timeout:" << timeout);
        return Err::kERR_SOCKET_READ_TIMEOUT;
      }

      struct sockaddr_un addr;
      ::memset(&addr, 0x00, sizeof(addr));
      SOCKLEN_T addr_len = sizeof(addr);
      INT32 size = ::recvfrom(handle_, in, max_size, MSG_TRUNC, reinterpret_cast<sockaddr *>(&addr), &addr_len);

      net_addr->SetPath(addr.sun_path);

      return size;
    }

    INT32 UnixDGram::WriteTo(const CHAR * out, INT32 size, const NetAddr * net_addr) {
      INT32 written = 0;

      struct sockaddr_un addr;
      addr.sun_family = AF_UNIX;
      ::snprintf(addr.sun_path, sizeof(addr.sun_path)-1, "%s", net_addr->GetPath().c_str());

      while (written < size) {
        INT32 once_written = ::sendto(handle_, out + written, size - written, 0,
                                      reinterpret_cast<sockaddr *>(&addr), sizeof(addr));
        if (0 > once_written) {
          LIB_NET_LOG_WARN("unix dgram send data failed socket:" << handle_ << " err msg:" << ::strerror(errno));
          return Err::kERR_SOCKET_SEND;
        }
        written += once_written;
      }

      return 0;
    }

    INT32 UnixDGram::Listen(const NetAddr & local) {
      INT32 result = Create();
      if (0 != result) {
        return result;
      }

      result = SetNBlock();
      if (0 != result) {
        return result;
      }

      ::unlink(local.GetPath().c_str());

      struct sockaddr_un svr_addr;
      ::memset(&svr_addr, 0x00, sizeof(svr_addr));
      svr_addr.sun_family = AF_UNIX;
      ::snprintf(svr_addr.sun_path, sizeof(svr_addr.sun_path)-1, "%s", local.GetPath().c_str());
      if (0 != ::bind(handle_, reinterpret_cast<sockaddr *>(&svr_addr), sizeof(svr_addr))) {
        return Err::kERR_SOCKET_BIND;
      }

      return 0;
    }
  }  // namespace net
}  // namespace lib
