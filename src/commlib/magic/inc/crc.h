// Copyright (c) 2013 zhou chang yue. All rights reserved.

#ifndef COMMLIB_MAGIC_INC_CRC_H_
#define COMMLIB_MAGIC_INC_CRC_H_

#include <string>
#include "commlib/public/inc/type.h"

namespace lib {
  namespace magic {
    class CRC {
     public:
       static UINT32 CRC32(const UCHAR * data, INT32 size);
    };
  }  // namespace magic
}  // namespace lib

#endif  // COMMLIB_MAGIC_INC_CRC_H_