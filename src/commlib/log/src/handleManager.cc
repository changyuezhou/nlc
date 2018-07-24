// Copyright (c) 2013 zhou chang yue. All rights reserved.

#include "commlib/magic/inc/str.h"
#include "commlib/etc/inc/conf.h"
#include "commlib/log/inc/basicFormat.h"
#include "commlib/log/inc/streamHandle.h"
#include "commlib/log/inc/fileHandle.h"
#include "commlib/log/inc/netHandle.h"
#include "commlib/log/inc/handleManager.h"
#include "commlib/log/inc/log.h"

namespace lib {
  namespace log {
    using lib::magic::String;
    using lib::magic::StringSplit;

    INT32 HandleManager::Initial(const conf & config) {
      const string & module = config.GetString("log.module");
      const string & level = config.GetString("log.level");
      const string & level_exact = config.GetString("log.level.exact");
      const string & format = config.GetString("log.format");
      const string & format_mode = config.GetString("log.format.mode");

      INT32 handler_size = config.GetINT32("log.handler.size");
      for (INT32 i = 0; i < handler_size; ++i) {
        CHAR section[1024] = {0};
        ::snprintf(section, sizeof(section), "log.handler%d", i);

        CHAR key[1024] = {0};
        ::snprintf(key, sizeof(key), "%s.module", section);
        string handler_module = config.GetString(key);
        if (0 >= handler_module.length()) {
          handler_module = module;
        }
        ::snprintf(key, sizeof(key), "%s.level", section);
        string handler_level = config.GetString(key);
        if (0 >= handler_level.length()) {
          handler_level = level;
        }
        ::snprintf(key, sizeof(key), "%s.level.exact", section);
        string handler_level_exact = config.GetString(key);
        if (0 >= handler_level_exact.length()) {
          handler_level_exact = level_exact;
        }
        ::snprintf(key, sizeof(key), "%s.format", section);
        string handler_format = config.GetString(key);
        if (0 >= handler_format.length()) {
          handler_format = format;
        }
        ::snprintf(key, sizeof(key), "%s.format.mode", section);
        string handler_format_mode = config.GetString(key);
        if (0 >= handler_format_mode.length()) {
          handler_format_mode = format_mode;
        }

        ::snprintf(key, sizeof(key), "%s.name", section);
        const string & name = config.GetString(key);
        CHAR handler[2048] = {0};
        if ((0 == ::strncasecmp("cout", name.c_str(), 3)) ||
           (0 == ::strncasecmp("clog", name.c_str(), 3)) ||
           (0 == ::strncasecmp("cerr", name.c_str(), 3))) {
          ::snprintf(handler, sizeof(handler), "STD,%s,%s:%s,%s:%s,%s",
                     handler_module.c_str(),
                     handler_level.c_str(),
                     handler_level_exact.c_str(),
                     handler_format.c_str(),
                     handler_format_mode.c_str(),
                     name.c_str());
        } else if ((0 == ::strncasecmp("file", name.c_str(), 4))) {
          ::snprintf(key, sizeof(key), "%s.file.flush", section);
          string handler_flush = config.GetString(key);
          ::snprintf(key, sizeof(key), "%s.file.muti.input", section);
          string handler_muti_input = config.GetString(key);
          ::snprintf(key, sizeof(key), "%s.file.name", section);
          string handler_file_name = config.GetString(key);
          ::snprintf(key, sizeof(key), "%s.file.size", section);
          string handler_file_size = config.GetString(key);

          ::snprintf(handler, sizeof(handler), "%s,%s,%s:%s,%s:%s,%s,%s,%s,%s",
                     name.c_str(),
                     handler_module.c_str(),
                     handler_level.c_str(),
                     handler_level_exact.c_str(),
                     handler_format.c_str(),
                     handler_format_mode.c_str(),
                     handler_file_name.c_str(),
                     handler_file_size.c_str(),
                     handler_flush.c_str(),
                     handler_muti_input.c_str());
        } else if ((0 == ::strncasecmp("tcp", name.c_str(), 3)) ||
                  (0 == ::strncasecmp("udp", name.c_str(), 3)) ||
                  (0 == ::strncasecmp("unix_stream", name.c_str(), 11)) ||
                  (0 == ::strncasecmp("unix_dgram", name.c_str(), 10))) {
          ::snprintf(key, sizeof(key), "%s.addr", section);
          string handler_address = config.GetString(key);

          ::snprintf(handler, sizeof(handler), "%s,%s,%s:%s,%s:%s,%s",
                     name.c_str(),
                     handler_module.c_str(),
                     handler_level.c_str(),
                     handler_level_exact.c_str(),
                     handler_format.c_str(),
                     handler_format_mode.c_str(),
                     handler_address.c_str());
        } else {
          return Err::kERR_FILE_HANDLE_INVALID;
        }

        RAW_LOG_DEBUG("register handle:" << handler);
        INT32 result = RegisterHandle(handler);
        if (0 != result) {
          return Err::kERR_FILE_HANDLE_CREATE_FAILED;
        }
      }

      return 0;
    }

    INT32 HandleManager::Initial(const string & config_file) {
      conf config;
      if (0 != config.Create(config_file)) {
        return Err::kERR_FILE_CONFIG_CREATE_FAILED;
      }

      return Initial(config);
    }

    // STD,FFFFFFFFFFFFFFFF,LEVEL:Y,FORMAT:FULL
    INT32 HandleManager::RegisterHandle(const string & handle) {
      StringSplit log_split(handle, ",");
      INT32 token_size = log_split.TokenSize();
      const string & type = log_split.GetToken(0);
      UINT64 mask = String::StringToHex64(log_split.GetToken(1).c_str());
      const string & level_str = log_split.GetToken(2);
      StringSplit level_split(level_str, ":");
      const string & level = level_split.GetToken(0);
      BOOL exact = FALSE;
      if (2 <= level_split.TokenSize()) {
        if (0 == ::strncasecmp("Y", level_split.GetToken(1).c_str(), 1)) {
          exact = TRUE;
        }
      }
      LEVEL value = Record::INFO;
      if (0 == ::strncasecmp("SPECIAL", level.c_str(), 7)) {
        value = Record::SPECIAL;
      } else if (0 == ::strncasecmp("DEBUG", level.c_str(), 5)) {
        value = Record::DEBUG;
      } else if (0 == ::strncasecmp("INFO", level.c_str(), 4)) {
        value = Record::INFO;
      } else if (0 == ::strncasecmp("WARN", level.c_str(), 4)) {
        value = Record::WARN;
      } else if (0 == ::strncasecmp("ERROR", level.c_str(), 5)) {
        value = Record::ERROR;
      } else if (0 == ::strncasecmp("FATAL", level.c_str(), 5)) {
        value = Record::FATAL;
      }

      const string & str_format = log_split.GetToken(3);
      StringSplit format_split(str_format, ":");
      Format * format = NULL;
      if (0 == ::strncasecmp("BASIC", format_split.GetToken(0).c_str(), 5)) {
        format = new BasicFormat(format_split.GetToken(1));
      } else {
        return Err::kERR_FILE_HANDLE_INVALID_FORMAT_FAILED;
      }

      INT32 result = 0;
      if (0 == ::strncasecmp("STD", type.c_str(), 3)) {
        const string & os = log_split.GetToken(4);
        result = RegisterStreamHandle(mask, format, value, exact, os);
      } else if (0 == ::strncasecmp("FILE", type.c_str(), 4)) {
        const string & file = log_split.GetToken(4);
        INT32 size = ::atoi(log_split.GetToken(5).c_str());
        BOOL flush = FALSE;
        if (7 <= token_size) {
          const string & str_flush = log_split.GetToken(6);
          if (0 == ::strncasecmp("Y", str_flush.c_str(), 1)) {
            flush = TRUE;
          }
        }

        BOOL muti_process = FALSE;
        if (8 <= token_size) {
          const string & str_muti_process = log_split.GetToken(7);
          if (0 == ::strncasecmp("Y", str_muti_process.c_str(), 1)) {
            muti_process = TRUE;
          }
        }

        result = RegisterFileHandle(mask, format, value, exact, file, size, flush, muti_process);
      } else if (0 == ::strncasecmp("TCP", type.c_str(), 3)) {
        INT32 net_type = 0;
        const string & addr = log_split.GetToken(4);
        result = RegisterNetHandle(mask, format, value, exact, addr, net_type);
      } else if (0 == ::strncasecmp("UDP", type.c_str(), 3)) {
        INT32 net_type = 1;
        const string & addr = log_split.GetToken(4);
        result = RegisterNetHandle(mask, format, value, exact, addr, net_type);
      } else if (0 == ::strncasecmp("UNIX_STREAM", type.c_str(), 11)) {
        INT32 net_type = 2;
        const string & addr = log_split.GetToken(4);
        result = RegisterNetHandle(mask, format, value, exact, addr, net_type);
      } else if (0 == ::strncasecmp("UNIX_DGRAM", type.c_str(), 10)) {
        INT32 net_type = 3;
        const string & addr = log_split.GetToken(4);
        result = RegisterNetHandle(mask, format, value, exact, addr, net_type);
      } else {
        return Err::kERR_FILE_HANDLE_INVALID_TYPE_FAILED;
      }

      if (0 != result) {
        return Err::kERR_FILE_HANDLE_INVALID_TYPE_FAILED;
      }

      return 0;
    }

    INT32 HandleManager::RegisterStreamHandle(UINT64 mask, Format * format, \
                                             LEVEL level, BOOL exact, \
                                             const string & os) {
      ostream * os_out = &clog;
      if (0 == ::strncasecmp("cout", os.c_str(), 4)) {
        os_out = &cout;
      } else if (0 == ::strncasecmp("cerr", os.c_str(), 4)) {
        os_out = &cerr;
      }

      Handle * handle = new StreamHandle(format, level, exact, mask, os_out);

      list_.push_back(handle);

      return 0;
    }

    INT32 HandleManager::RegisterFileHandle(UINT64 mask, Format * format, \
                                           LEVEL level, BOOL exact, \
                                           const string & file, INT32 size, \
                                           BOOL flush, BOOL muti_process) {
      INT32 granularity = 0;
      if (string::npos != file.find("{HH}")) {
        granularity = 2;
      } else if (string::npos != file.find("{DD}")) {
        granularity = 1;
      } else if (string::npos != file.find("{MM}")) {
        granularity = 3;
      }

      string lock_file = "";
      if (muti_process) {
        SIZE_T pos = file.find_last_of("/");
        if (string::npos == pos) {
          lock_file = "." + file + ".lock";
        } else {
          lock_file = file;
          lock_file = lock_file.replace(pos, 1, "/.") + ".lock";
        }
      }

      Handle * handle = new FileHandle(format, level, exact, mask, file, size, granularity, flush, muti_process, lock_file);

      list_.push_back(handle);

      return 0;
    }

    INT32 HandleManager::RegisterNetHandle(UINT64 mask, Format * format, \
                                          LEVEL level, BOOL exact, \
                                          const string & net_addr, INT32 type) {
      Handle * handle = new NetHandle(format, level, exact, mask, net_addr, type);

      list_.push_back(handle);

      return 0;
    }

    INT32 HandleManager::Logging(const Record & record) {
      INT32 size = list_.size();
      for (INT32 i = 0; i < size; i++) {
        list_[i]->Logging(record);
      }

      return 0;
    }

    VOID HandleManager::DestroyHandle() {
      INT32 size = list_.size();
      for (INT32 i = 0; i < size; i++) {
        list_[i]->DestroyHandle();
        delete list_[i];
      }

      list_.clear();
    }
  }  // namespace log
}  // namespace lib
