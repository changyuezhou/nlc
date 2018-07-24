// Copyright (c) 2013 zhou chang yue. All rights reserved.

#ifndef COMMLIB_MAGIC_INC_PIPEEVENT_H_
#define COMMLIB_MAGIC_INC_PIPEEVENT_H_

#include <unistd.h>
#include <string>
#include <cstdio>
#include "commlib/public/inc/type.h"

namespace lib {
  namespace magic {
    class PipeEvent {
     public:
       static const INT32 kINFINITE = 0;

     public:
       PipeEvent() {}
       ~PipeEvent() { Destroy(); }

     public:
       INT32 CreateEvent();
       INT32 WaitForSignal(INT32 timeout = kINFINITE);
       INT32 PostSignal();
       INT32 SetNBlock(INT32 fd);

     public:
       VOID Destroy() {
         if (0 < pipe_[0]) {
           ::close(pipe_[0]);
         }

         if (0 < pipe_[1]) {
           ::close(pipe_[1]);
         }
       }

     private:
       PipeEvent(const PipeEvent &);
       PipeEvent &operator=(const PipeEvent &);

     private:
       INT32 pipe_[2];
    };
  }  // namespace magic
}  // namespace lib

#endif  // COMMLIB_MAGIC_INC_PIPEEVENT_H_
