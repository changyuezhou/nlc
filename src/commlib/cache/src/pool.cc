// Copyright (c) 2013 zhou chang yue. All rights reserved.

#ifndef COMMLIB_CACHE_INC_POOL_CC_
#define COMMLIB_CACHE_INC_POOL_CC_

#include <stdlib.h>
#include "commlib/cache/inc/pool.h"

namespace lib {
  namespace cache {
    template<typename T>
    INT32 Pool<T>::Create() {
      free_point_ = new T[size_];
      if (NULL == free_point_) {
        return Err::kERR_POOL_CREATE_FAILED;
      }

      head_ = &free_point_[0];
      T * item = head_;
      for (INT32 i = 1; i < size_; i++) {
        item->SetNext(&free_point_[i]);
        item = item->GetNext();
      }

      return 0;
    }

    template<typename T>
    T * Pool<T>::GetFreePoint() {
      return free_point_;
    }

    template<typename T>
    T * Pool<T>::GetItem() {
      T * free = head_;
      head_ = head_->GetNext();
      return free;
    }

    template<typename T>
    VOID Pool<T>::FreeItem(T * item) {
      item->SetNext(head_);
      head_ = item;
    }
  }  // namespace cache
}  // namespace lib

#endif  // COMMLIB_CACHE_INC_POOL_CC_
