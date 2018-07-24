// Copyright (c) 2013 zhou chang yue. All rights reserved.

#ifndef COMMLIB_MAGIC_INC_TIMEFORMAT_H_
#define COMMLIB_MAGIC_INC_TIMEFORMAT_H_

#include <sys/time.h>
#include <string>
#include "commlib/public/inc/type.h"

namespace lib {
  namespace magic {
    using std::string;

    class TimeFormat {
     public:
       static const string GetStringISO(const time_t timestamp);
       static const string GetStringDate(const time_t timestamp);
       static const string GetStringISO();

       static const string GetCurDate(const string & format);

       static UINT64 GetCurTimestampLong();
       static UINT32 GetCurTimestampINT32();
    };
  }  // namespace magic
}  // namespace lib

#endif  // COMMLIB_MAGIC_INC_TIMEFORMAT_H_
