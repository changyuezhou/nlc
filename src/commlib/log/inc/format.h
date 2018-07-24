// Copyright (c) 2013 zhou chang yue. All rights reserved.

#ifndef COMMLIB_LOG_INC_FORMAT_H_
#define COMMLIB_LOG_INC_FORMAT_H_

#include <strings.h>
#include <string>
#include <ostream>
#include <iomanip>
#include "commlib/public/inc/type.h"
#include "commlib/log/inc/record.h"

namespace lib {
  namespace log {
    using std::string;
    using std::ostream;
    using std::setw;

    class Format {
     public:
       Format():mode_(4) {}
       explicit Format(const string & mode) {
         mode_ = GetMode(mode);
       }
       virtual ~Format() {}

     public:
       enum Mode {
         MOD_NONE = 0,
         MOD_MIN = 1,
         MOD_BRIEF = 2,
         MOD_LOCATE = 3,
         MOD_FULL = 4
       };

       static const CHAR ModTable[5][32];

       INT32 GetMode(const string & mode) {
         for (INT32 i = MOD_NONE; i <= MOD_FULL; i = i + 1) {
           if (0 == ::strncasecmp(mode.c_str(), ModTable[i], mode.length())) {
             return i;
           }
         }

         return 0;
       }

     public:
       virtual INT32 CreateFormat(const string & mode) = 0;
       virtual VOID Formatting(ostream * os_pointer, const Record & record) = 0;

     protected:
       INT32 mode_;
    };
  }  // namespace log
}  // namespace lib

#endif  // COMMLIB_LOG_INC_FORMAT_H_
