// Copyright (c) 2013 zhou chang yue. All rights reserved.

#ifndef COMMLIB_LOG_INC_NETHANDLE_H_
#define COMMLIB_LOG_INC_NETHANDLE_H_

#include <string>
#include <ostream>
#include <iostream>
#include "commlib/public/inc/type.h"
#include "commlib/log/inc/record.h"
#include "commlib/log/inc/handle.h"

namespace lib {
  namespace log {
    class NetHandle:public Handle {
     public:
       enum NET_TYPE {
         TCP = 0,
         UDP = 1,
         UNIX_STREAM = 2,
         UNIX_DGRAM
       };

     public:
       NetHandle(Format * format, \
                 LEVEL level, BOOL exact, UINT64 mask, \
                 const string & net_addr, INT32 type): \
                 Handle(format, level, exact, mask), \
                  net_addr_(net_addr), type_(type) {}
       virtual ~NetHandle() {}

     public:
       virtual INT32 Logging(const Record & record);
       virtual VOID DestroyHandle();

     public:
       VOID Open();
       INT32 WriteData(const CHAR * out, INT32 size);

     protected:
       string net_addr_;
       INT32 type_;
    };
  }  // namespace log
}  // namespace lib

#endif  // COMMLIB_LOG_INC_NETHANDLE_H_
