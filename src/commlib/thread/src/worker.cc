// Copyright (c) 2013 zhou chang yue. All rights reserved.

#include <unistd.h>
#include "commlib/thread/inc/worker.h"

namespace lib {
  namespace thread {
    INT32 Worker::Working(VOID * parameter) {
      while (TRUE) {
        if (need_stop_) {
          LIB_THREAD_LOG_DEBUG("thread stopping index " << index_);
          break;
        }

        Job * job = NULL;
        while ((NULL != (job = (*queue_.Pop())))) {
          job->Execute();
          job->SetRunning(TRUE);
          if (job->NeedDel()) {
            delete job;
          }
        }

        if (NULL == event_) {
          ::usleep(100);
        } else {
          event_->WaitForSignal();
        }
      }

      return 0;
    }

    INT32 Worker::AddJob(Job * job) {
      return queue_.Push(job);
    }

    VOID Worker::Destroy() {
    }
  }  // namespace thread
}  // namespace lib
