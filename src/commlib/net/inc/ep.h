// Copyright (c) 2013 zhou chang yue. All rights reserved.

#ifndef COMMLIB_NET_AP_INC_EP_H_
#define COMMLIB_NET_AP_INC_EP_H_

#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <stdlib.h>
#include <string>
#include <map>
#include "commlib/public/inc/type.h"
#include "commlib/net/inc/err.h"
#include "commlib/net/inc/log.h"

namespace lib {
  namespace net {
    using std::pair;
    using std::make_pair;

    class EP {
     public:
       typedef pair<INT32, epoll_event *> EPResult;

     public:
       static const INT32 DEFAULT_MAX_FD = 1000000;

     public:
       EP():max_fd_(DEFAULT_MAX_FD), epoll_fd_(-1), events_(NULL) {}
       ~EP() {
         if (events_) {
           ::free(events_);
         }
       }

     public:
       INT32 Create(INT32 max_fd = DEFAULT_MAX_FD);
       EPResult Wait(INT32 timeout = 1000);

     public:
       INT32 Add(INT32 fd, INT32 flag, INT32 ext = 0);
       INT32 Mod(INT32 fd, INT32 flag, INT32 ext = 0);
       INT32 Del(INT32 fd, INT32 flag, INT32 ext = 0);

     private:
       INT32 Ctrl(INT32 fd, INT32 flag, INT32 action, INT64 ext = 0);

     private:
       SIZE_T max_fd_;
       INT32 epoll_fd_;
       epoll_event * events_;
    };
  }  // namespace net
}  // namespace lib

#endif  // COMMLIB_NET_AP_INC_EP_H_