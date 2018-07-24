// Copyright (c) 2013 zhou chang yue. All rights reserved.

#include <string>
#include <vector>
#include "commlib/public/inc/type.h"
#include "commlib/magic/inc/timeFormat.h"
#include "commlib/magic/inc/str.h"
#include "commlib/cache/inc/cache.h"
#include "commlib/cache/inc/node_group.h"
#include "commlib/log/inc/handleManager.h"

using lib::cache::ChunkInfo;
using lib::cache::NodeMemInfo;
using lib::cache::Cache;
using lib::magic::TimeFormat;
using lib::magic::String;

using lib::log::HandleManager;

using std::string;
using std::vector;

INT32 main(INT32 argc, CHAR ** argv) {
  INT32 result = HandleManager::GetInstance()->Initial("node_group.conf");
  if (0 != result) {
    printf("initial log file failed");
    return -1;
  }

  INT32 size = 1024*1024*20;
  Cache cache;
  if (0 != cache.Initial(size)) {
    printf("initial cache failed\n");

    return -1;
  }

  if (0 != cache.Set("test", "111", 3)) {
    printf("cache set test failed\n");
    return -1;
  }
  cache.Dump();
/*
  CHAR buf[1024] = {0};
  INT32 max_size = sizeof(buf);
  if (0 != cache.Get("test", buf, &max_size)) {
    printf("cache get test failed\n");
  }
*/
  printf("delete key:test success\n");

  delete HandleManager::GetInstance();

  return 0;
}

