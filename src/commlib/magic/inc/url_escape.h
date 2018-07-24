// Copyright (c) 2013 zhou chang yue. All rights reserved.

#ifndef COMMLIB_MAGIC_INC_URL_ESCAPE_H_
#define COMMLIB_MAGIC_INC_URL_ESCAPE_H_

#include <sys/time.h>
#include <string>
#include "commlib/public/inc/type.h"

namespace lib {
  namespace magic {
    using std::string;

    class URL_ESCAPE {
     public:
       static char from_hex(char ch);
       static char to_hex(char code);
       static char * url_encode(const char *str, char * desc, int size);
       static char * url_decode(const char *str, char * desc, int size);
    };
  }  // namespace magic
}  // namespace lib

#endif  // COMMLIB_MAGIC_INC_URL_ESCAPE_H_