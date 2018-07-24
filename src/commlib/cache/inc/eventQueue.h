// Copyright (c) 2013 zhou chang yue. All rights reserved.

#ifndef COMMLIB_CACHE_INC_EVENTQUEUE_H_
#define COMMLIB_CACHE_INC_EVENTQUEUE_H_

#include <string>
#include "commlib/public/inc/type.h"
#include "commlib/magic/inc/queue.h"

namespace lib {
  namespace cache {
    using lib::magic::Queue;

    class EventQueue {
     public:
       static const INT32 kEVENT_QUEUE_MAX_SIZE = 30000;

     public:
       EventQueue() {}
       ~EventQueue() {}

     public:
       INT32 Push(INT32 event);
       INT32 * Pop();

     private:
       Queue<INT32, kEVENT_QUEUE_MAX_SIZE> queue_;
    };
  }  // namespace cache
}  // namespace lib

#endif  // COMMLIB_CACHE_INC_EVENTQUEUE_H_
