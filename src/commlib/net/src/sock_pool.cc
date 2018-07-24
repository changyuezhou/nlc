// Copyright (c) 2013 zhou chang yue. All rights reserved.

#include <unistd.h>
#include <sys/ioctl.h>
#include "commlib/net/inc/sock_pool.h"
#include "commlib/net/inc/log.h"
#include "commlib/net/inc/err.h"
#include "commlib/magic/inc/timeFormat.h"

namespace lib {
  namespace net {
    using lib::magic::TimeFormat;

    INT32 SockPool::Initial() {
      if (0 >= addr_.length()) {
        return Err::kERR_SOCKET_POOL_ADDR_INVALID;
      }

      if (min_ > max_) {
        return Err::kERR_SOCKET_POOL_MIN_LARGER_MAX;
      }

      if (need_listen_) {
        ep_ = new EP();
        if (NULL == ep_) {
          return Err::kERR_SOCKET_POOL_ALLOCATE_INVALID;
        }

        if (0 != ep_->Create()) {
          LIB_NET_LOG_ERROR("net create ep failed ..........................");
          return Err::kERR_SOCKET_POOL_ALLOCATE_INVALID;
        }
      }

      sock_ = reinterpret_cast<Sock **>(::malloc(sizeof(Sock*)*max_));
      if (NULL == sock_) {
        return Err::kERR_SOCKET_POOL_ALLOCATE_INVALID;
      }
      for (INT32 i = 0; i < max_; ++i) {
        sock_[i] = NULL;
      }

      last_active_ = reinterpret_cast<INT64 *>(::malloc(sizeof(INT64)*max_));
      if (NULL == last_active_) {
        return Err::kERR_SOCKET_POOL_ALLOCATE_INVALID;
      }

      circle_active_ = reinterpret_cast<INT64 *>(::malloc(sizeof(INT64)*max_));
      if (NULL == circle_active_) {
        return Err::kERR_SOCKET_POOL_ALLOCATE_INVALID;
      }

      for (INT32 i = 0; i < max_; ++i) {
        last_active_[i] = 0;
        circle_active_[i] = 0;
        sock_[i] = new SockStream();
        if (NULL == sock_[i]) {
          return Err::kERR_SOCKET_POOL_ALLOCATE_INVALID;
        }
      }

      if (0 != Connect(min_)) {
        return Err::kERR_SOCKET_CONNECT;
      }

      return 0;
    }

    INT32 SockPool::Initial(const string & addr, INT32 min, INT32 max, INT32 threshold, INT32 timeout) {
      min_ = min;
      max_ = max;
      addr_ = addr;
      timeout_ = timeout;
      circle_threshold_ = threshold;

      return Initial();
    }

    INT32 SockPool::GetConnected() {
      INT32 connected = 0;
      for (INT32 i = 0; i < max_; ++i) {
        if (NULL != sock_[i] && sock_[i]->IsValid()) {
          ++connected;
        }
      }

      return connected;
    }

    INT32 SockPool::Connect(INT32 num) {
      if (0 >= num) {
        return 0;
      }

      INT32 connected = 0;
      for (INT32 i = 0; i < max_ && connected < num; ++i) {
        if (NULL == sock_[i] || !sock_[i]->IsValid()) {
          INT32 result = sock_[i]->Create();
          if (0 != result) {
            LIB_NET_LOG_ERROR("net create socket failed ..........................");
            return Err::kERR_SOCKET_CREATE;
          }

          result = sock_[i]->Connect(addr_, timeout_);
          if (0 != result) {
            LIB_NET_LOG_ERROR("net connect failed addr: " << addr_ << "..........................");
            sock_[i]->Close();
            return Err::kERR_SOCKET_POOL_CONNECT_FAILED;
          }
          sock_[i]->SetNBlock();

          last_active_[i] = TimeFormat::GetCurTimestampLong();
          ++connected;

          if (need_listen_ && NULL != ep_) {
            AddToEP(sock_[i], i, ep_, EPOLLIN | EPOLLET);
          }
        }
      }

      return 0;
    }

    INT32 SockPool::Disconnect(INT32 num) {
      INT32 disconnect = 0;
      for (INT32 i = 0; i < max_ && disconnect < num; ++i) {
        if (NULL != sock_[i] && sock_[i]->IsValid()) {
          CloseSock(i);
          ++disconnect;
        }
      }

      return 0;
    }

    VOID SockPool::CloseAll() {
      for (INT32 i = 0; i < max_; ++i) {
        if (NULL != sock_[i] && sock_[i]->IsValid()) {
          CloseSock(i);
        }
      }
    }

    VOID SockPool::CloseSock(INT32 index) {
      if (0 > index || index >= max_) {
        return;
      }

      if (NULL != sock_[index]) {
        if (ep_) {
          DelFromEP(sock_[index], ep_);
        }
        sock_[index]->Close();
        last_active_[index] = 0;
      }
    }

    BOOL SockPool::IsBusy() {
      for (INT32 i = 0; i < max_; ++i) {
        if (NULL != sock_[i] && sock_[i]->IsValid()) {
          if (circle_active_[i] < circle_threshold_) {
            ClearCircleActive();
            return FALSE;
          }
        }
      }

      ClearCircleActive();
      return TRUE;
    }

    VOID SockPool::ClearCircleActive() {
      for (INT32 i = 0; i < max_; ++i) {
        circle_active_[i] = 0;
      }
    }

    INT32 SockPool::ValidConnect() {
      INT32 connected = GetConnected();
      INT32 need_connect = 0;
      if (connected < min_) {
        need_connect = min_ - connected;
      }

      if (IsBusy() && (connected < max_)) {
        need_connect = 1;
      }

      if (0 < need_connect) {
        LIB_NET_LOG_DEBUG(" add connect " << need_connect << " connected:" << connected << " circle threshold:" << circle_threshold_ << " ...........................................................................................");
        if (0 != Connect(need_connect)) {
          LIB_NET_LOG_ERROR("net valid connect failed addr: " << addr_ << "..........................");
        }

        return 0;
      }

      if (!IsBusy() && connected > min_) {
        LIB_NET_LOG_DEBUG(" disconnect connected:" << connected << " min_:" << min_ << " circle threshold:" << circle_threshold_ << " ...........................................................................................");
        if (0 != Disconnect(1)) {
          LIB_NET_LOG_ERROR("net valid disconnect failed addr: " << addr_ << "..........................");
        }
      }

      return 0;
    }

    INT32 SockPool::Read(INT32 index, CHAR * in, INT32 size) {
      if (0 > index || index >= max_) {
        return 0;
      }

      if (NULL != sock_[index] && sock_[index]->IsValid()) {
        INT32 result = sock_[index]->Read(in, size);
        if (0 >= result) {
          CloseSock(index);
        } else {
          last_active_[index] = TimeFormat::GetCurTimestampLong();
          pool_last_active_ = TimeFormat::GetCurTimestampLong();
        }

        return result;
      }

      return Err::kERR_SOCKET_RECV;
    }

    INT32 SockPool::Write(const CHAR * out, INT32 out_size) {
      for (INT32 i = 0; i < max_; ++i) {
        robin_ = robin_%max_;

        if (NULL == sock_[robin_] || !sock_[robin_]->IsValid()) {
          ++robin_;
          continue;
        }

        if (0 >= sock_[robin_]->Write(out, out_size)) {
          CloseSock(robin_);
          ++robin_;
          continue;
        }

        ++(circle_active_[robin_]);
        last_active_[robin_] = TimeFormat::GetCurTimestampLong();
        pool_last_active_ = TimeFormat::GetCurTimestampLong();

        ++robin_;

        return 0;
      }

      return Err::kERR_SOCKET_SEND;
    }

    EPResult SockPool::WaitForData(INT32 millisecond) {
      if (NULL == ep_) {
        return make_pair(0, reinterpret_cast<epoll_event *>(NULL));
      }

      return ep_->Wait(millisecond);
    }

    VOID SockPool::AddToEP(Sock * sock, INT32 index, EP * ep, INT32 flag) {
      if (NULL == sock || NULL == ep) {
        return;
      }

      EventExt * ext = new EventExt();
      if (NULL == ext) {
        return;
      }

      ext->pool_ = this;
      ext->sock_ = sock;
      ext->index_ = index;

      event_ext_list_.insert(pair<INT32, EventExt *>(sock->Handle(), ext));

      ep->Add(sock->Handle(), flag, reinterpret_cast<INT64>(ext));
    }

    VOID SockPool::ModFromEP(Sock * sock, EP * ep, INT32 flag) {
      if (NULL == sock || NULL == ep) {
        return;
      }

      EventExt * ext = event_ext_list_[sock->Handle()];
      if (NULL == ext) {
        return;
      }

      ep->Mod(sock->Handle(), flag, reinterpret_cast<INT64>(ext));
    }

    VOID SockPool::DelFromEP(Sock * sock, EP * ep) {
      if (NULL == sock || NULL == ep) {
        return;
      }

      EventExt * ext = event_ext_list_[sock->Handle()];
      if (NULL != ext) {
        delete ext;
      }

      event_ext_list_.erase(sock->Handle());

      ep->Del(sock->Handle(), 0, 0);
    }

    VOID SockPool::Destroy() {
      CloseAll();
      if (NULL != sock_) {
        for (INT32 i = 0; i < max_; ++i) {
          if (sock_[i]) {
            delete sock_[i];
          }
        }

        ::free(sock_);
      }

      if (last_active_) {
        ::free(last_active_);
      }

      if (ep_) {
        delete ep_;
      }
    }
  }  // namespace net
}  // namespace lib