// Copyright (c) 2013 zhou chang yue. All rights reserved.

#ifndef COMMLIB_MAGIC_INC_MONITOR_H_
#define COMMLIB_MAGIC_INC_MONITOR_H_

#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <string>
#include "commlib/public/inc/type.h"
#include "commlib/magic/inc/ignoreSignal.h"
#include "commlib/magic/inc/daemonize.h"
#include "commlib/magic/inc/singletonHolder.h"

namespace lib {
  namespace magic {
    using std::string;

    template<class T>
    class Monitor {
     public:
       Monitor() {}
       ~Monitor() {}

     public:
       VOID Start();
       static VOID Restart(INT32 sig);
    };

    template<class T>
    VOID Monitor<T>::Start() {
      pid_t pid = -1;

      pid = ::fork();
      if (0 == pid) {
        IgnoreSignal::IgnoreDaemon();
        ::signal(SIGCHLD, Monitor<T>::Restart);

        pid = ::fork();
        if (0 == pid) {
          IgnoreSignal::IgnoreAllSignal();
          T work;
          work.Run();
        }

        while (TRUE) {
          ::sleep(60);
        }
      }
    }

    template<class T>
    VOID Monitor<T>::Restart(INT32 sig) {
      pid_t pid = -1;
      INT32 status;

      pid = ::wait(&status);
      if (0 < pid) {
        pid = ::fork();
        if (0 == pid) {
          IgnoreSignal::IgnoreAllSignal();
          T work;
          work.Run();
        }
      }
    }
  }  // namespace magic
}  // namespace lib

#endif  // COMMLIB_MAGIC_INC_MONITOR_H_
