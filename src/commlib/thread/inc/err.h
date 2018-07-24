// Copyright (c) 2013 zhou chang yue. All rights reserved.

#ifndef COMMLIB_THREAD_INC_ERR_H_
#define COMMLIB_THREAD_INC_ERR_H_

#include <errno.h>
#include <string>
#include <cstdio>
#include "commlib/public/inc/type.h"

namespace lib {
  namespace thread {
    class Err {
     public:
      // Thread errors
      static const INT32 kERR_THREAD_CREATE = -100390;
      static const INT32 kERR_THREAD_CREATE_PIPE = -100391;

      // Pool errors
      static const INT32 kERR_POOL_CREATE_WORKER_FAILED = -100400;
      static const INT32 kERR_POOL_CREATE_EVENT_FAILED = -100401;
      static const INT32 kERR_POOL_CREATE_MUTEX_FAILED = -100402;
      static const INT32 kERR_POOL_ADD_JOB_FAILED = -100403;
    };
  }  // namespace thread
}  // namespace lib

#endif  // COMMLIB_THREAD_INC_ERR_H_
