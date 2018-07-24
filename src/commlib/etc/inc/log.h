// Copyright (c) 2013 zhou chang yue. All rights reserved.

#ifndef COMMLIB_ETC_INC_LOG_H_
#define COMMLIB_ETC_INC_LOG_H_

#include <string>
#include "commlib/public/inc/type.h"
#include "commlib/log/inc/log.h"

namespace lib {
  namespace etc {
    #define LIB_ETC_LOG_DEBUG(expression) RAW_LOG_DEBUG(expression)
    #define LIB_ETC_LOG_INFO(expression) RAW_LOG_INFO(expression)
    #define LIB_ETC_LOG_WARN(expression) RAW_LOG_WARN(expression)
    #define LIB_ETC_LOG_ERROR(expression) RAW_LOG_ERROR(expression)
    #define LIB_ETC_LOG_FATAL(expression) RAW_LOG_FATAL(expression)
  }  // namespace etc
}  // namespace lib

#endif  // COMMLIB_ETC_INC_LOG_H_
