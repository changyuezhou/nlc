#ifndef LTS_PUBLIC_INC_LOG_H
#define LTS_PUBLIC_INC_LOG_H
#include <string>
#include "commlib/public/inc/type.h"
#include "commlib/log/inc/log.h"


    #define LTS_PUB_LOG_DEBUG(expression)  LOG_DEBUG(64, expression)
    #define LTS_PUB_LOG_INFO(expression)  LOG_INFO(64, expression)
    #define LTS_PUB_LOG_WARN(expression) LOG_WARN(64, expression)
    #define LTS_PUB_LOG_ERROR(expression) LOG_ERROR(64, expression)
    #define LTS_PUB_LOG_FATAL(expression) LOG_FATAL(64, expression)

#endif
