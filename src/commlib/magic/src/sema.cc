// Copyright (c) 2013 zhou chang yue. All rights reserved.

#include "commlib/magic/inc/sema.h"

namespace lib {
  namespace magic {
    INT32 Semaphore::CreateSemaphore(const string & name, INT32 flag, mode_t mode, UINT32 val) {
      semaphore_ = ::sem_open(name.c_str(), flag, mode, val);
      if (SEM_FAILED == semaphore_) {
        return -1;
      }
      name_ = name;
      return 0;
    }
    INT32 Semaphore::CreateSemaphore(const string & name) {
      semaphore_ = ::sem_open(name.c_str(), O_CREAT | O_EXCL);
      if (SEM_FAILED == semaphore_) {
        return -1;
      }
      name_ = name;
      return 0;
    }
    INT32 Semaphore::CreateSemaphore(const string & name, INT32 flag) {
      semaphore_ = ::sem_open(name.c_str(), flag);
      if (SEM_FAILED == semaphore_) {
        return -1;
      }
      name_ = name;
      return 0;
    }

    INT32 Semaphore::WaitSemaphore() {
      return ::sem_wait(semaphore_);
    }

    INT32 Semaphore::PostSemaphore() {
      return ::sem_post(semaphore_);
    }

    INT32 Semaphore::GetSemaphoreValue(INT32 * val) {
      return ::sem_getvalue(semaphore_, val);
    }

    INT32 Semaphore::TryWaitSemaphore() {
      return ::sem_trywait(semaphore_);
    }

    INT32 Semaphore::CloseSemaphore() {
      return ::sem_close(semaphore_);
    }

    INT32 Semaphore::UnlinkSemaphore() {
      return ::sem_unlink(name_.c_str());
    }
  }  // namespace magic
}  // namespace lib
