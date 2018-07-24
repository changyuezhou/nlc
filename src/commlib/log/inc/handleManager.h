// Copyright (c) 2013 zhou chang yue. All rights reserved.

#ifndef COMMLIB_LOG_INC_HANDLEMANAGER_H_
#define COMMLIB_LOG_INC_HANDLEMANAGER_H_

#include <string>
#include <ostream>
#include <iostream>
#include <vector>
#include "commlib/public/inc/type.h"
#include "commlib/etc/inc/conf.h"
#include "commlib/log/inc/record.h"
#include "commlib/log/inc/handle.h"

namespace lib {
  namespace log {
    using std::vector;
    using std::string;
    using std::ostream;
    using std::clog;
    using std::cerr;
    using std::cout;
    using lib::etc::conf;

    class HandleManager {
     public:
       typedef Record::LEVEL LEVEL;

     public:
       HandleManager() {}
       virtual ~HandleManager() { DestroyHandle(); }

     public:
       INT32 Initial(const string & config_file);
       INT32 Initial(const conf & config);

     public:
       INT32 RegisterHandle(const string & handle);

     public:
       static HandleManager * GetInstance() {
         static HandleManager * log_manager = NULL;
         if (!log_manager) {
           log_manager = new HandleManager();
         }
         return log_manager;
       }

       static HandleManager * GetFrameInstance() {
         static HandleManager * log_frame_manager = NULL;
         if (!log_frame_manager) {
           log_frame_manager = new HandleManager();
         }
         return log_frame_manager;
       }

     private:
       INT32 RegisterStreamHandle(UINT64 mask, Format * format, \
                                  LEVEL level, BOOL exact, \
                                  const string & os);
       INT32 RegisterFileHandle(UINT64 mask, Format * format, \
                                LEVEL level, BOOL exact, \
                                const string & file, INT32 size, \
                                BOOL flush, BOOL muti_process);
       INT32 RegisterNetHandle(UINT64 mask, Format * format, \
                               LEVEL level, BOOL exact, \
                               const string & net_addr, INT32 type);

     public:
       INT32 Logging(const Record & record);
       VOID DestroyHandle();

     private:
       vector<Handle *> list_;
    };
  }  // namespace log
}  // namespace lib

#endif  // COMMLIB_LOG_INC_HANDLEMANAGER_H_
