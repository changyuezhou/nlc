// Copyright (c) 2013 zhou chang yue. All rights reserved.

#include <stdio.h>
#include <unistd.h>
#include "commlib/magic/inc/commandParser.h"

namespace lib {
  namespace magic {
    const string CommandParser::GetCommandValue(CHAR type, INT32 num, CHAR ** cmd) {
      CHAR opt_string[512] = {0};
      string value = "";
      ::snprintf(opt_string, sizeof(opt_string), "-%c:", type);
      CHAR ch = -1;
      if (type == (ch = ::getopt(num, cmd, opt_string))) {
        value = optarg;
      }

      return value;
    }
  }  // namespace magic
}  // namespace lib
