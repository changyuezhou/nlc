// Copyright (c) 2013 zhou chang yue. All rights reserved.

#include <unistd.h>
#include "commlib/net/inc/net_addr.h"

namespace lib {
  namespace net {
    VOID NetAddr::Serialize(const string & addr) {
      StringSplit split(addr, ":");
      INT32 token_size = split.TokenSize();

      if (2 == token_size) {
        const string & ip = split.GetToken(0);
        INT32 port = 0;
        if (2 <= token_size) {
          port = ::atoi(split.GetToken(1).c_str());
        }

        addr_.inet_.ip_ = ::inet_addr(ip.c_str());
        addr_.inet_.port_ = port;

        type_ = INET;
      } else {
        ::snprintf(addr_.path_, sizeof(addr_.path_), "%s", addr.c_str());
        type_ = UNIX;
      }
    }

    const string NetAddr::Deserialize() const {
      if (INET == type_) {
        INT32 port = addr_.inet_.port_;
        in_addr tmp;
        ::memcpy(&tmp, &addr_.inet_.ip_, 4);
        const CHAR * ip = ::inet_ntoa(tmp);
        CHAR addr[1024] = {0};
        ::snprintf(addr, sizeof(addr), "%s:%d", ip, port);

        return addr;
      }

      return addr_.path_;
    }

    const string NetAddr::GetIp() const {
      in_addr tmp;
      ::memcpy(&tmp, &addr_.inet_.ip_, 4);
      return ::inet_ntoa(tmp);
    }

    INT32 NetAddr::GetPort() const {
      return addr_.inet_.port_;
    }

    const string NetAddr::GetPath() const {
      return addr_.path_;
    }

    VOID NetAddr::SetIp(struct in_addr * ip) {
      ::memcpy(&(addr_.inet_.ip_), ip, 4);
    }

    VOID NetAddr::SetIp(const string & ip) {
      addr_.inet_.ip_ = ::inet_addr(ip.c_str());
    }

    VOID NetAddr::SetPort(INT32 port) {
      addr_.inet_.port_ = port;
    }

    VOID NetAddr::SetPath(const string & path) {
      ::snprintf(addr_.path_, sizeof(addr_.path_), "%s", path.c_str());
    }
  }  // namespace net
}  // namespace lib
