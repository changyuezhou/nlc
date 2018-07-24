// Copyright (c) 2013 zhou chang yue. All rights reserved.

#include "commlib/public/inc/type.h"
#include "commlib/log/inc/log.h"
#include "commlib/log/inc/handleManager.h"

using lib::log::HandleManager;

INT32 main(INT32 argc, CHAR ** argv) {
  INT32 result = HandleManager::GetInstance()->Initial("test.conf");
  if (0 != result) {
    printf("initial log file failed");
    return -1;
  }

  LOG_TEST_DEBUG("test log ......");
  LOG_TEST_INFO("test log ......");
  LOG_TEST_WARN("test log ......");
  LOG_TEST_ERROR("test log ......");
  LOG_TEST_FATAL("test log ......");

  return 0;
}
