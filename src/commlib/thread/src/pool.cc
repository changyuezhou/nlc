// Copyright (c) 2013 zhou chang yue. All rights reserved.

#include "commlib/thread/inc/pool.h"

namespace lib {
  namespace thread {
    INT32 Pool::Create(INT32 size) {
      if (need_safe_) {
        mutex_list_ = new Mutex[size];
        if (NULL == mutex_list_) {
          LIB_THREAD_LOG_ERROR("pool allcate event objs failed size " << size);
          return Err::kERR_POOL_CREATE_MUTEX_FAILED;
        }
      }

      worker_list_ = new Worker[size];
      if (NULL == worker_list_) {
        LIB_THREAD_LOG_ERROR("pool allcate worker objs failed size " << size);
        return Err::kERR_POOL_CREATE_WORKER_FAILED;
      }

      if (is_async_) {
        event_list_ = new PipeEvent[size];
        if (NULL == event_list_) {
          LIB_THREAD_LOG_ERROR("pool allcate event objs failed size " << size);
          return Err::kERR_POOL_CREATE_EVENT_FAILED;
        }

        for (INT32 i = 0; i < size; i++) {
          worker_list_[i].SetEvent(&event_list_[i]);
        }
      }

      for (INT32 i = 0; i < size; i++) {
        worker_list_[i].Running(reinterpret_cast<VOID *>(NULL), 32*1024);
      }

      LIB_THREAD_LOG_INFO("pool create successed worker size " << size);

      return 0;
    }

    INT32 Pool::AddJob(Job * job, UINT32 id) {
      INT32 index = id%size_;

      if (NULL != mutex_list_) {
        mutex_list_[index].Lock();
      }

      if (0 == id) {
        index = (lb_++)%size_;
      }

      INT32 result = 0;
      if (NULL != worker_list_) {
        result = worker_list_[index].AddJob(job);
      } else {
        result = Err::kERR_POOL_ADD_JOB_FAILED;
      }

      if (NULL != mutex_list_) {
        mutex_list_[index].UnLock();
      }

      return result;
    }

    VOID Pool::Destroy() {
      if (NULL != worker_list_) {
        delete[] worker_list_;
        worker_list_ = NULL;
      }

      if (NULL == event_list_) {
        delete[] event_list_;
        event_list_ = NULL;
      }

      if (NULL == mutex_list_) {
        delete[] mutex_list_;
        mutex_list_ = NULL;
      }
    }
  }  // namespace thread
}  // namespace lib
