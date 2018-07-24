// Copyright (c) 2013 zhou chang yue. All rights reserved.

#include <string>
#include <vector>
#include "commlib/public/inc/type.h"
#include "commlib/magic/inc/timeFormat.h"
#include "commlib/magic/inc/str.h"
#include "commlib/log/inc/handleManager.h"
#include "commlib/dist/inc/election_cli.h"

using lib::magic::TimeFormat;
using lib::magic::String;

using lib::log::HandleManager;

using lib::dist::Election;
using lib::dist::ElectionCli;

using std::string;
using std::vector;

INT32 main(INT32 argc, CHAR ** argv) {
  if (argc != 2) {
    printf("usage: [config file] [node id]\n");

    return 0;
  }

  INT32 result = HandleManager::GetInstance()->Initial(argv[1]);
  if (0 != result) {
    printf("initial log file failed");
    return -1;
  }

  Election * election = new ElectionCli("127.0.0.1:2181", 30000, "/test");
  result = election->Initial();
  if (0 != result) {
    printf("connect to service:127.0.0.1:2181 failed\n");

    return 0;
  }
/*
  while (TRUE) {
    printf("client running normal ............................\n");
    ::sleep(10);
  }
*/
  delete election;
  delete HandleManager::GetInstance();

  return 0;
}