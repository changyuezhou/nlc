// Copyright (c) 2013 zhou chang yue. All rights reserved.

#ifndef COMMLIB_THREAD_INC_WORKER_H_
#define COMMLIB_THREAD_INC_WORKER_H_

#include <string>
#include "commlib/public/inc/type.h"
#include "commlib/thread/inc/thread.h"
#include "commlib/thread/inc/job.h"
#include "commlib/thread/inc/log.h"
#include "commlib/thread/inc/err.h"
#include "commlib/magic/inc/queue.h"
#include "commlib/magic/inc/pipeEvent.h"

namespace lib {
  namespace thread {
    using lib::magic::Queue;
    using lib::magic::PipeEvent;

    class Worker:public Thread {
     public:
       static const INT32 kJOB_QUEUE_MAX_SIZE = 30000;

     public:
       explicit Worker(PipeEvent * event = NULL):event_(event) {}
       virtual ~Worker() { Destroy(); }

     public:
       virtual INT32 Working(VOID * parameter);

     public:
       INT32 AddJob(Job * job);
       VOID SetEvent(PipeEvent * event) { event_ = event; }

     public:
       VOID Destroy();

     private:
       const Worker & operator=(const Worker &);
       Worker(const Worker &);

     private:
       Queue<Job *, kJOB_QUEUE_MAX_SIZE> queue_;
       PipeEvent * event_;
    };
  }  // namespace thread
}  // namespace lib

#endif  // COMMLIB_THREAD_INC_WORKER_H_
