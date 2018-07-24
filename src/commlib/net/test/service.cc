// Copyright (c) 2013 zhou chang yue. All rights reserved.

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include "commlib/net/inc/net_addr.h"
#include "commlib/net/inc/sock.h"


using std::string;
using lib::net::SockStream;
using lib::net::UnixStream;
using lib::net::NetAddr;
using lib::net::Sock;

INT32 main(INT32 argc, CHAR ** argv) {
  if (2 > argc) {
    printf("usage: service [addr]\r\n");
    return -1;
  }

  SockStream * sock = new SockStream();

      NetAddr local;
      local.Serialize(argv[1]);
      INT32 result = 0;
      if (NULL == sock || 0 != (result = sock->Listen(local, 5))) {
        printf("listen failed addr:%s\r\n", argv[1]);
        return 0;
      }
      /*  
  INT32 result = svr.Create(argv[1]);
  if (0 != result) {
    printf("create service failed result:%d", result);
    return -1;
  }

  SockStream * sock = NULL;
  NetAddr * net_addr = NULL;
  string addr(argv[1]);

  if (string::npos != addr.find_first_of(":")) {
    net_addr = new INetAddr();
    sock = new SockStream();
  } else {
    net_addr = new UNetAddr();
    sock = new UnixStream();
  }

  while (TRUE) {
    if (0 == svr.GetSockStream(sock, net_addr, Sock::INFINITE)) {
      printf("recv from %s connecting handle %d\r\n", net_addr->GetAddr().c_str(), sock->Handle());

      CHAR in[1024] = {0};
      INT32 size = sizeof(in);

      size = sock->Read(in, size, 2000);
      if (0 < size) {
        printf("recv data size %d content is %s\r\n", size, in);
        sock->Write(in, size);
        sock->Close();
      } else {
        sock->Close();
      }
    }

    ::usleep(100);
  }
  */

  delete sock;

  return 0;
}
