// Copyright (c) 2013 zhou chang yue. All rights reserved.

#ifndef COMMLIB_ETC_INC_ERR_H_
#define COMMLIB_ETC_INC_ERR_H_

#include <string>
#include <cstdio>
#include "commlib/public/inc/type.h"

namespace lib {
  namespace etc {
    using std::string;

    class Err {
     public:
       // ini errors
       static const INT32 kERR_PARSER_FILE = -100100;
       static const INT32 kERR_PARSER_FILE_OPEN = -100101;
       static const INT32 kERR_PARSER_FILE_LOAD_KEY = -100102;
       static const INT32 kERR_LOAD_KEY_NOT_FOUND = -100103;
       static const INT32 kERR_LOAD_KEY_MUTI = -100104;
       static const INT32 kERR_LOAD_ARRAY_FAILED = -100105;
       static const INT32 kERR_ARRAY_EMPTY = -100106;
    };
  }  // namespace etc
}  // namespace lib

#endif  // COMMLIB_ETC_INC_ERR_H_
