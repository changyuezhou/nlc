// Copyright (c) 2013 zhou chang yue. All rights reserved.

#include <sys/un.h>
#include <arpa/inet.h>
#include <string>
#include <sstream>
#include "commlib/magic/inc/dir.h"
#include "commlib/log/inc/netHandle.h"

namespace lib {
  namespace log {
    using lib::magic::StringSplit;
    using std::ostringstream;

    VOID NetHandle::Open() {
      sockaddr_in in_addr;
      sockaddr_un un_addr;
      string ip = "";
      INT32 port = 0;
      string path = "";

      switch (type_) {
      case TCP:
        fd_ = ::socket(AF_INET, SOCK_STREAM, 0);
        break;
      case UDP:
        fd_ = ::socket(AF_INET, SOCK_DGRAM, 0);
        break;
      case UNIX_STREAM:
        fd_ = ::socket(AF_UNIX, SOCK_STREAM, 0);
        break;
      case UNIX_DGRAM:
        fd_ = ::socket(AF_UNIX, SOCK_DGRAM, 0);
        break;
      default:
        break;
      }

      INT32 result = 0;
      if (TCP == type_ || UDP == type_) {
        ::memset(&in_addr, 0x00, sizeof(in_addr));

        StringSplit split(net_addr_, ":");
        INT32 token_size = split.TokenSize();

        ip = split.GetToken(0);
        if (2 <= token_size) {
          port = ::atoi(split.GetToken(1).c_str());
        }

        in_addr.sin_family = AF_INET;
        in_addr.sin_port = htons(port);
        in_addr.sin_addr.s_addr = ::inet_addr(ip.c_str());

        result = ::connect(fd_, reinterpret_cast<sockaddr *>(&in_addr), sizeof(in_addr));
        if (0 != result) {
          fd_ = -1;
        }
      } else {
        ::memset(&un_addr, 0x00, sizeof(un_addr));
        un_addr.sun_family = AF_UNIX;
        ::snprintf(un_addr.sun_path, sizeof(un_addr.sun_path)-1, "%s", net_addr_.c_str());

        result = ::connect(fd_, reinterpret_cast<sockaddr *>(&un_addr), sizeof(un_addr));
        if (0 != result) {
          fd_ = -1;
        }
      }
    }

    INT32 NetHandle::Logging(const Record & record) {
      if (exact_ && level_ != record.GetLevel()) {
        return 0;
      }
      ScopeLock<Mutex> scope_lock(&mutex_);
      if ((mask_ && record.GetMask()) && (level_ <= record.GetLevel())) {
        if (NULL != format_) {
          ostringstream os;
          format_->Formatting(&os, record);
          string message(os.str());
          INT32 result = WriteData(message.c_str(), message.length());
          if (0 != result) {
            fd_ = -1;
          }
          return result;
        }  // end of format
      }  // end of mask and level

      return 0;
    }

    VOID NetHandle::DestroyHandle() {
      Handle::DestroyHandle();
    }

    INT32 NetHandle::WriteData(const CHAR * out, INT32 size) {
      if (0 > fd_) {
        Open();
      }
      INT32 written = 0;
      while (written < size) {
        INT32 once_written = 0;
        once_written = ::send(fd_, out + written, size - written, 0);
        if (0 > once_written) {
          return -1;
        }
        written += once_written;
      }

      return 0;
    }
  }  // namespace log
}  // namespace lib
