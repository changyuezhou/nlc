// Copyright (c) 2013 zhou chang yue. All rights reserved.

#ifndef COMMLIB_THREAD_INC_JOB_H_
#define COMMLIB_THREAD_INC_JOB_H_

#include <string>
#include "commlib/public/inc/type.h"
#include "commlib/thread/inc/err.h"

namespace lib {
  namespace thread {
    class Job {
     public:
       Job():is_running_(FALSE), need_destroy_(FALSE) {}
       explicit Job(BOOL need_del):is_running_(FALSE), need_destroy_(need_del) {}
       virtual ~Job() { Destroy(); }

     public:
       virtual INT32 Execute() = 0;

     public:
       BOOL IsRunning() { return is_running_; }
       BOOL NeedDel() { return need_destroy_; }
       VOID SetRunning(BOOL running) { is_running_ = running; }

     public:
       VOID Destroy() {}

     private:
       const Job & operator=(const Job &);
       Job(const Job &);

     private:
       BOOL is_running_;
       BOOL need_destroy_;
    };
  }  // namespace thread
}  // namespace lib

#endif  // COMMLIB_THREAD_INC_JOB_H_
