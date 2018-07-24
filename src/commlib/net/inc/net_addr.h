// Copyright (c) 2013 zhou chang yue. All rights reserved.

#ifndef COMMLIB_NET_INC_NET_ADDR_H_
#define COMMLIB_NET_INC_NET_ADDR_H_

#include <string.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <string>
#include "commlib/net/inc/log.h"
#include "commlib/magic/inc/str.h"


namespace lib {
  namespace net {
    using std::string;
    using lib::magic::String;
    using lib::magic::StringSplit;

    class NetAddr {
     public:
       enum ADDR_TYPE {
         INET = 0,
         UNIX = 1
       };

     public:
       static const INT32 MAX_NET_PATH = 256;

     public:
       struct INet {
         INT32 ip_;
         INT32 port_;
       };

       union Addr {
         INet inet_;
         CHAR path_[MAX_NET_PATH];
       };

     public:
       explicit NetAddr(INT32 type = INET):type_(type) {}
       virtual ~NetAddr() {}

     public:
       NetAddr(const NetAddr & r):type_(r.type_) {
         ::memcpy(&this->addr_, &r.addr_, sizeof(this->addr_));
       }

       const NetAddr & operator=(const NetAddr & r) {
         this->type_ = r.type_;
         ::memcpy(&this->addr_, &r.addr_, sizeof(this->addr_));
         return *this;
       }

     public:
       VOID Serialize(const string & addr);
       const string Deserialize() const;

     public:
       const string GetIp() const;
       INT32 GetPort() const;
       const string GetPath() const;

     public:
       VOID SetIp(struct in_addr * ip);
       VOID SetIp(const string & ip);
       VOID SetPort(INT32 port);
       VOID SetPath(const string & path);

     private:
       INT32 type_;
       Addr addr_;
    };
  }  // namespace net
}  // namespace lib

#endif  // COMMLIB_NET_INC_NET_ADDR_H_
