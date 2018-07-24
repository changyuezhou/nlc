// Copyright (c) 2013 zhou chang yue. All rights reserved.

#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include "commlib/net/inc/dgram.h"
#include "commlib/net/inc/stream.h"
#include "commlib/net/inc/sock_pool.h"
#include "commlib/net/inc/ep.h"

using lib::net::DGramAgent;
using lib::net::StreamAgent;
using lib::net::SockDGram;
using lib::net::SockStream;
using lib::net::EP;
using lib::net::SockPool;

INT32 main(INT32 argc, CHAR ** argv) {
  if (2 > argc) {
    printf("usage: cs [addr]\r\n");
    return -1;
  }

  SockPool sock_pool;

  if (0 < size) {
    printf("recv from service %s\r\n", out);
  }

  return 0;
}
