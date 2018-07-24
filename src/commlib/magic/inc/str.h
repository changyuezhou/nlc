// Copyright (c) 2013 zhou chang yue. All rights reserved.

#ifndef COMMLIB_MAGIC_INC_STR_H_
#define COMMLIB_MAGIC_INC_STR_H_

#include <string.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <functional>
#include <utility>
#include <cctype>
#include <regex.h>
#include "commlib/public/inc/type.h"
#include "commlib/magic/inc/log.h"

namespace lib {
  namespace magic {
    using std::string;
    using std::hex;
    using std::dec;
    using std::vector;
    using std::find_if;
    using std::not1;
    using std::ptr_fun;
    using std::isspace;
    using std::ispunct;
    using std::ostringstream;
    using std::pair;
    using std::make_pair;

    struct IsQuotation {
      bool operator() (const int& x) const {return x==34;}
      typedef int argument_type;
    };

    class StringSplit {
     public:
       StringSplit(const string &str, const string &split) {
         token_.clear();
         CHAR s1[102400] = {0};  // maybe dynamic malloc
         ::snprintf(s1, sizeof(s1) - 1, "%s", str.c_str());
         CHAR * s2 = NULL;
         CHAR * token = NULL;
         token = ::strtok_r(s1, split.c_str(), &s2);
         while (NULL != token) {
           token_.push_back(token);
           token = strtok_r(NULL, split.c_str(), &s2);
         }
       }

       ~StringSplit() {
         token_.clear();
       }

     public:
       const string & operator [] (INT32 index) { return token_[index]; }

     public:
       INT32 size() { return token_.size(); }
      
       INT32 TokenSize() {
         return token_.size();
       }

       const string & GetToken(INT32 index) {
         return token_[index];
       }

       const vector<string> & GetArray() { return token_; }

     private:
       vector<string> token_;
    };

    class String {
     public:
       static inline string Trim_Left(string str) {
         str.erase(str.begin(), find_if(str.begin(), str.end(), not1(ptr_fun<INT32, INT32>(isspace))));
         return str;
       }

       static inline string Trim_Right(string str) {
         str.erase(find_if(str.rbegin(), str.rend(), not1(ptr_fun<INT32, INT32>(isspace))).base(), str.end());

         return str;
       }

       static inline string Trim_Mid(string str) {
         str.erase (std::remove(str.begin(), str.end(), ' '), str.end());

         return str;
       }

       static inline string Trim_CHAR(string str, CHAR c) {
         str.erase (std::remove(str.begin(), str.end(), c), str.end());
         return str;
       }

       static inline string Trim(string str) {
         return Trim_Mid(Trim_Left(Trim_Right(str)));
       }

       static inline BOOL IsPhoneNumber(const string & str) {
         for (std::string::const_iterator it=str.begin(); it!=str.end(); ++it) {
           if (!::isdigit(*it) && ('+' != *it) && ('-' != *it)) {
             return FALSE;
           }
         }

         return TRUE;
       }

       static inline BOOL IsCardID(const string & str) {
         for (std::string::const_iterator it=str.begin(); it!=str.end(); ++it) {
           if (!::isdigit(*it) && ('A' > *it) && ('Z' < *it)) {
             return FALSE;
           }
         }

         if (18 != str.length()) {
           return FALSE;
         }

         return TRUE;
       }

       static inline BOOL IsNumber(const string & str) {
         for (std::string::const_iterator it=str.begin(); it!=str.end(); ++it) {
           if (!::isdigit(*it)) {
             return FALSE;
           }
         }

         return TRUE;
       }

       static inline BOOL IsEmail(const string & email) {
         const string pattern = "([0-9A-Za-z\\-_\\.]+)@([0-9a-z]+\\.[a-z]{2,3}(\\.[a-z]{2})?)";
         INT32   status = -1;
         regex_t re;
         CHAR err_msg[1024] = {0};

         if (0 != (status = ::regcomp(&re, pattern.c_str(), REG_EXTENDED))) {
           LIB_MAGIC_LOG_ERROR("reg compare failed pattern:" << pattern);
           ::regerror(status, &re, err_msg, sizeof(err_msg));
           return FALSE;      /* Report error. */
         }
         status = ::regexec(&re, email.c_str(), (size_t) 0, NULL, 0);
         ::regfree(&re);
         if (0 != status) {
           ::regerror(status, &re, err_msg, sizeof(err_msg));
           LIB_MAGIC_LOG_ERROR("reg execute failed pattern:" << pattern << " email:" << email << " msg:" << err_msg);
           return FALSE;      /* Report error. */
         }

         return TRUE;
       }

       static inline BOOL IsURL(const string & url) {
         const string pattern = "https?://.+";
         INT32   status = -1;
         regex_t re;
         CHAR err_msg[1024] = {0};

         if (0 != (status = ::regcomp(&re, pattern.c_str(), REG_EXTENDED))) {
           LIB_MAGIC_LOG_ERROR("reg compare failed pattern:" << pattern);
           ::regerror(status, &re, err_msg, sizeof(err_msg));
           return FALSE;      /* Report error. */
         }
         status = ::regexec(&re, url.c_str(), (size_t) 0, NULL, 0);
         ::regfree(&re);
         if (0 != status) {
           ::regerror(status, &re, err_msg, sizeof(err_msg));
           LIB_MAGIC_LOG_ERROR("reg execute failed pattern:" << pattern << " url:" << url << " msg:" << err_msg);
           return FALSE;      /* Report error. */
         }

         return TRUE;
       }

       static inline string ToString(INT32 n) {
         ostringstream stm;
         stm << n;
         return stm.str();
       }

       static inline string ToString(UINT32 n) {
         ostringstream stm;
         stm << n;
         return stm.str();
       }

       static inline string ToString(UINT64 n) {
         ostringstream stm;
         stm << n;
         return stm.str();
       }

       static inline string ToString(INT64 n) {
         ostringstream stm;
         stm << n;
         return stm.str();
       }

       static inline UINT64 StringToHex64(const string & module) {
         UINT64 low = 0;
         UINT64 high = 0;
         if (8 < module.length()) {
           low = ::strtoul(module.substr(8).c_str(), NULL, 16);
           high = ::strtoul(module.substr(0, 7).c_str(), NULL, 16);
         } else {
           low = ::strtoul(module.c_str(), NULL, 16);
         }
         UINT64 value = (high << 32) + low;
         return value;
       }

       static inline const string StringPrintToHex(const string & src) {
         std::ostringstream os;
         os << hex;
         for (string::const_iterator it = src.begin(); it != src.end(); ++it) {
           os << (static_cast<SHORT>(*it) & 0xff);
         }
         os << dec;
         return os.str();
       }

       static inline const string StringPrintToHex(const CHAR * src, INT32 size) {
         const UCHAR * p = reinterpret_cast<UCHAR *>(const_cast<CHAR *>(src));
         CHAR hex[1024*64] = {0};
         INT32 len = 0;
         for (INT32 i = 0; i < size; i++) {
           len += ::snprintf(hex + len, sizeof(hex)-len, "%02X", p[i]);
         }
         return hex;
       }

       static inline pair<string, INT32> StringToIpPort(const string & addr) {
         StringSplit split(addr, ":");
         INT32 token_size = split.TokenSize();

         INT32 port = 0;
         if (2 <= token_size) {
           port = ::atoi(split.GetToken(1).c_str());
         }

         const string & ip = split.GetToken(0);

         return make_pair(ip, port);
       }
    };
  }  // namespace magic
}  // namespace lib

#endif  // COMMLIB_MAGIC_INC_STR_H_
