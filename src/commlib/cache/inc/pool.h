// Copyright (c) 2013 zhou chang yue. All rights reserved.

#ifndef COMMLIB_CACHE_INC_POOL_H_
#define COMMLIB_CACHE_INC_POOL_H_

#include <string>
#include "commlib/public/inc/type.h"
#include "commlib/cache/inc/err.h"

namespace lib {
  namespace cache {
    template<typename T>
    class Pool {
     public:
       explicit Pool(INT32 size = 10000):size_(size), head_(NULL) {}
       ~Pool() {
         if (NULL != free_point_) {
           delete[] free_point_;
         }
       }

     public:
       INT32 Create();
       T * GetFreePoint();
       T * GetItem();
       VOID FreeItem(T * item);

     private:
       INT32 size_;
       T * head_;
       T * free_point_;
    };
  }  // namespace cache
}  // namespace lib

#endif  // COMMLIB_CACHE_INC_POOL_H_
