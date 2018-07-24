// Copyright (c) 2013 zhou chang yue. All rights reserved.

#ifndef COMMLIB_MAGIC_INC_TSC_H_
#define COMMLIB_MAGIC_INC_TSC_H_

#include <string>
#include "commlib/public/inc/type.h"

namespace lib {
  namespace magic {
    class Tsc {
     public:
       inline static UINT64 Rtdscll() {
         UINT64 value = 0;
         do {
           UINT32 __a, __d;
           asm volatile("rdtsc" : "=a"(__a), "=d"(__d));
           (value) = ((UINT64)(__a)) | (((UINT64)__d) << 32);
         } while (0);

         return value;
       }

       inline static INT32 GetCpuRealClock() {
         UINT32 khz;
         UINT64 tsc0 = 0, tsc1 = 0, tsc2 = 0;
         struct timespec tv;
         tsc0 = Rtdscll();
         tv.tv_sec = 1;
         tv.tv_nsec = 1000000;
         while (0 != nanosleep(&tv, &tv)) {}
         tsc1 = Rtdscll();
         tv.tv_sec = 0;
         tv.tv_nsec = 1000000;
         while (0 != nanosleep(&tv, &tv)) {}
         tsc2 = Rtdscll();
         khz = (2*tsc1 - tsc0 - tsc2)/1000;
         return khz;
       }
    };
  }  // namespace magic
}  // namespace lib

#endif  // COMMLIB_MAGIC_INC_TSC_H_
