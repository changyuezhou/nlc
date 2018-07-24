// Copyright (c) 2012 zhou chang yue. All rights reserved.

#ifndef COMMLIB_MAGIC_INC_STAT_H_
#define COMMLIB_MAGIC_INC_STAT_H_

#include <string>
#include "commlib/public/inc/type.h"
#include "commlib/magic/inc/atomic.h"

namespace lib {
  namespace magic {
    class Stat {
     public:
       Stat() {
         stat_.counter = 0;
         tsc_.counter = 0;
       }
       ~Stat() {}

     public:
       VOID Add(INT32 count);
       VOID Add();
       INT32 GetCount() const;
       VOID Clear();
       VOID AddTsc(INT32 count);
       INT32 GetTscCount() const;

     private:
       atomic_t stat_;
       atomic_t tsc_;
    };
  }  // namespace magic
}  // namespace lib

#endif  // COMMLIB_MAGIC_INC_STAT_H_
