// Copyright (c) 2013 zhou chang yue. All rights reserved.

#ifndef COMMLIB_MAGIC_INC_IGNORESIGNAL_H_
#define COMMLIB_MAGIC_INC_IGNORESIGNAL_H_

#include <signal.h>
#include <string>
#include "commlib/public/inc/type.h"

namespace lib {
  namespace magic {
    class IgnoreSignal {
     public:
       static VOID IgnoreAllSignal();
       static VOID IgnoreDaemon();
    };
  }  // namespace magic
}  // namespace lib

#endif  // COMMLIB_MAGIC_INC_IGNORESIGNAL_H_
