// Copyright (c) 2013 zhou chang yue. All rights reserved.

#ifndef COMMLIB_CACHE_INC_CHUNK_H_
#define COMMLIB_CACHE_INC_CHUNK_H_

#include <string>
#include "commlib/public/inc/type.h"
#include "commlib/cache/inc/err.h"

namespace lib {
  namespace cache {
    typedef struct _chunk {
      INT32 prev_size_;
      INT32 size_;
      INT32 prev_;
      INT32 next_;
    } Chunk;

    typedef struct _chunk_info {
      static const INT32 SORT_BINS_COUNT = 127;
      static const INT32 SORT_BINS_MAX_BYTE = 1024;
      static const INT32 SORT_BINS_STEP = 8;
      static const INT32 SMALL_BINS_COUNT = 63;
      static const INT32 SMALL_BINS_MIN_BYTE = sizeof(Chunk);  // 16
      static const INT32 SMALL_BINS_MAX_BYTE = SMALL_BINS_MIN_BYTE + SORT_BINS_STEP*SMALL_BINS_COUNT; // 520
      static const INT32 LARGE_BINS_COUNT = SORT_BINS_COUNT - SMALL_BINS_COUNT;  // 64
      static const INT32 LARGE_BINS_MIN_BYTE = SMALL_BINS_MAX_BYTE;  // 520
      static const INT32 LARGE_BINS_MAX_BYTE = LARGE_BINS_MIN_BYTE + LARGE_BINS_COUNT*SORT_BINS_STEP; // 1032
      static const INT32 FAST_BINS_COUNT = SMALL_BINS_COUNT;  // 63
      static const INT32 FAST_BINS_MIN_BYTE = SMALL_BINS_MIN_BYTE;  // 16
      static const INT32 FAST_BINS_MAX_BYTE = FAST_BINS_MIN_BYTE + FAST_BINS_COUNT*SORT_BINS_STEP;  // 520
      static const INT32 MID_LARGE_BINS_COUNT = 127;
      static const INT32 MID_LARGE_BINS_STEP = 64;
      static const INT32 MID_LARGE_BINS_MIN_BYTE = LARGE_BINS_MAX_BYTE;  // 1032
      static const INT32 MID_LARGE_BINS_SEARCH_STEP = 8;
      static const INT32 MID_LARGE_BINS_MAX_BYTE = MID_LARGE_BINS_MIN_BYTE + MID_LARGE_BINS_COUNT*MID_LARGE_BINS_STEP; // 9160
      static const INT32 UNSORT_LARGE_BINS_MIN_BYTE = MID_LARGE_BINS_MAX_BYTE;  // 9160

      INT64 flag_;
      pthread_mutex_t mutex_;
      INT64 size_;
      INT64 sort_bins_[SORT_BINS_COUNT];
      INT64 fast_bins_[FAST_BINS_COUNT];
      INT64 mid_large_bins_[MID_LARGE_BINS_COUNT];
      INT64 unsorted_bins_;
      INT64 chunk_start_;
      INT64 top_;
      INT64 bottom_;
    } ChunkInfo;
  }
}

#endif  // COMMLIB_CACHE_INC_CHUNK_H_