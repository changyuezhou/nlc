// Copyright (c) 2013 zhou chang yue. All rights reserved.

#ifndef COMMLIB_LOG_INC_RECORD_H_
#define COMMLIB_LOG_INC_RECORD_H_

#include <strings.h>
#include <string>
#include <iostream>
#include <ostream>
#include "commlib/public/inc/type.h"

namespace lib {
  namespace log {
    using std::string;
    using std::ostream;
    using std::endl;

    struct Record {
     public:
       enum LEVEL {
         SPECIAL = 1,
         DEBUG = 10,
         INFO = 11,
         WARN = 12,
         ERROR = 13,
         FATAL = 14,
         ALL
       };

     public:
       Record(UINT64 mask, const string & level, const string & file, \
              INT32 line, pid_t pid, pthread_t pth_id,                \
              time_t timestamp, const string & msg):                  \
              mask_(mask), level_(DEBUG), file_name_(file),           \
              line_(line), timestamp_(timestamp),                     \
              process_id_(pid), pthread_id_(pth_id), \
              message_(msg) {
                if (0 == ::strncasecmp("SPECIAL", level.c_str(), 7)) {
                  level_ = SPECIAL;
                } else if (0 == ::strncasecmp("DEBUG", level.c_str(), 5)) {
                  level_ = DEBUG;
                } else if (0 == ::strncasecmp("INFO", level.c_str(), 4)) {
                  level_ = INFO;
                } else if (0 == ::strncasecmp("WARN", level.c_str(), 4)) {
                  level_ = WARN;
                } else if (0 == ::strncasecmp("ERROR", level.c_str(), 5)) {
                  level_ = ERROR;
                } else if (0 == ::strncasecmp("FATAL", level.c_str(), 5)) {
                  level_ = FATAL;
                }
       }
       ~Record() {}

     public:
       const string & GetMessage() const {
         return message_;
       }

       time_t GetTimestamp() const {
         return timestamp_;
       }

       const string GetLevelStr() const {
         string level = "DEBUG";
         switch (level_) {
         case SPECIAL:
           level = "SPECIAL";
           break;
         case DEBUG:
           level = "DEBUG";
           break;
         case INFO:
           level = "INFO";
           break;
         case WARN:
           level = "WARN";
           break;
         case ERROR:
           level = "ERROR";
           break;
         case FATAL:
           level = "FATAL";
           break;
         case ALL:
           level = "ALL";
           break;
         }
         return level;
       }

       LEVEL GetLevel() const {
         return level_;
       }

       const string & GetFileName() const {
         return file_name_;
       }

       INT32 GetLine() const {
         return line_;
       }

       pid_t GetProcessId() const {
         return process_id_;
       }

       pthread_t GetThreadId() const {
         return pthread_id_;
       }

       UINT64 GetMask() const {
         return mask_;
       }

     public:
       const Record & operator=(const Record & record) {
         this->level_ = record.GetLevel();
         this->file_name_ = record.GetFileName();
         this->process_id_ = record.GetProcessId();
         this->pthread_id_ = record.GetThreadId();
         this->line_ = record.GetLine();
         this->message_ = record.GetMessage();
         this->timestamp_ = record.GetTimestamp();
         this->mask_ = record.GetMask();

         return *this;
       }

     private:
       UINT64 mask_;
       LEVEL level_;
       string file_name_;
       INT32 line_;
       time_t timestamp_;
       pid_t process_id_;
       pthread_t pthread_id_;
       string message_;
    };

    const ostream & operator<<(ostream & os, const Record & record);
  }  // namespace log
}  // namespace lib

#endif  // COMMLIB_LOG_INC_RECORD_H_
