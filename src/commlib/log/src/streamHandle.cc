// Copyright (c) 2013 zhou chang yue. All rights reserved.

#include "commlib/log/inc/streamHandle.h"

namespace lib {
  namespace log {
    INT32 StreamHandle::Logging(const Record & record) {
      if (exact_ && level_ != record.GetLevel()) {
        return 0;
      }
      ScopeLock<Mutex> scope_lock(&mutex_);
      if ((mask_ && record.GetMask()) && (level_ <= record.GetLevel())) {
        if (NULL != format_) {
          format_->Formatting(out_stream_, record);
        }
      }

      return out_stream_->good()?0:(Err::kERR_STREAM_HANDLE_OBJECT_NOT_GOOD);
    }

    VOID StreamHandle::DestroyHandle() {
      Handle::DestroyHandle();
    }
  }  // namespace log
}  // namespace lib
