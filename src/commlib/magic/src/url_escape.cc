// Copyright (c) 2013 zhou chang yue. All rights reserved.

#include <stdio.h>
#include <sys/time.h>
#include "commlib/magic/inc/url_escape.h"

namespace lib {
  namespace magic {
    char URL_ESCAPE::from_hex(char ch) {
      return ::isdigit(ch) ? ch - '0' : tolower(ch) - 'a' + 10;
    }

    char URL_ESCAPE::to_hex(char code) {
      static char hex[] = "0123456789abcdef";
      return hex[code & 15];
    }

    char * URL_ESCAPE::url_encode(const char *str, char * desc, int size) {
      const char *pstr = str;
      char *buf = desc, *pbuf = buf;
      while (*pstr) {
        if (isalnum(*pstr) || *pstr == '-' || *pstr == '_' || *pstr == '.' || *pstr == '~')
          *pbuf++ = *pstr;
        else if (*pstr == ' ')
          *pbuf++ = '+';
        else
          *pbuf++ = '%', *pbuf++ = to_hex(*pstr >> 4), *pbuf++ = to_hex(*pstr & 15);
        pstr++;
      }
      *pbuf = '\0';
      return buf;
    }

    char * URL_ESCAPE::url_decode(const char *str, char * desc, int size) {
      const char *pstr = str;
      char *buf = desc, *pbuf = buf;
      while (*pstr) {
        if (*pstr == '%') {
          if (pstr[1] && pstr[2]) {
            *pbuf++ = from_hex(pstr[1]) << 4 | from_hex(pstr[2]);
            pstr += 2;
          }
        } else if (*pstr == '+') {
          *pbuf++ = ' ';
        } else {
          *pbuf++ = *pstr;
        }
        pstr++;
      }
      *pbuf = '\0';
      return buf;
    }
  }  // namespace magic
}  // namespace lib