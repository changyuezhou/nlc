// Copyright (c) 2013 zhou chang yue. All rights reserved.

#ifndef COMMLIB_CACHE_INC_ALLOCATOR_H_
#define COMMLIB_CACHE_INC_ALLOCATOR_H_

#include <string>
#include "commlib/public/inc/type.h"
#include "commlib/cache/inc/err.h"

namespace lib {
  namespace cache {
    class Allocator {
     public:
       Allocator() {}
       virtual ~Allocator() {}

     public:
       virtual VOID * malloc(INT32 size) = 0;
       virtual VOID free(VOID * address) = 0;
    };
  }
}

#endif  // COMMLIB_CACHE_INC_ALLOCATOR_H_