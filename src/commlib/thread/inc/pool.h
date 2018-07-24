// Copyright (c) 2013 zhou chang yue. All rights reserved.

#ifndef COMMLIB_THREAD_INC_POOL_H_
#define COMMLIB_THREAD_INC_POOL_H_

#include <string>
#include "commlib/public/inc/type.h"
#include "commlib/thread/inc/thread.h"
#include "commlib/thread/inc/job.h"
#include "commlib/thread/inc/worker.h"
#include "commlib/thread/inc/log.h"
#include "commlib/thread/inc/err.h"
#include "commlib/magic/inc/pipeEvent.h"
#include "commlib/magic/inc/mutex.h"
#include "commlib/magic/inc/scopeLock.h"

namespace lib {
  namespace thread {
    using std::string;
    using lib::magic::PipeEvent;
    using lib::magic::Mutex;
    using lib::magic::ScopeLock;

    class Pool {
     public:
       static const INT32 kPOOL_SIZE = 8;

     public:
       explicit Pool(INT32 size = kPOOL_SIZE, BOOL async = FALSE):need_safe_(FALSE),
         is_async_(async), size_(size), lb_(0),
         worker_list_(NULL),
         event_list_(NULL) {}
       ~Pool() { Destroy(); }

     public:
       INT32 AddJob(Job * job, UINT32 id = 0);

     public:
       INT32 Create(INT32 size = kPOOL_SIZE);
       VOID Destroy();

     private:
       const Pool & operator=(const Pool &);
       Pool(const Pool &);

     private:
       BOOL need_safe_;
       BOOL is_async_;
       INT32 size_;
       INT32 lb_;
       Worker * worker_list_;
       PipeEvent * event_list_;
       Mutex * mutex_list_;
    };
  }  // namespace thread
}  // namespace lib

#endif  // COMMLIB_THREAD_INC_POOL_H_
