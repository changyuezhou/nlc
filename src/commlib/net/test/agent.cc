// Copyright (c) 2013 zhou chang yue. All rights reserved.

#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include "commlib/net/inc/stream.h"

using lib::net::DGramAgent;
using lib::net::StreamAgent;
using lib::net::SockDGram;
using lib::net::SockStream;

INT32 main(INT32 argc, CHAR ** argv) {
  if (2 > argc) {
    printf("usage: agent [addr]\r\n");
    return -1;
  }

  // DGramAgent agent;
  StreamAgent agent;
  CHAR out[1024] = {0};
  INT32 max_size = sizeof(out);
  struct timeval tv;
  ::memset(&tv, 0x00, sizeof(tv));
  agent.Send("123", 3, argv[1]);
  ::gettimeofday(&tv, NULL);
  ::printf("**** start sec:%ld usec:%ld\r\n", tv.tv_sec, tv.tv_usec);
  INT32 size = agent.Recv(out, max_size, 2000);
  ::gettimeofday(&tv, NULL);
  ::printf("**** end sec:%ld usec:%ld\r\n", tv.tv_sec, tv.tv_usec);

  if (0 < size) {
    printf("recv from service %s\r\n", out);
  }

  return 0;
}
