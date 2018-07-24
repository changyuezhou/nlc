// Copyright (c) 2013 zhou chang yue. All rights reserved.

#include <sys/socket.h>
#include <sys/fcntl.h>
#include "commlib/magic/inc/pipeEvent.h"
#include "commlib/magic/inc/err.h"
#include "commlib/magic/inc/log.h"

namespace lib {
  namespace magic {
    INT32 PipeEvent::CreateEvent() {
      if (0 != ::pipe(pipe_)) {
        LIB_MAGIC_LOG_ERROR("create pipe failed err msg:" << ::strerror(errno));
        return Err::kERR_PIPE_CREATE;
      }

      INT32 ret = SetNBlock(pipe_[0]);
      if (0 != ret) {
        return ret;
      }
      ret = SetNBlock(pipe_[1]);
      if (0 != ret) {
        return ret;
      }

      return 0;
    }

    INT32 PipeEvent::WaitForSignal(INT32 timeout) {
      fd_set rdset;
      FD_ZERO(&rdset);
      FD_SET(pipe_[0], &rdset);

      struct timeval tm;
      tm.tv_sec = timeout/1000;
      tm.tv_usec = (timeout%1000)*1000;

      if (kINFINITE == timeout) {
        ::select(pipe_[0]+1, &rdset, NULL, NULL, NULL);
      } else {
        ::select(pipe_[0]+1, &rdset, NULL, NULL, &tm);
      }

      CHAR buf[32] = {0};
      ::read(pipe_[0], buf, sizeof(buf));

      return 0;
    }

    INT32 PipeEvent::PostSignal() {
      return (1 == ::write(pipe_[1], "I", 1)?0:-1);
    }

    INT32 PipeEvent::SetNBlock(INT32 fd) {
      INT32 flag;
      flag = ::fcntl(fd, F_GETFL, 0);
      flag |= O_NONBLOCK;
      flag |= O_NDELAY;

      if (0 != ::fcntl(fd, F_SETFL, flag)) {
        LIB_MAGIC_LOG_ERROR("set pipe nonblock failed err msg:" << ::strerror(errno));
        return Err::kERR_PIPE_SET_NONBLOCK;
      }

      return 0;
    }
  }  // namespace magic
}  // namespace lib
