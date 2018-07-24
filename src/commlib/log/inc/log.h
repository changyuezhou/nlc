// Copyright (c) 2013 zhou chang yue. All rights reserved.

#ifndef COMMLIB_LOG_INC_LOG_H_
#define COMMLIB_LOG_INC_LOG_H_

#include <string>
#include <cstdio>
#include <iostream>
#include <sstream>
#include <cstring>

#include "commlib/log/inc/record.h"
#include "commlib/log/inc/handleManager.h"

namespace lib {
  namespace log {
    class Mask {
     public:
       static const INT32 kLOG_THREAD = 1;
       static const INT32 kLOG_CACHE = 2;
       static const INT32 kLOG_NET = 4;
       static const INT32 kLOG_NSF_PROXY = 8;
       static const INT32 kLOG_NSF_WORKER = 16;
       static const INT32 kLOG_NSF_CTRL = 32;
    };

    #define StreamLogging(level, expression)\
      do {\
        using std::ostringstream;\
        using std::string;\
        using std::cout;\
        using std::endl;\
        ostringstream out_os;\
        out_os << expression;\
        string const & out_string = out_os.str();\
        cout << "[" << level << "][file:" << (::strrchr(__FILE__, '/')+1);\
        cout << " line:" << __LINE__ << "][" << out_string << "]" << endl;\
      } while (0)

    #define RAW_LOG_DEBUG(expression) StreamLogging("DEBUG", expression)
    #define RAW_LOG_INFO(expression) StreamLogging("INFO", expression)
    #define RAW_LOG_WARN(expression) StreamLogging("WARN", expression)
    #define RAW_LOG_ERROR(expression) StreamLogging("ERROR", expression)
    #define RAW_LOG_FATAL(expression) StreamLogging("FATAL", expression)

    #define LoggingTo(logger, mask, level, expression)                \
      do {                                                              \
        using std::ostringstream;                                       \
        using std::string;                                              \
        using std::cout;                                                \
        using std::endl;                                                \
        using lib::log::Record;                                         \
        using lib::log::HandleManager;                                  \
        ostringstream out_os;                                           \
        out_os << expression;                                           \
        string const & out_string = out_os.str();                       \
        Record record(mask, level, ::strrchr(__FILE__, '/')+1, __LINE__, ::getpid(), ::pthread_self(), ::time(NULL), out_string); \
        logger->Logging(record);                                        \
      } while (0)

    #define LOG(mask, level, expression) LoggingTo(HandleManager::GetInstance(), mask, level, expression)

    #define LOG_SPECIAL(mask, expression) LOG(mask, "SPECIAL", expression)

    #define LOG_DEBUG(mask, expression) LOG(mask, "DEBUG", expression)
    #define LOG_INFO(mask, expression) LOG(mask, "INFO", expression)
    #define LOG_WARN(mask, expression) LOG(mask, "WARN", expression)
    #define LOG_ERROR(mask, expression) LOG(mask, "ERROR", expression)
    #define LOG_FATAL(mask, expression) LOG(mask, "FATAL", expression)

    #define LOG_FRAME(mask, level, expression) LoggingTo(HandleManager::GetFrameInstance(), mask, level, expression)

    #define LOG_FRAME_SPECIAL(mask, expression) LOG_FRAME(mask, "SPECIAL", expression)
    #define LOG_FRAME_DEBUG(mask, expression) LOG_FRAME(mask, "DEBUG", expression)
    #define LOG_FRAME_INFO(mask, expression) LOG_FRAME(mask, "INFO", expression)
    #define LOG_FRAME_WARN(mask, expression) LOG_FRAME(mask, "WARN", expression)
    #define LOG_FRAME_ERROR(mask, expression) LOG_FRAME(mask, "ERROR", expression)
    #define LOG_FRAME_FATAL(mask, expression) LOG_FRAME(mask, "FATAL", expression)

    #define LOG_TEST_DEBUG(expression) LOG_DEBUG(0xFFFFFFFFFFFFFFFF, expression)
    #define LOG_TEST_INFO(expression) LOG_INFO(0xFFFFFFFFFFFFFFFF, expression)
    #define LOG_TEST_WARN(expression) LOG_WARN(0xFFFFFFFFFFFFFFFF, expression)
    #define LOG_TEST_ERROR(expression) LOG_ERROR(0xFFFFFFFFFFFFFFFF, expression)
    #define LOG_TEST_FATAL(expression) LOG_FATAL(0xFFFFFFFFFFFFFFFF, expression)
  }  // namespace log
}  // namespace lib

#endif  // COMMLIB_LOG_INC_LOG_H_
