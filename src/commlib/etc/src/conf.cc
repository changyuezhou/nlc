// Copyright (c) 2013 zhou chang yue. All rights reserved.

#include <string.h>
#include <cstdlib>
#include "commlib/etc/inc/conf.h"
#include "commlib/public/inc/type.h"
#include "commlib/magic/inc/str.h"
#include "commlib/etc/inc/err.h"
#include "commlib/etc/inc/log.h"

namespace lib {
  namespace etc {
    using std::string;
    using std::map;
    using std::getline;
    using lib::magic::String;

    INT32 conf::LoadKeyValue(istream * fin) {
      string line;

      while (getline(*fin, line)) {
        if (line.empty())
          continue;

        SIZE_T pos = line.find_first_of("#");
        if (string::npos != pos)
          line.erase(pos);

        line = String::Trim(line);

        if (line.empty())
          continue;

        if (string::npos == (pos = line.find_first_of("{"))) {
          LIB_ETC_LOG_ERROR("not found {");
          return Err::kERR_LOAD_KEY_NOT_FOUND;
        }

        string section = String::Trim(line.substr(0, pos));
        INT32 result = LoadItem(section, fin);
        if (0 != result) {
          LIB_ETC_LOG_ERROR("config file item parse failed");
          return Err::kERR_PARSER_FILE;
        }
      }

      return 0;
    }

    INT32 conf::LoadItem(const string & section, istream * fin) {
      string line = "";

      while (getline(*fin, line)) {
        if (line.empty())
          continue;

        SIZE_T pos = line.find_first_of("#");
        if (string::npos != pos)
          line.erase(pos);

        line = String::Trim(line);

        if (line.empty())
          continue;

        if (string::npos == (pos = line.find_first_of("{=}"))) {
          LIB_ETC_LOG_ERROR("not found =} section:" << section);
          return Err::kERR_LOAD_KEY_NOT_FOUND;
        }

        if ('=' == line[pos]) {
          string name = String::Trim(line.substr(0, pos));
          string value = String::Trim(line.substr(pos+1));

          if (string::npos == (pos = value.find_first_of("["))) {
            name = section + "." + name;
            value = String::Trim_CHAR(value, '"');
            if (!key_value_.insert(make_pair(name, value)).second) {
              LIB_ETC_LOG_ERROR("key " << name << " is exists value:" << value << " section:" << section);
              return Err::kERR_LOAD_KEY_MUTI;
            }
          } else {
            INT32 result = LoadArray(section + "." + name, fin);
            if (0 != result) {
              LIB_ETC_LOG_ERROR("load array item failed name:" << name << " value:" << value << " section:" << section);
              return Err::kERR_LOAD_ARRAY_FAILED;
            }
          }
        } else if ('}' == line[pos]) {
          break;
        } else if ('{' == line[pos]) {
          string nest_section = section + "." + String::Trim(line.substr(0, pos));
          INT32 result = LoadItem(nest_section, fin);
          if (0 != result) {
            LIB_ETC_LOG_ERROR("config file item parse failed section:" << section << " nest section:" << nest_section);
            return Err::kERR_PARSER_FILE;
          }
        }
      }

      return 0;
    }

    INT32 conf::LoadArray(const string & section, istream * fin) {
      string line = "";

      INT32 i = 0;

      while (getline(*fin, line)) {
        if (line.empty())
          continue;

        SIZE_T pos = line.find_first_of("#");
        if (string::npos != pos)
          line.erase(pos);

        line = String::Trim(line);

        if (line.empty())
          continue;

        if (string::npos != (pos = line.find_first_of(","))) {
          continue;
        }

        if (string::npos == (pos = line.find_first_of("{[]"))) {
          LIB_ETC_LOG_ERROR("not found {] section:" << section);
          return -1;
        }

        if ('{' == line[pos]) {
          INT32 result = LoadArrayItem(i, section, fin);
          if (0 != result) {
            return -1;
          }
          ++i;
        } else if (']' == line[pos]) {
          CHAR array_size[256] = {0};
          ::snprintf(array_size, sizeof(array_size), "%d", i);
          string key = section + ".size";
          if (!key_value_.insert(make_pair(key, array_size)).second) {
            LIB_ETC_LOG_ERROR("key " << key << " is exists section:" << section);
            return -1;
          }
          break;
        }
      }
      return 0;
    }

    INT32 conf::LoadArrayItem(INT32 index, const string & section, istream * fin) {
      string line = "";
      while (getline(*fin, line)) {
        if (line.empty())
          continue;

        SIZE_T pos = line.find_first_of("#");
        if (string::npos != pos)
          line.erase(pos);

        line = String::Trim(line);

        if (line.empty())
          continue;

        if (string::npos == (pos = line.find_first_of("{=}"))) {
          LIB_ETC_LOG_ERROR("not found =} index:" << index << " section:" << section << " line:" << line);
          return Err::kERR_LOAD_KEY_NOT_FOUND;
        }

        if ('=' == line[pos]) {
          string name = String::Trim(line.substr(0, pos));
          string value = String::Trim(line.substr(pos+1));

          CHAR section_index[256] = {0};
          ::snprintf(section_index, sizeof(section_index), "%s%d.%s", section.c_str(), index, name.c_str());

          if (string::npos == (pos = value.find_first_of("["))) {
            value = String::Trim_CHAR(value, '"');
            if (!key_value_.insert(make_pair(section_index, value)).second) {
              LIB_ETC_LOG_ERROR("key " << section_index << " is exists index:" << index << " section:" << section);
              return Err::kERR_LOAD_KEY_MUTI;
            }
          } else {
            INT32 result = LoadArray(section_index, fin);
            if (0 != result) {
              LIB_ETC_LOG_ERROR("load array item failed name:" << name << " value:" << value << " section:"
                                                               << section_index << " line:" << line);
              return Err::kERR_LOAD_ARRAY_FAILED;
            }
          }
        } else if ('{' == line[pos]) {
          string nest_section = section + String::ToString(index) + "." + String::Trim(line.substr(0, pos));
          INT32 result = LoadItem(nest_section, fin);
          if (0 != result) {
            LIB_ETC_LOG_ERROR("config file item parse failed index:" << index << " section:" << section
                                                                     << " nest section:" << nest_section);
            return Err::kERR_PARSER_FILE;
          }
        } else if ('}' == line[pos]) {
          break;
        }
      }

      return 0;
    }
  }  // namespace etc
}  // namespace lib
