// Copyright (c) 2013 zhou chang yue. All rights reserved.

#ifndef COMMLIB_NET_INC_SOCK_POOL_H_
#define COMMLIB_NET_INC_SOCK_POOL_H_

#include <fcntl.h>
#include <sys/socket.h>
#include <string>
#include <map>
#include "commlib/public/inc/type.h"
#include "commlib/net/inc/sock.h"
#include "commlib/net/inc/net_addr.h"
#include "commlib/net/inc/ep.h"
#include "commlib/net/inc/err.h"
#include "commlib/net/inc/log.h"

namespace lib {
  namespace net {
    using std::map;
    using std::pair;
    using std::make_pair;

    typedef EP::EPResult EPResult;

    class SockPool {
     public:
       typedef struct _event_ext {
         SockPool * pool_;
         Sock * sock_;
         INT32 index_;
       } EventExt;

       typedef map<INT32, EventExt *> EventExtList;

     public:
       typedef pair<Sock *, INT64> SOCK_POOL_ITERATOR;

     public:
       enum STATUS {
         CONNECTING = 1,
         CONNECTED = 2,
         CLOSED = 3
       };

     public:
       explicit SockPool(BOOL need_listen):addr_(""), min_(1), max_(1), robin_(0),timeout_(10),
                                           pool_last_active_(0), circle_threshold_(50), need_listen_(need_listen),
                  sock_(NULL), last_active_(NULL), circle_active_(NULL), ep_(NULL) {}
       SockPool(BOOL need_listen , const string & addr, INT32 min, INT32 max, INT32 timeout, INT32 threshold):addr_(addr),
                                                           min_(min), max_(max),
                                                           robin_(0), timeout_(timeout), pool_last_active_(0),
                                                           circle_threshold_(threshold), need_listen_(need_listen),
                                                           sock_(NULL), last_active_(NULL), circle_active_(NULL), ep_(NULL) {}

       virtual ~SockPool() { Destroy(); }

     public:
       SOCK_POOL_ITERATOR operator[](INT32 index) {
         INT64 last_active = 0;
         if (index < 0 || index >= max_) {
           return make_pair(reinterpret_cast<Sock *>(NULL), last_active);
         }

         return make_pair(sock_[index], last_active_[index]);
       }

       INT32 pool_size() { return max_; }

     public:
       INT32 Initial();
       INT32 Initial(const string & addr, INT32 min, INT32 max, INT32 threshold, INT32 timeout = Sock::NO_WAIT);
       INT32 Write(const CHAR * out, INT32 out_size);
       INT32 Read(INT32 index, CHAR * in, INT32 size);
       INT32 ValidConnect();

     public:
       VOID SetLastActive(INT32 index, INT64 last_active) {
         if (index < 0 || index >= max_) {
           return;
         }

         last_active_[index] = last_active;
       }

       VOID SetPoolLastActive(INT64 last_active) { pool_last_active_ = last_active; }

       INT64 GetPoolLastActive() { return pool_last_active_; }

     protected:
       INT32 Connect(INT32 num);
       INT32 Disconnect(INT32 num);
       INT32 GetConnected();
       BOOL IsBusy();
       VOID ClearCircleActive();

     public:
       EPResult WaitForData(INT32 millisecond);
       VOID CloseSock(INT32 index);

     protected:
       VOID AddToEP(Sock * sock, INT32 index, EP * ep, INT32 flag);
       VOID ModFromEP(Sock * sock, EP * ep, INT32 flag);
       VOID DelFromEP(Sock * sock, EP * ep);
       VOID CloseAll();

     protected:
       VOID Destroy();

     private:
       string addr_;
       INT32 min_;
       INT32 max_;
       INT32 robin_;
       INT32 timeout_;
       INT64 pool_last_active_;
       INT32 circle_threshold_;
       BOOL need_listen_;
       Sock ** sock_;
       INT64 * last_active_;
       INT64 * circle_active_;
       EP * ep_;
       EventExtList event_ext_list_;
    };
  }  // namespace net
}  // namespace lib

#endif  // COMMLIB_NET_INC_SOCK_POOL_H_