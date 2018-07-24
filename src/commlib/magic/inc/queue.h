// Copyright (c) 2013 zhou chang yue. All rights reserved.

#ifndef COMMLIB_MAGIC_INC_QUEUE_H_
#define COMMLIB_MAGIC_INC_QUEUE_H_

#include <string.h>
#include <string>
#include "commlib/public/inc/type.h"
#include "commlib/magic/inc/log.h"

namespace lib {
  namespace magic {
    template<class T, INT32 kSIZE>
    class Queue {
     public:
       Queue():rptr_(0), wptr_(0) {}

     public:
       INT32 Push(const T & d) {
         if ((wptr_ + 1)%kSIZE == rptr_) {
           LIB_MAGIC_LOG_WARN("QueueTemplate queue is full can't push size " \
                              << kSIZE << " wptr: " << wptr_ << " rptr: " << rptr_);
           return -1;
         }
         ::memcpy(&data_[wptr_], &d, sizeof(T));
         wptr_ = (wptr_ + 1)%kSIZE;
         return 0;
       }

       T * Pop() {
         if (rptr_ == wptr_) {
           return NULL;
         }

         T * d = &data_[rptr_];
         rptr_ = (rptr_ + 1)%kSIZE;

         return d;
       }

     private:
       volatile INT32 rptr_;
       volatile INT32 wptr_;

       T data_[kSIZE];
    };
  }  // namespace magic
}  // namespace lib

#endif  // COMMLIB_MAGIC_INC_QUEUE_H_
