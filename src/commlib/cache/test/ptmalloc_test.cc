// Copyright (c) 2013 zhou chang yue. All rights reserved.

#include <string>
#include <vector>
#include "commlib/public/inc/type.h"
#include "commlib/magic/inc/timeFormat.h"
#include "commlib/magic/inc/str.h"
#include "commlib/cache/inc/ptmalloc.h"
#include "commlib/log/inc/handleManager.h"

using lib::cache::PTMalloc;
using lib::cache::ChunkInfo;
using lib::magic::TimeFormat;
using lib::magic::String;

using lib::log::HandleManager;

using std::string;
using std::vector;

INT32 main(INT32 argc, CHAR ** argv) {
  INT32 result = HandleManager::GetInstance()->Initial("ptmalloc.conf");
  if (0 != result) {
    printf("initial log file failed");
    return -1;
  }

  INT32 size = 1024*1024*4;
  CHAR * mem = reinterpret_cast<CHAR *>(::malloc(size));
  if (NULL == mem) {
    printf("memory malloc failed size: %d\n", size);

    return 0;
  }

  memset(mem, 0x00, size);
  ChunkInfo * chunk_info = reinterpret_cast<ChunkInfo *>(mem);
  chunk_info->bottom_ = sizeof(ChunkInfo);
  chunk_info->chunk_start_ = chunk_info->bottom_;
  chunk_info->top_ = size;
  chunk_info->size_ = chunk_info->top_ - chunk_info->bottom_;

  PTMalloc malloc(chunk_info);

  malloc.Dump();
  /*
  for (INT32 i = 0; i < 3; ++i) {
    INT32 alloc_size = 0;
    if (i < 2) {
      alloc_size = ChunkInfo::MID_LARGE_BINS_MAX_BYTE - 16 + 8 * (i + 1);
    } else {
      alloc_size = ChunkInfo::MID_LARGE_BINS_MAX_BYTE - 16 + 8 * 1;
    }
    //INT32 alloc_size = 8*(i+1);
    printf("malloc size: %d \n", alloc_size);
    VOID * p = malloc.malloc(alloc_size);
    if (NULL == p) {
      printf("malloc size: %d failed\n", alloc_size);
    } else {
      malloc.free(p);
    }
  }
  */

  VOID * p[63] = {0};

  for (INT32 i = 0; i < 3; ++i) {
    INT32 alloc_size = 8*(i+1);
    printf("malloc size: %d \n", alloc_size);
    p[i] = malloc.malloc(alloc_size);
    if (NULL == p) {
      printf("malloc size: %d failed\n", alloc_size);
    } else {
      // malloc.free(p);
    }
  }

  INT32 alloc_size = 1000;
  printf("malloc size: %d \n", alloc_size);
  malloc.malloc(alloc_size);

  for (INT32 i = 0; i < 3; ++i) {
    if (i != 1) {
      malloc.free(p[i]);
    }
  }

  malloc.Dump();

  malloc.MergeFragment();

  malloc.Dump();
  malloc.DumpSeq();
  ::free(mem);

  return 0;
}