// Copyright (c) 2013 zhou chang yue. All rights reserved.

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include "commlib/net/inc/net_addr.h"
#include "commlib/net/inc/dgram.h"
#include "commlib/net/inc/stream.h"

using std::string;
using lib::net::DGramSvr;
using lib::net::StreamSvr;
using lib::net::SockDGram;
using lib::net::SockStream;
using lib::net::UnixStream;
using lib::net::NetAddr;
using lib::net::INetAddr;
using lib::net::UNetAddr;
using lib::net::Sock;

INT32 main(INT32 argc, CHAR ** argv) {
  if (2 > argc) {
    printf("usage: service [addr]\r\n");
    return -1;
  }

  DGramSvr svr;
  INT32 result = svr.Create(argv[1]);
  if (0 != result) {
    printf("create service failed result:%d", result);
    return -1;
  }

  CHAR out[1024] = {0};
  INT32 max_size = sizeof(out);

  NetAddr * net_addr = NULL;
  string addr(argv[1]);

  if (string::npos != addr.find_first_of(":")) {
    net_addr = new INetAddr();
  } else {
    net_addr = new UNetAddr();
  }

  while (TRUE) {
    INT32 size = svr.Recv(out, max_size, net_addr, 1000000);
    if (0 < size) {
      printf("recv from agent %s\r\n", out);
    }

    net_addr->SerialString();
    printf("remote addr is %s\r\n", net_addr->GetAddr().c_str());
    size = svr.Send(out, size, net_addr);

    ::sleep(1);
  }
  delete net_addr;

  return 0;
}
