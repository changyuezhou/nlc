// Copyright (c) 2013 zhou chang yue. All rights reserved.

#include <string.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include "commlib/magic/inc/gen_key.h"

namespace lib {
  namespace magic {
    UINT32 GenKey::GetIP() {
      struct ifaddrs * if_addr = NULL;
      ::getifaddrs(&if_addr);

      while (NULL != if_addr) {
        if (AF_INET == if_addr->ifa_addr->sa_family) {
          return (((struct sockaddr_in *)if_addr->ifa_addr)->sin_addr).s_addr;
        }

        if_addr = if_addr->ifa_next;
      }

      return 0;
    }

    UINT32 GenKey::GetTimestamp() {
      return ::time(NULL);
    }

    UINT32 GenKey::GetPID() {
      return ::getpid();
    }

    INT32 GenKey::InitialPrefix() {
      ::memset(prefix_, 0, sizeof(prefix_));
      ::snprintf(prefix_, sizeof(prefix_), "%X%X%X", GetIP(), GetTimestamp(), GetPID());

      return 0;
    }

    const string GenKey::GetID() {
      if (counter_ > 2000000000) {
        InitialPrefix();
        counter_ = 0;
      }
      CHAR id[128] = {0};
      ::snprintf(id, sizeof(id), "%s%010d", prefix_, counter_++);

      return id;
    }

	
	
  }  // namespace magic
}  // namespace lib
