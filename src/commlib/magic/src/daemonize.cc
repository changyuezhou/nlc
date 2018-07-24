// Copyright (c) 2013 zhou chang yue. All rights reserved.

#include "commlib/magic/inc/daemonize.h"
#include "commlib/magic/inc/ignoreSignal.h"
#include "commlib/magic/inc/log.h"

namespace lib {
  namespace magic {
    INT32 Daemon::Daemonize(INT32 nochdir, INT32 noclose, BOOL force_core, \
                            const CHAR * lock_file, const CHAR *run_as_user) {
      /* already a daemon */
      if (::getppid() == 1) {
        return 0;
      }

      /* run as daemon */
      // don't close standard files here. We will close them after file lock check
      int rc = ::daemon(nochdir, 1);
      if (rc) {
        LIB_MAGIC_LOG_ERROR("Failed to run as daemon, code=" << errno << " (" << ::strerror(errno) << ")");
        return EXIT_FAILURE; /* can not lock */
      }

      IgnoreSignal::IgnoreAllSignal();

      /* Create the lock file as the current user */
      if (lock_file && lock_file[0]) {
        int lfp = ::open(lock_file, O_RDWR | O_CREAT | O_TRUNC, 0640);
        if (lfp < 0) {
          LIB_MAGIC_LOG_ERROR("unable to create lock file " << lock_file << ", code=" \
               << errno << " (" << ::strerror(errno) << ")");
          return EXIT_FAILURE;
        }
        // lock whole file
        struct flock stLock = { F_WRLCK, SEEK_SET, 0, 0, 0 };
        if (::fcntl(lfp, F_SETLK, &stLock) < 0) {
          LIB_MAGIC_LOG_ERROR("unable to lock the lock file " << lock_file \
                        << ", code=" << errno << " (" << ::strerror(errno) << ")");;
          return EXIT_FAILURE; /* can not lock */
        }

        /* write PID to lock file */
        char buf[16] = { 0 };
        ::snprintf(buf, sizeof(buf), "%u", ::getpid());
        ::write(lfp, buf, ::strlen(buf));
      }

      /* Drop user if there is one, and we were run as root */
      if (run_as_user && run_as_user[0] && (::getuid() == 0 || ::geteuid() == 0)) {
        struct passwd pd;
        struct passwd* pwdptr=&pd;
        struct passwd* tempPwdPtr;
        char pwdbuffer[200];
        int  pwdlinelen = sizeof(pwdbuffer);
        if (::getpwnam_r(run_as_user, pwdptr, pwdbuffer, pwdlinelen, &tempPwdPtr)) {
          if (::setuid(pd.pw_uid)) {
            LIB_MAGIC_LOG_ERROR("unable to switch user to " << run_as_user \
                          << ", code=" << errno << " (" << strerror(errno) << ")");
            return EXIT_FAILURE;
          }
        }
      }

      /* Redirect standard files to /dev/null */
      if (!noclose) {
        ::freopen("/dev/null", "r", stdin);
        ::freopen("/dev/null", "w", stdout);
        ::freopen("/dev/null", "w", stderr);
      }

      if (force_core) {
        struct rlimit corelimit;
        // set core size to unlimited
        corelimit.rlim_cur = RLIM_INFINITY;
        corelimit.rlim_max = RLIM_INFINITY;
        if (::setrlimit(RLIMIT_CORE, &corelimit) < 0) {
          LIB_MAGIC_LOG_ERROR("Cannot set core limits to unlimited, error = " \
                        << errno << " (" << ::strerror(errno) << ")");
          return EXIT_FAILURE;
        }

        // force setuid program to make a core dump
        if (::prctl(PR_SET_DUMPABLE, 1) < 0) {
          LIB_MAGIC_LOG_ERROR("Cannot enable core dumping, error = " \
                        << errno << " (" << ::strerror(errno) << ")");
          return EXIT_FAILURE;
        }
      }

      return 0;
    }

    INT32 Daemon::WriteLockFile(const CHAR * lock_file) {
      /* Create the lock file as the current user */
      if (lock_file && lock_file[0]) {
        int lfp = ::open(lock_file, O_RDWR | O_CREAT | O_TRUNC, 0640);
        if (lfp < 0) {
          LIB_MAGIC_LOG_ERROR("unable to create lock file " \
                        << lock_file << ", code=" << errno << " (" << ::strerror(errno) << ")");
          return EXIT_FAILURE;
        }
        // lock whole file
        struct flock stLock = { F_WRLCK, SEEK_SET, 0, 0, 0 };
        if (::fcntl(lfp, F_SETLK, &stLock) < 0) {
          LIB_MAGIC_LOG_ERROR("unable to lock the lock file " \
                        << lock_file << ", code=" << errno << " (" << ::strerror(errno) << ")");
          return EXIT_FAILURE; /* can not lock */
        }

        /* write PID to lock file */
        char buf[16] = { 0 };
        ::snprintf(buf, sizeof(buf), "%u", ::getpid());
        ::write(lfp, buf, ::strlen(buf));
      }

      return 0;
    }
  }  // namespace magic
}  // namespace lib
