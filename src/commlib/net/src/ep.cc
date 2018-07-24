// Copyright (c) 2013 zhou chang yue. All rights reserved.

#include "commlib/net/inc/ep.h"

namespace lib {
  namespace net {
    INT32 EP::Create(INT32 max_fd) {
      events_ = reinterpret_cast<struct epoll_event *>(::malloc(max_fd*sizeof(struct epoll_event)));
      if (NULL == events_) {
        LIB_NET_LOG_ERROR("epoll allocate event handle objs failed size:" << max_fd);
        return Err::kERR_EPOLL_CREATE_EVENTS_POOL_FAILED;
      }

      epoll_fd_ = ::epoll_create(max_fd);
      if (0 >= epoll_fd_) {
        LIB_NET_LOG_ERROR("epoll create max fd failed msg:" << ::strerror(errno));
        return Err::kERR_EPOLL_CREATE_FD_FAILED;
      }

      max_fd_ = max_fd;

      return 0;
    }

    EP::EPResult EP::Wait(INT32 timeout) {
      INT32 ready = ::epoll_wait(epoll_fd_, events_, max_fd_, timeout);

      return make_pair(ready, events_);
    }

    INT32 EP::Ctrl(INT32 fd, INT32 flag, INT32 action, INT64 ext) {
      struct epoll_event ev;
      ::memset(&ev, 0x00, sizeof(ev));
      ev.data.fd = fd;
      ev.data.u64 = ext;
      ev.events = flag;

      if (0 > ::epoll_ctl(epoll_fd_, action, fd, &ev)) {
        LIB_NET_LOG_ERROR("epoll ctrl failed socket " << fd \
                              << " action " << action << " errmsg:" << ::strerror(errno));
        return Err::kERR_EPOLL_CTRL_FAILED;
      }

      return 0;
    }

    INT32 EP::Add(INT32 fd, INT32 flag, INT32 ext) {
      LIB_NET_LOG_DEBUG("epoll register " << fd << " to pool flag " << flag << " ext " << ext);

      return Ctrl(fd, flag, EPOLL_CTL_ADD, ext);
    }

    INT32 EP::Mod(INT32 fd, INT32 flag, INT32 ext) {
      LIB_NET_LOG_DEBUG("epoll mod " << fd << " from pool flag " << flag << " ext " << ext);

      return Ctrl(fd, flag, EPOLL_CTL_MOD, ext);
    }

    INT32 EP::Del(INT32 fd, INT32 flag, INT32 ext) {
      LIB_NET_LOG_DEBUG("epoll del " << fd << " from pool flag " << flag << " ext " << ext);

      return Ctrl(fd, flag, EPOLL_CTL_DEL, ext);
    }
  }  // namespace net
}  // namespace lib