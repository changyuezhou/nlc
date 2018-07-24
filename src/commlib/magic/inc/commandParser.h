// Copyright (c) 2013 zhou chang yue. All rights reserved.

#ifndef COMMLIB_MAGIC_INC_COMMANDPARSER_H_
#define COMMLIB_MAGIC_INC_COMMANDPARSER_H_

#include <string>
#include "commlib/public/inc/type.h"

namespace lib {
  namespace magic {
    using std::string;

    class CommandParser {
     public:
       static const string GetCommandValue(CHAR  type, INT32 num, CHAR ** cmd);
    };
  }  // namespace magic
}  // namespace lib

#endif  // COMMLIB_MAGIC_INC_COMMANDPARSER_H_
