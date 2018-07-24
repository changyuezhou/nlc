// Copyright (c) 2013 zhou chang yue. All rights reserved.

#ifndef COMMLIB_NET_INC_SOCK_H_
#define COMMLIB_NET_INC_SOCK_H_

#include <fcntl.h>
#include <sys/socket.h>
#include <string>
#include "commlib/public/inc/type.h"
#include "commlib/net/inc/net_addr.h"
#include "commlib/net/inc/err.h"
#include "commlib/net/inc/log.h"

namespace lib {
  namespace net {
    class Sock {
     public:
       enum SOCKET_TYPE {
         NONE = 0,
         TCP = 1,
         UDP = 2,
         UNIX_STREAM = 3,
         UNIX_DGRAM = 4,
         STREAM_ACCEPTOR = 5,
         DGRAM_ACCEPTOR = 6
       };

     public:
       enum {
         NO_WAIT = 0,
         INFINITE = -1
       };

       enum {
         EVENT_READ = 1,
         EVENT_WRITE = 2,
         EVENT_READ_WRITE = 3
       };

     public:
       Sock():handle_(INVALID_SOCKET_HANDLE) {}
       explicit Sock(SOCKET h):handle_(h) {
         if (0 >= h) {
           handle_ = INVALID_SOCKET_HANDLE;
         }
       }
      virtual ~Sock() { Destroy(); }

     public:
       virtual INT32 Create() { return 0; }
       virtual SOCKET_TYPE Type() { return NONE; }

     public:
       virtual INT32 Read(CHAR * in, INT32 max_size, INT32 timeout = NO_WAIT);
       virtual INT32 Write(const CHAR * out, INT32 size);
       virtual INT32 SetReused();
       virtual INT32 Connect(const string & addr, INT32 timeout = NO_WAIT);
       virtual INT32 ConnectAsync(const string & addr);
       virtual INT32 WriteByAddr(const string & addr, const CHAR * out, INT32 size, CHAR * in, INT32 max_size, INT32 timeout = NO_WAIT) { return 0; }

     public:
       SOCKET Handle() { return handle_; }
       VOID SetHandle(SOCKET h);
       INT32 SetReceiveBufSize(INT32 size);
       INT32 SetSendBufSize(INT32 size);
       INT32 SetNBlock();
       INT32 SetNBlock(SOCKET h);
       INT32 Close();
       INT32 GetReadableSize();
       INT32 WaitForReady(INT32 timeout, INT32 flag);
       INT32 IsValid() { return handle_ != INVALID_SOCKET_HANDLE; }
       INT32 IsNBlock();
       BOOL IsAlive();

     public:
       VOID Destroy();

     protected:
       SOCKET handle_;

     private:
       const Sock & operator=(const Sock &);
       Sock(const Sock &);
    };

    class SockStream: public Sock {
     public:
       SockStream(): Sock() {}
       explicit SockStream(SOCKET h):Sock(h) {}
      virtual ~SockStream() { Destroy(); }

     public:
       virtual INT32 Create();
       virtual SOCKET_TYPE Type() { return TCP; }
       virtual INT32 Accept(NetAddr * remote);
       virtual INT32 Listen(const NetAddr & local, INT32 backlog = 100);
       virtual INT32 Connect(const string & addr, INT32 timeout = NO_WAIT);
       virtual INT32 ConnectAsync(const string & addr);
       virtual INT32 WriteByAddr(const string & addr, const CHAR * out, INT32 size, CHAR * in, INT32 max_size, INT32 timeout = NO_WAIT);

    public:
       VOID Destroy() { Sock::Destroy(); }

     private:
       const SockStream & operator=(const SockStream &);
       SockStream(const SockStream &);
    };

    class UnixStream:public SockStream {
     public:
       UnixStream() {}
       explicit UnixStream(SOCKET h):SockStream(h) {}
       virtual ~UnixStream() { Destroy(); }

     public:
       virtual INT32 Create();
       virtual INT32 SetReused() { return 0; }
       virtual SOCKET_TYPE Type() { return UNIX_STREAM; }
       virtual INT32 Accept(NetAddr * remote);
       virtual INT32 Listen(const NetAddr & local, INT32 backlog = 100);

     public:
       VOID Destroy() { SockStream::Destroy(); }

     private:
       const UnixStream & operator=(const UnixStream);
       UnixStream(const UnixStream &);
    };

    class SockDGram: public Sock {
     public:
       SockDGram() : Sock() {}
       explicit SockDGram(SOCKET h):Sock(h) {}
      virtual ~SockDGram() { Destroy(); }

     public:
       virtual INT32 Create();
       virtual INT32 ReadFrom(CHAR * in, INT32 max_size, NetAddr * net_addr, INT32 timeout = NO_WAIT);
       virtual INT32 WriteTo(const CHAR * out, INT32 size, const NetAddr * net_addr);
       virtual SOCKET_TYPE Type() { return UDP; }
       virtual INT32 Listen(const NetAddr & local);

     public:
       VOID Destroy() { Sock::Destroy(); }

     private:
       const SockDGram & operator=(const SockDGram &);
       SockDGram(const SockDGram &);
    };

    class UnixDGram:public SockDGram {
     public:
       UnixDGram() {}
       explicit UnixDGram(SOCKET h):SockDGram(h) {}
       virtual ~UnixDGram() { Destroy(); }

     public:
       virtual INT32 Create();
       virtual INT32 SetReused() { return 0; }
       virtual SOCKET_TYPE Type() { return UNIX_DGRAM; }
       virtual INT32 Listen(const NetAddr & local);

     public:
       virtual INT32 ReadFrom(CHAR * in, INT32 max_size, NetAddr * net_addr, INT32 timeout = NO_WAIT);
       virtual INT32 WriteTo(const CHAR * out, INT32 size, const NetAddr * net_addr);

     public:
       VOID Destroy() { SockDGram::Destroy(); }

     private:
       const UnixDGram & operator=(const UnixDGram);
       UnixDGram(const UnixDGram &);
    };
  }  // namespace net
}  // namespace lib

#endif  // COMMLIB_NET_INC_SOCK_H_
