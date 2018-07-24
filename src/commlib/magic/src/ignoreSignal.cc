// Copyright (c) 2013 zhou chang yue. All rights reserved.

#include "commlib/magic/inc/ignoreSignal.h"

namespace lib {
  namespace magic {
    VOID IgnoreSignal::IgnoreAllSignal() {
      ::signal(SIGTSTP, SIG_IGN);
      ::signal(SIGHUP, SIG_IGN);
      ::signal(SIGQUIT, SIG_IGN);
      ::signal(SIGPIPE, SIG_IGN);
      ::signal(SIGTTOU, SIG_IGN);
      ::signal(SIGTTIN, SIG_IGN);
      ::signal(SIGSTOP, SIG_IGN);
      ::signal(SIGTERM, SIG_IGN);
      ::signal(SIGINT, SIG_IGN);

      ::signal(SIGFPE, SIG_IGN);
      ::signal(SIGKILL, SIG_IGN);
      ::signal(SIGBUS, SIG_IGN);
      ::signal(SIGSEGV, SIG_IGN);
      ::signal(SIGSYS, SIG_IGN);
      ::signal(SIGALRM, SIG_IGN);

      ::signal(SIGURG, SIG_IGN);
      ::signal(SIGCONT, SIG_IGN);
      ::signal(SIGCHLD, SIG_IGN);
      ::signal(SIGIO, SIG_IGN);
      ::signal(SIGXCPU, SIG_IGN);
      ::signal(SIGXFSZ, SIG_IGN);

      ::signal(SIGWINCH, SIG_IGN);
      ::signal(SIGPWR, SIG_IGN);
      ::signal(SIGPROF, SIG_IGN);

      ::signal(SIGVTALRM, SIG_IGN);
    }

    VOID IgnoreSignal::IgnoreDaemon() {
      ::signal(SIGTSTP, SIG_IGN);
      ::signal(SIGHUP, SIG_IGN);
      ::signal(SIGQUIT, SIG_IGN);
      ::signal(SIGPIPE, SIG_IGN);
      ::signal(SIGTTOU, SIG_IGN);
      ::signal(SIGTTIN, SIG_IGN);
      ::signal(SIGSTOP, SIG_IGN);
      ::signal(SIGTERM, SIG_IGN);
      ::signal(SIGINT, SIG_IGN);
    }
  }  // namespace magic
}  // namespace lib
