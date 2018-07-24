// Copyright (c) 2013 zhou chang yue. All rights reserved.

#ifndef COMMLIB_MAGIC_INC_ERR_H_
#define COMMLIB_MAGIC_INC_ERR_H_

#include <errno.h>
#include <string>
#include <cstdio>
#include "commlib/public/inc/type.h"

namespace lib {
  namespace magic {
    using std::string;

    class Err {
     public:
       // errors
       static const INT32 kERR_MAGIC_DLL_LOAD_FILE = -100200;

       // pipe errors
       static const INT32 kERR_PIPE_CREATE = -100201;
       static const INT32 kERR_PIPE_SET_NONBLOCK = -100202;
       static const INT32 kERR_CRC_PARAMETER_INVALID = -100202;
    };
  }  // namespace magic
}  // namespace lib

#endif  // COMMLIB_MAGIC_INC_ERR_H_
