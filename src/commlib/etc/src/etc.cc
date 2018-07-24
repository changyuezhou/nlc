// Copyright (c) 2013 zhou chang yue. All rights reserved.

#include <string.h>
#include <cstdlib>
#include "commlib/etc/inc/etc.h"
#include "commlib/etc/inc/log.h"

namespace lib {
  namespace etc {
    const string etc::GetString(const string & key) const {
      if (key_value_.find(key) == key_value_.end()) {
        return "";
      }

      return key_value_.at(key);
    }

    INT32 etc::GetINT32(const string & key) const {
      if (key_value_.find(key) == key_value_.end()) {
        return 0;
      }

      return ::atoi(key_value_.at(key).c_str());
    }

    INT32 etc::GetUINT32(const string & key) const {
      if (key_value_.find(key) == key_value_.end()) {
        return 0;
      }

      return ::strtoul(key_value_.at(key).c_str(), NULL, 10);
    }

    INT32 etc::GetINT64(const string & key) const {
      if (key_value_.find(key) == key_value_.end()) {
        return 0;
      }

      return ::strtoll(key_value_.at(key).c_str(), NULL, 10);
    }

    INT32 etc::GetUINT64(const string & key) const {
      if (key_value_.find(key) == key_value_.end()) {
        return 0;
      }

      return ::strtoull(key_value_.at(key).c_str(), NULL, 10);
    }

    INT32 etc::GetBOOL(const string & key) const {
      if (key_value_.find(key) == key_value_.end()) {
        return FALSE;
      }

      if ((0 < key_value_.at(key).length()) && (0 == ::strncasecmp(key_value_.at(key).c_str(), "Y", ::strlen("Y")))) {
        return TRUE;
      }

      return FALSE;
    }

    VOID etc::Dump() {
      INT32 size = key_value_.size();
      if (0 >= size) {
        return;
      }

      LIB_ETC_LOG_INFO("******************************************************");
      KeyValue::const_iterator iter = key_value_.begin();
      while (iter != key_value_.end()) {
        LIB_ETC_LOG_INFO(iter->first << ":" << iter->second);
        iter++;
      }
      LIB_ETC_LOG_INFO("******************************************************");
    }
  }  // namespace etc
}  // namespace lib
