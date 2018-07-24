// Copyright (c) 2013 zhou chang yue. All rights reserved.

#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sstream>
#include "commlib/log/inc/fileHandle.h"
#include "commlib/magic/inc/dir.h"
#include "commlib/log/inc/log.h"

namespace lib {
  namespace log {
    using lib::magic::Dir;
    using std::ostringstream;

    INT32 FileHandle::Logging(const Record & record) {
      if (exact_ && level_ != record.GetLevel()) {
        return 0;
      }

      if (muti_process_write_) {
        if (0 < fd_) {
          ::close(fd_);
          fd_ = -1;
        }

        ScopeLock<FileLock> scope_lock(&file_lock_);
        return WriteRecord(record);
      }

      ScopeLock<Mutex> scope_lock(&mutex_);

      return WriteRecord(record);
    }

    INT32 FileHandle::WriteRecord(const Record & record) {
      if ((mask_ && record.GetMask()) && (level_ <= record.GetLevel())) {
        if (NULL != format_) {
          ostringstream os;
          format_->Formatting(&os, record);
          string message(os.str());
          INT32 result = WriteDataToFile(message);
          if (0 != result) {
            fd_ = -1;
            result = WriteDataToFile(message);
            if (0 != result) {
              RAW_LOG_ERROR("write datas to file:" << file_name_ << " failed err msg:" << ::strerror(errno));
            }
          }
        }  // end of format
      }  // end of mask and level

      return 0;
    }

    VOID FileHandle::DestroyHandle() {
      Handle::DestroyHandle();
    }

    INT32 FileHandle::WriteDataToFile(const string & data) {
      if (NeedCreateFile()) {
        INT32 result = OpenFile();
        if (0 != result) {
          return Err::kERR_FILE_HANDLE_DESC_INVALID;
        }
      }

      INT32 size = data.length();
      INT32 written = 0;
      while ((0 < fd_) && (0 < size)) {
        INT32 once_written = ::write(fd_, data.c_str() + written, size - written);
        if (0 >= once_written) {
          ::close(fd_);
          return Err::kERR_FILE_HANDLE_WRITE_FILE_FAILED;
        }
        written += once_written;
        size -= written;
      }

      if (flush_) {
        ::fsync(fd_);
      }

      off_t currpos = ::lseek(fd_, 0, SEEK_CUR);
      if (file_size_ < currpos) {
        ::close(fd_);
        CHAR str_new[256] = {0};
        ::snprintf(str_new, sizeof(str_new), "%s_%ld", \
                   file_name_.c_str(), ::time(NULL));
        INT32 result = ::rename(file_name_.c_str(), str_new);
        if (0 != result) {
          return Err::kERR_FILE_HANDLE_RENAME_FILE_FAILED;
        }

        result = OpenFile();
        if (0 != result) {
          return Err::kERR_FILE_HANDLE_REOPEN_FILE_FAILED;
        }
      }

      return 0;
    }

    INT32 FileHandle::OpenFile() {
      string new_file_name = GetCurFile();
      if (0 >= (fd_ = ::open(new_file_name.c_str(), O_CREAT|O_WRONLY, S_IRWXU|S_IRGRP|S_IROTH))) {
        return -1;
      }

      cur_file_name_ = new_file_name;

      ::lseek(fd_, 0, SEEK_END);

      return 0;
    }

    BOOL FileHandle::NeedCreateFile() {
      if (0 >= fd_) {
        return TRUE;
      }

      if (1 > granularity_ || 3 < granularity_) {
        return FALSE;
      }

      if (0 == GetCurFile().compare(cur_file_name_)) {
        return FALSE;
      }

      ::close(fd_);
      fd_ = -1;

      return TRUE;
    }

    const string FileHandle::GetCurFile() {
      const string tag = GetCurTag();
      string new_file_name = file_name_;
      SIZE_T pos = string::npos;
      if (1 == granularity_) {
        pos = file_name_.find("{DD}");
      } else if (2 == granularity_) {
        pos = file_name_.find("{HH}");
      } else if (3 == granularity_) {
        pos = file_name_.find("{MM}");
      }

      if (string::npos != pos) {
        new_file_name = new_file_name.replace(pos, 4, tag);
      }

      return new_file_name;
    }

    const string FileHandle::GetCurTag() {
      struct tm tm_time;
      time_t timestamp = ::time(NULL);
      ::localtime_r(&timestamp, &tm_time);
      CHAR timestamp_string[512] = {0};
      ::snprintf(&timestamp_string[0], sizeof(timestamp_string) - 1, \
                 "%04d%02d%02d",                                  \
                 tm_time.tm_year + 1900,                            \
                 tm_time.tm_mon + 1,                                \
                 tm_time.tm_mday);

      if (2 == granularity_) {
        ::snprintf(timestamp_string + ::strlen(timestamp_string), \
                   sizeof(timestamp_string) - ::strlen(timestamp_string), \
                   "_%02d",                                       \
                   tm_time.tm_hour);
      } else if (3 == granularity_) {
        ::snprintf(timestamp_string + ::strlen(timestamp_string), \
                   sizeof(timestamp_string) - ::strlen(timestamp_string), \
                   "_%02d_%02d",                                       \
                   tm_time.tm_hour, tm_time.tm_min);
      } else if (1 == granularity_) {
      } else {
        ::memset(timestamp_string, 0x00, sizeof(timestamp_string));
      }

      return timestamp_string;
    }
  }  // namespace log
}  // namespace lib
