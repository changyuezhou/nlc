// Copyright (c) 2013 zhou chang yue. All rights reserved.

#ifndef COMMLIB_MAGIC_INC_STRINGCHECK_H_
#define COMMLIB_MAGIC_INC_STRINGCHECK_H_

#include <unistd.h>
#include <string.h>
#include <string>
#include <algorithm>
#include "commlib/public/inc/type.h"

namespace lib {
  namespace magic {
    using std::string;

    class StringCheck {
     public:
       static inline BOOL check_ip(const string & str) {
         for (INT32 i = 0; i< str.length(); i++) {
           if (('.' != str.at(i)) &&                     \
               (('0' > str.at(i)) || ('9' < str.at(i)))) {
             return FALSE;
           }
         }
       }

       static inline BOOL check_num(const string & str) {
         INT32 len = str.length();
         for (INT32 i = 0; i < len; i++) {
           if (('0' > str.at(i)) || ('9' < str.at(i))) {
             return FALSE;
           }
         }

         return TRUE;
       }

       static inline BOOL check_bool(const CHAR * str) {
         if ((2 == ::strlen(str)) && (0 == ::strncasecmp(str, "no", 2))) {
           return TRUE;
         }

         if ((3 == ::strlen(str)) && (0 == ::strncasecmp(str, "yes", 3))) {
           return TRUE;
         }

         return FALSE;
       }

       static inline BOOL check_file(const CHAR * str) {
         if (::access(str, F_OK)) {
           return FALSE;
         }

         return TRUE;
       }
    };
  }  // namespace magic
}  // namespace lib

#endif  // COMMLIB_MAGIC_INC_STRINGCHECK_H_
