// Copyright (c) 2013 zhou chang yue. All rights reserved.

#ifndef COMMLIB_MAGIC_INC_DELAY_H_
#define COMMLIB_MAGIC_INC_DELAY_H_

#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <string>
#include "commlib/public/inc/type.h"

namespace lib {
  namespace magic {
    class Delay {
     public:
       static VOID DelaySecond(INT32 second);
       static VOID DelayMSecond(INT32 m_second);
       static VOID DelayUSecond(INT32 u_second);
    };
  }  // namespace magic
}  // namespace lib

#endif  // COMMLIB_MAGIC_INC_DELAY_H_
