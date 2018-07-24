// Copyright (c) 2013 zhou chang yue. All rights reserved.

#ifndef COMMLIB_MAGIC_INC_SEMA_H_
#define COMMLIB_MAGIC_INC_SEMA_H_

#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <string>
#include "commlib/public/inc/type.h"

namespace lib {
  namespace magic {
    using std::string;

    class Semaphore {
     public:
       Semaphore() {}
       ~Semaphore() {}

     public:
       INT32 CreateSemaphore(const string &name, INT32 flag, mode_t mode, UINT32 val);
       INT32 CreateSemaphore(const string &name, INT32 flag);
       INT32 CreateSemaphore(const string &name);

       INT32 WaitSemaphore();
       INT32 TryWaitSemaphore();

       INT32 PostSemaphore();
       INT32 GetSemaphoreValue(INT32 * val);

       INT32 CloseSemaphore();

       INT32 UnlinkSemaphore();

     private:
       sem_t * semaphore_;
       string name_;
    };
  }  // namespace magic
}  // namespace lib

#endif  // COMMLIB_MAGIC_INC_SEMA_H_
