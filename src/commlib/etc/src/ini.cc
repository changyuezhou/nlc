// Copyright (c) 2013 zhou chang yue. All rights reserved.

#include <string>
#include "commlib/public/inc/type.h"
#include "commlib/magic/inc/str.h"
#include "commlib/etc/inc/etc.h"
#include "commlib/etc/inc/err.h"
#include "commlib/etc/inc/log.h"

namespace lib {
  namespace etc {
    using std::string;
    using std::getline;
    using lib::magic::String;

    INT32 ini::Create(const string & file) {
      INT32 result = ParseFile(file);

      return result;
    }

    INT32 ini::ParseFile(const string & file) {
      ifstream fin(file.c_str());

      if (!fin.is_open()) {
        LIB_ETC_LOG_ERROR("open file " << file << " failed");
        return Err::kERR_PARSER_FILE_OPEN;
      }

      INT32 result = LoadKeyValue(&fin);

      if (0 != result) {
        LIB_ETC_LOG_ERROR("load key value failed file:" << file);
      }

      fin.close();

      return result;
    }

    INT32 ini::LoadKeyValue(istream * fin) {
      string line;
      string section;

      while (getline(*fin, line)) {
        if (line.empty())
          continue;

        SIZE_T pos = line.find_first_of("#");
        if (string::npos != pos)
          line.erase(pos);

        line = String::Trim(line);

        if (line.empty())
          continue;

        if (string::npos == (pos = line.find_first_of("[="))) {
          LIB_ETC_LOG_ERROR("not found [=");
          return Err::kERR_LOAD_KEY_NOT_FOUND;
        }

        if ('=' == line[pos]) {
          string name = String::Trim(line.substr(0, pos));
          string value = String::Trim(line.substr(pos+1));
          name = section + "." + name;

          if (!key_value_.insert(make_pair(name, value)).second) {
            LIB_ETC_LOG_ERROR("key " << name << " is exists");
            return Err::kERR_LOAD_KEY_MUTI;
          }
        } else if ('[' == line[pos]) {
          SIZE_T end_pos = line.find_last_of("]");
          if (string::npos == end_pos) {
            LIB_ETC_LOG_ERROR("not found ]");
            return Err::kERR_LOAD_KEY_NOT_FOUND;
          }

          section = String::Trim(line.substr(pos+1, end_pos-pos-1));
        }
      }
      return 0;
    }
  }  // namespace etc
}  // namespace lib
