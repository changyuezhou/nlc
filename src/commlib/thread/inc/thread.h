// Copyright (c) 2013 zhou chang yue. All rights reserved.

#ifndef COMMLIB_THREAD_INC_THREAD_H_
#define COMMLIB_THREAD_INC_THREAD_H_

#include <string>
#include "commlib/public/inc/type.h"
#include "commlib/thread/inc/err.h"

namespace lib {
  namespace thread {
    class Thread {
     public:
       typedef struct _tagParam {
         VOID * this_;
         VOID * param_;

         _tagParam() {
           this_ = NULL;
           param_ = NULL;
         }
       } TagParam;

     public:
       typedef VOID * (*_ThreadFunc)(VOID * parameter);

     public:
       explicit Thread(INT32 index = 0):index_(index) {
         pipe_[0] = 0;
         pipe_[1] = 0;
         need_stop_ = FALSE;
       }
       virtual ~Thread() { Destroy(); }

     public:
       virtual INT32 CreateThread(_ThreadFunc func, VOID * parameter, INT32 stack);
       virtual INT32 Running(VOID * parameter, INT32 stack_size);

     public:
       virtual INT32 Working(VOID * parameter);

     public:
       INT32 GetIndex() { return index_; }
       pthread_t GetPID() { return pid_; }
       VOID Stop() { need_stop_ = TRUE; }
       VOID SetPID(pthread_t pid) { pid_ = pid; }

     public:
       INT32 PostStopSignal();
       INT32 WaitForStop(INT32 timeout = 0);

     public:
       VOID Destroy();

     private:
       explicit Thread(const Thread &) {}
       const Thread & operator=(const Thread &) { return *this; }

     private:
       static VOID * WorkingThread(VOID * parameter);

     protected:
       BOOL need_stop_;
       INT32 index_;

     private:
       INT32 pipe_[2];
       TagParam param_;
       pthread_t pid_;
    };
  }  // namespace thread
}  // namespace lib

#endif  // COMMLIB_THREAD_INC_THREAD_H_
