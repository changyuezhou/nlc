// Copyright (c) 2013 zhou chang yue. All rights reserved.

#ifndef COMMLIB_MAGIC_INC_LOG_H_
#define COMMLIB_MAGIC_INC_LOG_H_

#include <string>
#include "commlib/public/inc/type.h"
#include "commlib/log/inc/log.h"

namespace lib {
  namespace magic {
    #define LIB_MAGIC_LOG_DEBUG(expression) RAW_LOG_DEBUG(expression)
    #define LIB_MAGIC_LOG_INFO(expression) RAW_LOG_INFO(expression)
    #define LIB_MAGIC_LOG_WARN(expression) RAW_LOG_WARN(expression)
    #define LIB_MAGIC_LOG_ERROR(expression) RAW_LOG_ERROR(expression)
    #define LIB_MAGIC_LOG_FATAL(expression) RAW_LOG_FATAL(expression)
  }  // namespace magic
}  // namespace lib

#endif  // COMMLIB_MAGIC_INC_LOG_H_
