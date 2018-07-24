// Copyright (c) 2013 zhou chang yue. All rights reserved.

#include "commlib/cache/inc/eventQueue.h"
#include "commlib/cache/inc/log.h"

namespace lib {
  namespace cache {
    INT32 EventQueue::Push(INT32 event) {
      return queue_.Push(event);
    }

    INT32 * EventQueue::Pop() {
      return queue_.Pop();
    }
  }  // namespace cache
}  // namespace lib
