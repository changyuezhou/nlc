// Copyright (c) 2013 zhou chang yue. All rights reserved.

#ifndef COMMLIB_MAGIC_INC_SCOPELOCK_H_
#define COMMLIB_MAGIC_INC_SCOPELOCK_H_

#include <string>
#include "commlib/public/inc/type.h"

namespace lib {
  namespace magic {
    template<class Lock>
    class ScopeLock {
     public:
       explicit ScopeLock(Lock * lock):lock_(lock), is_locked_(FALSE) {
        if (0 == lock_->Lock()) {
           is_locked_ = TRUE;
         }
       }

       ~ScopeLock() {
         if (is_locked_) {
           lock_->UnLock();
         }
       }
     public:
       BOOL is_locked() { return is_locked_; }
         
     private:
       Lock * lock_;
       BOOL is_locked_;
    };
  }  // namespace magic
}  // namespace lib

#endif  // COMMLIB_MAGIC_INC_SCOPELOCK_H_
