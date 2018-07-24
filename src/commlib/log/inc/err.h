// Copyright (c) 2013 zhou chang yue. All rights reserved.

#ifndef COMMLIB_LOG_INC_ERR_H_
#define COMMLIB_LOG_INC_ERR_H_

#include <errno.h>
#include <string>
#include <cstdio>
#include "commlib/public/inc/type.h"

namespace lib {
  namespace log {
    using std::string;

    class Err {
     public:
       static const INT32 kERR_STREAM_HANDLE_OBJECT_NOT_GOOD = -100190;
       static const INT32 kERR_FILE_HANDLE_DESC_INVALID = -100191;
       static const INT32 kERR_FILE_HANDLE_WRITE_FILE_FAILED = 100192;
       static const INT32 kERR_FILE_HANDLE_SEEK_FILE_FAILED = -100193;
       static const INT32 kERR_FILE_HANDLE_RENAME_FILE_FAILED = -100194;
       static const INT32 kERR_FILE_HANDLE_REOPEN_FILE_FAILED = -100195;
       static const INT32 kERR_FILE_HANDLE_INVALID_FORMAT_FAILED = -100196;
       static const INT32 kERR_FILE_HANDLE_INVALID_TYPE_FAILED = -100197;
       static const INT32 kERR_FILE_CONFIG_CREATE_FAILED = -100198;
       static const INT32 kERR_FILE_HANDLE_INVALID = -100199;
       static const INT32 kERR_FILE_HANDLE_CREATE_FAILED = -100200;
    };
  }  // namespace log
}  // namespace lib

#endif  // COMMLIB_LOG_INC_ERR_H_
