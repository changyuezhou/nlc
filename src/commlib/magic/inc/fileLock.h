// Copyright (c) 2013 zhou chang yue. All rights reserved.

#ifndef COMMLIB_MAGIC_INC_FILELOCK_H_
#define COMMLIB_MAGIC_INC_FILELOCK_H_

#include <unistd.h>
#include <fcntl.h>
#include <string>
#include "commlib/public/inc/type.h"
#include "commlib/magic/inc/log.h"

namespace lib {
  namespace magic {
    using std::string;

    class FileLock {
     public:
       FileLock():file_(""), file_fd_(-1) {}
       explicit FileLock(const string & file):file_(file), file_fd_(-1) {}
       ~FileLock() { Destroy(); }

     public:
       INT32 CreateFileLock();
       INT32 CreateFileLock(const string &file);

       BOOL IsOpen();

       inline BOOL ReadLock(INT32 offset, INT32 len, INT32 whence) {
         return ControlFile(F_SETLK, F_RDLCK, offset, whence, len);
       }

       inline BOOL WriteLock(INT32 offset, INT32 len, INT32 whence) {
         return ControlFile(F_SETLK, F_WRLCK, offset, whence, len);
       }

       inline BOOL ReadLockW(INT32 offset, INT32 len, INT32 whence) {
         return ControlFile(F_SETLKW, F_RDLCK, offset, whence, len);
       }

       inline BOOL WriteLockW(INT32 offset, INT32 len, INT32 whence) {
         return ControlFile(F_SETLKW, F_WRLCK, offset, whence, len);
       }

       inline BOOL ReleaseLock(INT32 offset, INT32 len, INT32 whence) {
         return ControlFile(F_SETLK, F_UNLCK, offset, whence, len);
       }

       inline INT32 Lock() {
         if (WriteLock(0, 0, SEEK_SET)) {
           return 0;
         }
         return -1;
       }

       inline INT32 LockW() {
         if (WriteLockW(0, 0, SEEK_SET)) {
           return 0;
         }
         return -1;
       }

       inline INT32 SharedLock() {
         if (ReadLock(0, 0, SEEK_SET)) {
           return 0;
         }
         return -1;
       }

       inline INT32 SharedLockW() {
         if (ReadLockW(0, 0, SEEK_SET)) {
           return 0;
         }
         return -1;
       }

       inline INT32 UnLock() {
         if (ReleaseLock(0, 0, SEEK_SET)) {
           return 0;
         }

         return -1;
       }

     public:
       VOID Destroy() {
         if (0 < file_fd_) {
           ::close(file_fd_);
         }
       }

     private:
       BOOL ControlFile(INT32 cmd, INT32 type, off_t offset, INT32 whence, off_t len);

     private:
       string file_;
       INT32 file_fd_;
    };
  }  // namespace magic
}  // namespace lib

#endif  // COMMLIB_MAGIC_INC_FILELOCK_H_
