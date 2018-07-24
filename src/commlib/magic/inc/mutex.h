// Copyright (c) 2013 zhou chang yue. All rights reserved.

#ifndef COMMLIB_MAGIC_INC_MUTEX_H_
#define COMMLIB_MAGIC_INC_MUTEX_H_

#include <string>
#include <cstdio>
#include <iostream>
#include "commlib/public/inc/type.h"

namespace lib {
  namespace magic {
    class Mutex {
     public:
       Mutex():p_mutex_(NULL) {
         pthread_mutexattr_t attr;
         ::pthread_mutexattr_init(&attr);
         ::pthread_mutex_init(&mutex_, &attr);
         ::pthread_mutexattr_destroy(&attr);
       }

       Mutex(pthread_mutex_t * p_mutex):p_mutex_(p_mutex) {}

       ~Mutex() {
         if (NULL == p_mutex_) {
           ::pthread_mutex_destroy(&mutex_);
         }
       }

     public:
       INT32 Lock() {
         if (NULL != p_mutex_) {
           return ::pthread_mutex_lock(p_mutex_);
         }
         return ::pthread_mutex_lock(&mutex_);
       }

       INT32 UnLock() {
         if (NULL != p_mutex_) {
           return ::pthread_mutex_unlock(p_mutex_);
         }
         return ::pthread_mutex_unlock(&mutex_);
       }

       INT32 TryLock() {
         if (NULL != p_mutex_) {
           return ::pthread_mutex_trylock(p_mutex_);
         }
         return ::pthread_mutex_trylock(&mutex_);
       }

     private:
       pthread_mutex_t * p_mutex_;
       pthread_mutex_t mutex_;
    };
  }  // namespace magic
}  // namespace lib

#endif  // COMMLIB_MAGIC_INC_MUTEX_H_
