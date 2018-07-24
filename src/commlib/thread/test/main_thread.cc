// Copyright (c) 2013 zhou chang yue. All rights reserved.

#include <stdio.h>
#include <unistd.h>
#include "commlib/public/inc/type.h"
#include "commlib/thread/inc/thread.h"

using lib::thread::Thread;

class A : public Thread {
 public:
   A() {}
   virtual ~A() {}

 public:
   virtual INT32 Working(VOID * parameter);
};

INT32 A::Working(VOID * parameter) {
  INT32 i = 0;
  while (TRUE) {
    i++;
  }

  if (0 < i) {
    printf("11223\n");
  }

  return 0;
}

INT32 main(INT32 argc, CHAR ** argv) {
  A a;
  A b;
  A c;
  A d;
  A e;
  A f;

  a.Running(NULL, 1024);
  b.Running(NULL, 1024);
  c.Running(NULL, 1024);
  d.Running(NULL, 1024);
  e.Running(NULL, 1024);
  f.Running(NULL, 1024);

  while (TRUE) {
    ::sleep(2);
  }

  return 0;
}
