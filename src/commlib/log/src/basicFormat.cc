// Copyright (c) 2013 zhou chang yue. All rights reserved.

#include "commlib/magic/inc/timeFormat.h"
#include "commlib/log/inc/basicFormat.h"

namespace lib {
  namespace log {
    const CHAR Format::ModTable[5][32]={
      "NONE",
      "MIN",
      "BRIEF",
      "LOCATE",
      "FULL"
    };

    INT32 BasicFormat::CreateFormat(const string & mode) {
      mode_ = GetMode(mode);

      return 0;
    }

    VOID BasicFormat::Formatting(ostream * os_pointer, const Record & record) {
      ostream & os = *os_pointer;

      if (mode_ >= MOD_MIN) {
        os << "[" << setw(5) << record.GetLevelStr() << "]";
      }

      if (mode_ >= MOD_BRIEF) {
        os << "[" << lib::magic::TimeFormat::GetStringISO(record.GetTimestamp()) << "]";
      }

      if (mode_ >= MOD_LOCATE) {
        os << "[" << "file: " << record.GetFileName() << "]";
        os << "[" << "line: " << dec << record.GetLine() << "]";
      }

      if (mode_ >= MOD_FULL) {
        os << "[" << "process: " << hex << record.GetProcessId() << dec << "]";
        os << "[" << "thread: " << hex << record.GetThreadId() << dec<< "]";
      }

      if (mode_ == MOD_NONE) {
        os << record.GetMessage() << endl;
      } else {
        os << "[" << record.GetMessage() << "]" << endl;
      }
    }
  }  // namespace log
}  // namespace lib
