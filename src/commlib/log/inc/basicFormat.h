// Copyright (c) 2013 zhou chang yue. All rights reserved.

#ifndef COMMLIB_LOG_INC_BASICFORMAT_H_
#define COMMLIB_LOG_INC_BASICFORMAT_H_

#include <string>
#include <iostream>
#include "commlib/log/inc/record.h"
#include "commlib/log/inc/format.h"
#include "commlib/public/inc/type.h"

namespace lib {
  namespace log {
    using std::string;
    using std::ostream;
    using std::setw;
    using std::dec;
    using std::hex;

    class BasicFormat:public Format {
     public:
       BasicFormat():Format("FULL") {}
       explicit BasicFormat(const string & mode):Format(mode) {}
       virtual ~BasicFormat() {}

     public:
       virtual INT32 CreateFormat(const string & mode);
       virtual VOID Formatting(ostream * os_pointer, const Record & record);
    };
  }  // namespace log
}  // namespace lib

#endif  // COMMLIB_LOG_INC_BASICFORMAT_H_
