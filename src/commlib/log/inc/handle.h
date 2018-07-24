// Copyright (c) 2013 zhou chang yue. All rights reserved.

#ifndef COMMLIB_LOG_INC_HANDLE_H_
#define COMMLIB_LOG_INC_HANDLE_H_

#include <unistd.h>
#include <string>
#include <cstdio>
#include <iostream>
#include "commlib/public/inc/type.h"
#include "commlib/log/inc/record.h"
#include "commlib/log/inc/format.h"
#include "commlib/magic/inc/mutex.h"
#include "commlib/magic/inc/scopeLock.h"
#include "commlib/log/inc/err.h"

namespace lib {
  namespace log {
    using lib::magic::Mutex;
    using lib::magic::ScopeLock;

    class Handle {
     public:
       typedef Record::LEVEL LEVEL;

     public:
       static const INT32 kLOG_THREAD = 1;
       static const INT32 kLOG_CACHE = 2;
       static const INT32 kLOG_NET = 4;
       static const INT32 kLOG_NSF = 8;
       static const INT32 kLOG_PLUS = 16;

     public:
       Handle(Format * format, \
              LEVEL level, BOOL exact, UINT64 mask):    \
              fd_(-1), format_(format), \
              level_(level), exact_(exact), mask_(mask) {}
       virtual ~Handle() {}

     public:
       virtual INT32 Logging(const Record & record) = 0;
       virtual VOID DestroyHandle() {
         if (0 < fd_) {
           ::close(fd_);
           fd_ = -1;
         }

         if (NULL != format_) {
           delete format_;
           format_ = NULL;
         }
       }

     protected:
       INT32 fd_;
       Format * format_;
       LEVEL level_;
       BOOL exact_;
       UINT64 mask_;
       Mutex mutex_;
    };
  }  // namespace log
}  // namespace lib

#endif  // COMMLIB_LOG_INC_HANDLE_H_
