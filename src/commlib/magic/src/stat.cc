// Copyright (c) 2012 zhou chang yue. All rights reserved.

#include <string>
#include "commlib/magic/inc/stat.h"

namespace lib {
  namespace magic {
    VOID Stat::Add(INT32 count) {
      atomic_add(count, &stat_);
    }

    VOID Stat::Add() {
      atomic_inc(&stat_);
    }

    INT32 Stat::GetCount() const {
      return stat_.counter;
    }

    INT32 Stat::GetTscCount() const {
      return tsc_.counter;
    }

    VOID Stat::AddTsc(INT32 count) {
      atomic_add(count, &tsc_);
    }

    VOID Stat::Clear() {
      stat_.counter = 0;
      tsc_.counter = 0;
    }
  }  // namespace magic
}  // namespace lib
