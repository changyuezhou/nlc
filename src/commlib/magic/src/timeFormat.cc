// Copyright (c) 2013 zhou chang yue. All rights reserved.

#include <stdio.h>
#include <sys/time.h>
#include "commlib/magic/inc/timeFormat.h"

namespace lib {
  namespace magic {
    const string TimeFormat::GetStringISO(const time_t timestamp) {
      struct tm tm_time;
      localtime_r(&timestamp, &tm_time);
      CHAR timestamp_string[512] = {0};
      ::snprintf(&timestamp_string[0], sizeof(timestamp_string) - 1, \
                 "%04d-%02d-%02d %02d:%02d:%02d",                   \
                 tm_time.tm_year + 1900,                            \
                 tm_time.tm_mon + 1,                                \
                 tm_time.tm_mday,                                   \
                 tm_time.tm_hour,                                   \
                 tm_time.tm_min,                                    \
                 tm_time.tm_sec);
      return timestamp_string;
    }

    const string TimeFormat::GetStringDate(const time_t timestamp) {
      struct tm tm_time;
      localtime_r(&timestamp, &tm_time);
      CHAR timestamp_string[512] = {0};
      ::snprintf(&timestamp_string[0], sizeof(timestamp_string) - 1, \
                 "%04d-%02d-%02d",                                  \
                 tm_time.tm_year + 1900,                            \
                 tm_time.tm_mon + 1,                                \
                 tm_time.tm_mday);
      return timestamp_string;
    }

    const string TimeFormat::GetStringISO() {
      time_t cur_time;
      time(&cur_time);
      struct tm tm_time;
      localtime_r(&cur_time, &tm_time);
      CHAR timestamp_string[512] = {0};
      ::snprintf(&timestamp_string[0], sizeof(timestamp_string) - 1, \
                 "%02d:%02d:%02d %04d-%02d-%02d",                   \
                 tm_time.tm_hour,                                   \
                 tm_time.tm_min,                                    \
                 tm_time.tm_sec,                                    \
                 tm_time.tm_year + 1900,                            \
                 tm_time.tm_mon + 1,                                \
                 tm_time.tm_mday);

      return timestamp_string;
    }

    UINT64 TimeFormat::GetCurTimestampLong() {
      struct timeval tv;
      ::gettimeofday(&tv,NULL);

      return tv.tv_sec * 1000 + tv.tv_usec / 1000;
    }

    UINT32 TimeFormat::GetCurTimestampINT32() {
      struct timeval tv;
      ::gettimeofday(&tv,NULL);

      return tv.tv_sec;
    }
  }  // namespace magic
}  // namespace lib
