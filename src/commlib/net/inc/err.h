// Copyright (c) 2013 zhou chang yue. All rights reserved.

#ifndef COMMLIB_NET_INC_ERR_H_
#define COMMLIB_NET_INC_ERR_H_

#include <errno.h>
#include <string>
#include <cstdio>
#include "commlib/public/inc/type.h"

namespace lib {
  namespace net {
    using std::string;

    class Err {
     public:
       // stream errors
       static const INT32 kERR_STREAM_CREATE_OBJ = -100300;
       static const INT32 kERR_STREAM_CREATE_SOCKET = -100301;
       static const INT32 kERR_STREAM_BIND_LISTEN = -100302;
       static const INT32 kERR_STREAM_RECV_TIMEOUT = -100303;
       static const INT32 kERR_STREAM_SOCKET_CLOSED = -100304;
       static const INT32 kERR_STREAM_SOCKET_ERROR = -100305;
       static const INT32 kERR_STREAM_NO_NEW_HANDLE = -100306;
       static const INT32 kERR_STREAM_PARAM_ERROR = -100307;

       static const INT32 kERR_DGRAM_CREATE_OBJ = -100310;
       static const INT32 kERR_DGRAM_CREATE_SOCKET = -100311;
       static const INT32 kERR_DGRAM_BIND_LISTEN = -100312;

       // Socket errors
       static const INT32 kERR_SOCKET_CONNECTING = -100370;
       static const INT32 kERR_SOCKET_SEND = -100380;
       static const INT32 kERR_SOCKET_RECV = -100381;
       static const INT32 kERR_SOCKET_REUSED = -100382;
       static const INT32 kERR_SOCKET_NBLOCK = -100383;
       static const INT32 kERR_SOCKET_RCV_SIZE = -100384;
       static const INT32 kERR_SOCKET_SND_SIZE = -100385;
       static const INT32 kERR_SOCKET_SET_OPTION = -100386;
       static const INT32 kERR_SOCKET_GET_OPTION = -100387;
       static const INT32 kERR_SOCKET_BIND = -100388;
       static const INT32 kERR_SOCKET_LISTEN = -100389;
       static const INT32 kERR_SOCKET_CONNECT = -100390;
       static const INT32 kERR_SOCKET_CLOSE = -100391;
       static const INT32 kERR_SOCKET_CLOSE_RECV = -100392;
       static const INT32 kERR_SOCKET_CLOSE_SEND = -100393;
       static const INT32 kERR_SOCKET_READ_TIMEOUT = -100914;
       static const INT32 kERR_SOCKET_READ_CLOSED = -100395;
       static const INT32 kERR_SOCKET_READ_RESP = -100396;
       static const INT32 kERR_SOCKET_INVALID_HANDLE = -100397;
       static const INT32 kERR_SOCKET_CREATE = -100398;
       static const INT32 kERR_SOCKET_GET_READABLE_SIZE = -100399;
       static const INT32 kERR_SOCKET_SET_RECEIVE_BUF = -100400;
       static const INT32 kERR_SOCKET_SET_SEND_BUF = -100401;

       //
       static const INT32 kERR_SOCKET_POOL_ADDR_INVALID = -100310;
       static const INT32 kERR_SOCKET_POOL_ALLOCATE_INVALID = -100311;
       static const INT32 kERR_SOCKET_POOL_MIN_LARGER_MAX = -100312;
       static const INT32 kERR_SOCKET_POOL_CREATE_SOCK_FAILED = -100313;
       static const INT32 kERR_SOCKET_POOL_CONNECT_FAILED = -100314;
       static const INT32 kERR_SOCKET_POOL_HAS_NO_VALID_SOCK = -100315;

       static const INT32 kERR_EPOLL_CREATE_EVENTS_POOL_FAILED = -100320;
       static const INT32 kERR_EPOLL_CREATE_FD_FAILED = -100321;
       static const INT32 kERR_EPOLL_CTRL_FAILED = -100322;
    };
  }  // namespace net
}  // namespace lib

#endif  // COMMLIB_NET_INC_ERR_H_
