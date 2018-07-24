// Copyright (c) 2013 zhou chang yue. All rights reserved.

#include "commlib/magic/inc/fileLock.h"

namespace lib {
  namespace magic {
    INT32 FileLock::CreateFileLock() {
      file_fd_ = ::open(file_.c_str(), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
      if (0 >= file_fd_) {
        LIB_MAGIC_LOG_ERROR("FileLock open failed file " << file_);
        return -1;
      }
      return 0;
    }

    INT32 FileLock::CreateFileLock(const string & file) {
      file_fd_ = ::open(file.c_str(), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
      if (0 >= file_fd_) {
        LIB_MAGIC_LOG_ERROR("FileLock open failed file " << file_);
        return -1;
      }
      file_ = file;
      return 0;
    }

    BOOL FileLock::IsOpen() {
      return (file_fd_ > 0);
    }

    BOOL FileLock::ControlFile(INT32 cmd, INT32 type, off_t offset, INT32 whence, off_t len) {
      if (!IsOpen()) {
        if (0 != CreateFileLock()) {
          return FALSE;
        }
      }
      struct flock lock;
      lock.l_type = type;  // F_RDLCK, F_WRLCK, F_UNLCK
      lock.l_start = offset;
      lock.l_whence = whence;  // SEEK_SET, SEEK_CUR, SEEK_END
      lock.l_len = len;

      if (0 > fcntl(file_fd_, cmd, &lock)) {
        return FALSE;
      }

      return TRUE;
    }
  }  // namespace magic
}  // namespace lib
