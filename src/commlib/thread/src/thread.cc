// Copyright (c) 2013 zhou chang yue. All rights reserved.

#include <unistd.h>
#include <sys/select.h>
#include "commlib/thread/inc/thread.h"
#include "commlib/thread/inc/log.h"

namespace lib {
  namespace thread {
    VOID * Thread::WorkingThread(VOID * parameter) {
      if (NULL == parameter) {
        LIB_THREAD_LOG_ERROR("Thread working thread parameter is null");
        return NULL;
      }

      TagParam * param = reinterpret_cast<TagParam *>(parameter);
      Thread * thread = reinterpret_cast<Thread *>(param->this_);
      if (NULL == thread) {
        LIB_THREAD_LOG_ERROR("Thread working thread object is null");
        return NULL;
      }
      INT32 result = thread->Working(param->param_);
      if (0 > result) {
        LIB_THREAD_LOG_WARN("Thread working thread running failed result:" << result);
      }

      result = thread->PostStopSignal();
      if (0 > result) {
        LIB_THREAD_LOG_WARN("Thread working thread post stop signal failed result:" << result);
      }

      return NULL;
    }

    INT32 Thread::Running(VOID * parameter, INT32 stack_size) {
      INT32 result = 0;
      if (0 != (result = ::pipe(pipe_))) {
        LIB_THREAD_LOG_ERROR("create pipe failed err msg:" << ::strerror(errno));
        return Err::kERR_THREAD_CREATE_PIPE;
      }

      param_.this_ = reinterpret_cast<VOID *>(this);
      param_.param_ = reinterpret_cast<VOID *>(parameter);

      if (0 > (result = CreateThread(Thread::WorkingThread, reinterpret_cast<VOID *>(&param_), stack_size))) {
        LIB_THREAD_LOG_ERROR("Thread running create thread net service work thread failed");
        return result;
      }

      return result;
    }

    INT32 Thread::CreateThread(_ThreadFunc func, VOID * parameter, INT32 stack) {
      pthread_t pid;
      pthread_attr_t opt;
      ::pthread_attr_init(&opt);
      ::pthread_attr_setdetachstate(&opt, PTHREAD_CREATE_DETACHED);
      ::pthread_attr_setstacksize(&opt, stack);

      INT32 ret = ::pthread_create(&pid, &opt, func, parameter);
      ::usleep(300);
      ::pthread_attr_destroy(&opt);

      if (0 != ret) {
        LIB_THREAD_LOG_ERROR("Thread create thread result code:" << ret << " failed errmsg:" << ::strerror(errno));
        return Err::kERR_THREAD_CREATE;
      }
      pid_ = pid;

      return ret;
    }

    INT32 Thread::Working(VOID * parameter) {
      while (TRUE) {
        if (need_stop_) {
          LIB_THREAD_LOG_DEBUG("thread stopping index " << index_);
          break;
        }

        LIB_THREAD_LOG_DEBUG("Thread running normal index " << index_);

        ::sleep(2);
      }

      return 0;
    }

    INT32 Thread::WaitForStop(INT32 timeout) {
      fd_set rdset;
      FD_ZERO(&rdset);
      FD_SET(pipe_[0], &rdset);

      struct timeval tm;
      tm.tv_sec = timeout/1000;
      tm.tv_usec = (timeout%1000)*1000;

      if (0 == timeout) {
        ::select(pipe_[0]+1, &rdset, NULL, NULL, NULL);
        need_stop_ = TRUE;
      } else {
        if (0 < ::select(pipe_[0]+1, &rdset, NULL, NULL, &tm)) {
          need_stop_ = TRUE;
          CHAR buf[32] = {0};
          ::read(pipe_[0], buf, sizeof(buf));
        }
      }

      return 0;
    }

    INT32 Thread::PostStopSignal() {
      return (1 == ::write(pipe_[1], "I", 1)?0:-1);
    }

    VOID Thread::Destroy() {
      if (0 < pipe_[0]) {
        ::close(pipe_[0]);
      }

      if (0 < pipe_[1]) {
        ::close(pipe_[1]);
      }
    }
  }  // namespace thread
}  // namespace lib
