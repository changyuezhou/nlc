// Copyright (c) 2013 zhou chang yue. All rights reserved.

#ifndef COMMLIB_LOG_INC_FILEHANDLE_H_
#define COMMLIB_LOG_INC_FILEHANDLE_H_

#include <string>
#include <ostream>
#include <iostream>
#include "commlib/public/inc/type.h"
#include "commlib/log/inc/record.h"
#include "commlib/log/inc/handle.h"
#include "commlib/magic/inc/fileLock.h"

namespace lib {
  namespace log {
    using lib::magic::FileLock;

    class FileHandle:public Handle {
     public:
       FileHandle(Format * format, \
                  LEVEL level, BOOL exact, UINT64 mask, \
                  const string & file, INT32 size, \
                  INT32 granularity, BOOL flush, \
                  BOOL muti_process_write = FALSE, \
                  const string & lock_file = ""):Handle(format, level, exact, mask), \
                  file_name_(file), lock_file_(lock_file), \
                  cur_file_name_(""), file_size_(size*1024*1024), \
                  granularity_(granularity), flush_(flush), \
                  muti_process_write_(muti_process_write), \
                  file_lock_(lock_file_) {}
       virtual ~FileHandle() {}

     public:
       virtual INT32 Logging(const Record & record);
       virtual VOID DestroyHandle();

     public:
       INT32 WriteRecord(const Record & record);

     protected:
       INT32 WriteDataToFile(const string & data);
       INT32 OpenFile();
       BOOL NeedCreateFile();
       const string GetCurTag();
       const string GetCurFile();

     protected:
       string file_name_;
       string lock_file_;
       string cur_file_name_;
       INT32 file_size_;
       INT32 granularity_;
       BOOL flush_;
       BOOL muti_process_write_;
       FileLock file_lock_;
    };
  }  // namespace log
}  // namespace lib

#endif  // COMMLIB_LOG_INC_FILEHANDLE_H_
