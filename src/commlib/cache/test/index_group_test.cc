// Copyright (c) 2013 zhou chang yue. All rights reserved.

#include <string>
#include <vector>
#include "commlib/public/inc/type.h"
#include "commlib/magic/inc/timeFormat.h"
#include "commlib/magic/inc/str.h"
#include "commlib/cache/inc/index_cache.h"
#include "commlib/log/inc/handleManager.h"

using lib::cache::ChunkInfo;
using lib::cache::IndexMemInfo;
using lib::cache::IndexCache;
using lib::magic::TimeFormat;
using lib::magic::String;

using lib::log::HandleManager;

using std::string;
using std::vector;

typedef vector<string> KEYS;

INT32 main(INT32 argc, CHAR ** argv) {
  INT32 result = HandleManager::GetInstance()->Initial("index_group.conf");
  if (0 != result) {
    printf("initial log file failed");
    return -1;
  }

  INT32 size = 1024*1024*20;
  IndexCache cache;
  if (0 != cache.Initial(size)) {
    printf("initial cache failed\n");

    return -1;
  }

  if (0 != cache.Insert("test", "111")) {
    printf("index group obj set test failed\n");
    return -1;
  }

  if (0 != cache.Insert("test", "222")) {
    printf("index group obj set test failed\n");
    return -1;
  }

  if (0 != cache.Insert("test1", "333")) {
    printf("index group obj set test failed\n");
    return -1;
  }

  if (0 != cache.Insert("test2", "444")) {
    printf("index group obj set test failed\n");
    return -1;
  }

  if (0 != cache.Insert("aaa", "000")) {
    printf("index group obj set test failed\n");
    return -1;
  }

  if (0 != cache.Insert("zzz", "555")) {
    printf("index group obj set test failed\n");
    return -1;
  }

  cache.Dump();
/*
  if (0 != index_group_obj.Delete("test", "222")) {
    printf("index group obj set test failed\n");
    return -1;
  }

  if (0 != index_group_obj.Delete("aaa", "000")) {
    printf("index group obj set test failed\n");
    return -1;
  }

  if (0 != index_group_obj.Delete("zzz", "555")) {
    printf("index group obj set test failed\n");
    return -1;
  }

  if (0 != index_group_obj.Delete("test1", "333")) {
    printf("index group obj set test failed\n");
    return -1;
  }

  index_group_obj.Dump();
*/

  KEYS keys;
  cache.GetKeysEQ("test", keys);
  INT32 keys_size = keys.size();
  for (INT32 i = 0; i < keys_size; ++i) {
    printf("eq test key: %s\n", keys[i].c_str());
  }

  keys.clear();
  cache.GetKeysNE("test", keys);
  keys_size = keys.size();
  for (INT32 i = 0; i < keys_size; ++i) {
    printf("ne test key: %s\n", keys[i].c_str());
  }

  keys.clear();
  cache.GetKeysGT("test", keys);
  keys_size = keys.size();
  for (INT32 i = 0; i < keys_size; ++i) {
    printf("gt test key: %s\n", keys[i].c_str());
  }

  keys.clear();
  cache.GetKeysEGT("test", keys);
  keys_size = keys.size();
  for (INT32 i = 0; i < keys_size; ++i) {
    printf("gte test key: %s\n", keys[i].c_str());
  }

  keys.clear();
  cache.GetKeysLT("test", keys);
  keys_size = keys.size();
  for (INT32 i = 0; i < keys_size; ++i) {
    printf("lt test key: %s\n", keys[i].c_str());
  }

  keys.clear();
  cache.GetKeysELT("test", keys);
  keys_size = keys.size();
  for (INT32 i = 0; i < keys_size; ++i) {
    printf("lte test key: %s\n", keys[i].c_str());
  }

  keys.clear();
  cache.GetKeysBE("test", "test2", keys);
  keys_size = keys.size();
  for (INT32 i = 0; i < keys_size; ++i) {
    printf("be test and test2 key: %s\n", keys[i].c_str());
  }

  keys.clear();
  KEYS index_keys;
  index_keys.push_back("test");
  index_keys.push_back("aaa");
  cache.GetKeysIN(index_keys, keys);
  keys_size = keys.size();
  for (INT32 i = 0; i < keys_size; ++i) {
    printf("in test and aaa key: %s\n", keys[i].c_str());
  }

  keys.clear();
  cache.GetKeysNotIN(index_keys, keys);
  keys_size = keys.size();
  for (INT32 i = 0; i < keys_size; ++i) {
    printf("not in test and aaa key: %s\n", keys[i].c_str());
  }

  delete HandleManager::GetInstance();

  return 0;
}

