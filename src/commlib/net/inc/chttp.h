// Copyright (c) 2013 zhou chang yue. All rights reserved.

#ifndef COMMLIB_NET_INC_CHTTP_H_
#define COMMLIB_NET_INC_CHTTP_H_

#include <stdio.h>
#include <curl/curl.h>
#include <string>
#include <map>
#include "commlib/public/inc/type.h"
#include "commlib/net/inc/err.h"
#include "commlib/net/inc/log.h"

namespace lib {
  namespace net {
    using std::string;
    using std::map;

    class CHttp {
     public:
       enum METHOD {
         GET = 1,
         POST = 2,
         PUT = 3,
         DELETE = 4,
         HEAD = 5,
         OPTIONS = 6,
         TRACE = 7,
         CONNECT = 8
       };

       enum PROTOCOL {
         HTTP = 1,
         HTTPS = 2
       };

       struct call_back_param {
         CHAR * data_;
         INT32 * size_;
         INT32 notify_fd_;
       };

       typedef map<string, string> HEADERS;

     public:
       CHttp(): curl_(NULL) {}
       virtual ~CHttp() {}

     public:
       static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp);

     public:
       INT32 Request(PROTOCOL protocol, METHOD method, const HEADERS & headers, const string & host, const string & path,
                     const CHAR * data, INT32 size, CHAR * resp, INT32 * resp_size, INT32 timeout = 2000);

     protected:
       INT32 WaitForStop(INT32 fd, INT32 timeout);

     private:
       CURL * curl_;
       INT32 pipe_[2];
    };
  }  // namespace net
}  // namespace lib
#endif  // COMMLIB_NET_INC_CHTTP_H_