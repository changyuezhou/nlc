// Copyright (c) 2013 zhou chang yue. All rights reserved.

#include "commlib/magic/inc/delay.h"

namespace lib {
  namespace magic {
    VOID Delay::DelaySecond(INT32 second) {
      struct timeval tv;
      fd_set rset;
      tv.tv_sec = second;
      tv.tv_usec = 0;
      FD_ZERO(&rset);
      ::select(0, &rset, NULL, NULL, &tv);
    }

    VOID Delay::DelayMSecond(INT32 m_second) {
      struct timeval tv;
      fd_set rset;
      tv.tv_sec = 0;
      tv.tv_usec = m_second*1000;
      FD_ZERO(&rset);
      ::select(0, &rset, NULL, NULL, &tv);
    }

    VOID Delay::DelayUSecond(INT32 u_second) {
      struct timeval tv;
      fd_set rset;
      tv.tv_sec = 0;
      tv.tv_usec = u_second;
      FD_ZERO(&rset);
      ::select(0, &rset, NULL, NULL, &tv);
    }
  }  // namespace magic
}  // namespace lib
