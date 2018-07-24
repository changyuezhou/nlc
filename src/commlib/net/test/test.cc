// Copyright (c) 2013 zhou chang yue. All rights reserved.

#include "commlib/public/inc/type.h"

INT32 main(INT32 argc, CHAR ** argv) {
  INT32 women = 0;
  INT32 max = 100;

  for (INT32 i = 1; i <= 100; i++) {
    women = max - i;
  }

  return 0;
}
