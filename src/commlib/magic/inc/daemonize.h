// Copyright (c) 2013 zhou chang yue. All rights reserved.

#ifndef COMMLIB_MAGIC_INC_DAEMONIZE_H_
#define COMMLIB_MAGIC_INC_DAEMONIZE_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <sys/prctl.h>
#include <fcntl.h>
#include <errno.h>
#include <pwd.h>
#include <signal.h>
#include <string>
#include "commlib/public/inc/type.h"

namespace lib {
  namespace magic {
    class Daemon {
     public:
       static INT32 Daemonize(INT32 nochdir, INT32 noclose, BOOL force_core, \
                              const CHAR * lock_file = NULL, const CHAR *run_as_user = NULL);
       static INT32 WriteLockFile(const CHAR * lock_file);
    };
  }  // namespace magic
}  // namespace lib

#endif  // COMMLIB_MAGIC_INC_DAEMONIZE_H_
