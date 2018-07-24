// Copyright (c) 2013 zhou chang yue. All rights reserved.

#ifndef COMMLIB_LOG_INC_STREAMHANDLE_H_
#define COMMLIB_LOG_INC_STREAMHANDLE_H_

#include <string>
#include <ostream>
#include <iostream>
#include "commlib/public/inc/type.h"
#include "commlib/log/inc/handle.h"

namespace lib {
  namespace log {
    class StreamHandle:public Handle {
     public:
       StreamHandle(Format * format, \
                    LEVEL level, BOOL exact, UINT64 mask, ostream * os): \
                    Handle(format, level, exact, mask), \
                    out_stream_(os) {}
       virtual ~StreamHandle() {}

     public:
       virtual INT32 Logging(const Record & record);
       virtual VOID DestroyHandle();

     private:
       ostream * out_stream_;
    };
  }  // namespace log
}  // namespace lib

#endif  // COMMLIB_LOG_INC_STREAMHANDLE_H_
