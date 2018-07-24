// Copyright (c) 2013 zhou chang yue. All rights reserved.

#ifndef COMMLIB_DIST_INC_ERR_H_
#define COMMLIB_DIST_INC_ERR_H_

#include <string>
#include <cstdio>
#include <zookeeper/zookeeper.h>
#include "commlib/public/inc/type.h"

namespace lib {
  namespace dist {
    using std::string;

    class Err {
     public:
       // ini errors
       static const INT32 kERR_ZOO_SYSTEM_ERROR = -100100;
       static const INT32 kERR_ZOO_RUNTIME_INCONSISTENCY = -100101;
       static const INT32 kERR_ZOO_DATA_INCONSISTENCY = -100102;
       static const INT32 kERR_ZOO_CONNECTION_LOST = -100103;
       static const INT32 kERR_ZOO_MARSHALLING_ERROR = -100104;
       static const INT32 kERR_ZOO_UNIMPLEMENTED = -100105;
       static const INT32 kERR_ZOO_OPERATION_TIMEOUT = -100106;
       static const INT32 kERR_ZOO_BAD_ARGUMENTS = -100107;
       static const INT32 kERR_ZOO_INVALID_STATE = -100108;
       static const INT32 kERR_ZOO_API_ERROR = -100109;
       static const INT32 kERR_ZOO_NO_NODE = -100110;
       static const INT32 kERR_ZOO_NO_AUTH = -100111;
       static const INT32 kERR_ZOO_BAD_VERSION = -100112;
       static const INT32 kERR_ZOO_NO_CHILDREN_FOR_EPHEMERAL = -100113;
       static const INT32 kERR_ZOO_NODE_EXISTS = -100114;
       static const INT32 kERR_ZOO_NOT_EMPTY = -100115;
       static const INT32 kERR_ZOO_SESSION_EXPIRED = -100116;
       static const INT32 kERR_ZOO_INVALID_CALLBACK = -100117;
       static const INT32 kERR_ZOO_INVALID_ACL = -100118;
       static const INT32 kERR_ZOO_AUTH_FAILED = -100119;
       static const INT32 kERR_ZOO_CLOSING = -100120;
       static const INT32 kERR_ZOO_NOTHING = -100121;
       static const INT32 kERR_ZOO_SESSION_MOVED = -100122;
       static const INT32 kERR_ZOO_INITIAL_FAILED = -100123;
       static const INT32 kERR_ZOO_HANDLE_INVALID = -100124;
       static const INT32 kERR_ZOO_UNKNOWN = -100999;

       static INT32 ZooErrStatus(INT32 code) {
         if (ZSYSTEMERROR == code) {
           return kERR_ZOO_SYSTEM_ERROR;
         } else if (ZRUNTIMEINCONSISTENCY == code) {
           return kERR_ZOO_RUNTIME_INCONSISTENCY;
         } else if (ZDATAINCONSISTENCY == code) {
           return kERR_ZOO_DATA_INCONSISTENCY;
         } else if (ZCONNECTIONLOSS == code) {
           return kERR_ZOO_CONNECTION_LOST;
         } else if (ZMARSHALLINGERROR == code) {
           return kERR_ZOO_MARSHALLING_ERROR;
         } else if (ZUNIMPLEMENTED == code) {
           return kERR_ZOO_UNIMPLEMENTED;
         } else if (ZOPERATIONTIMEOUT == code) {
           return kERR_ZOO_OPERATION_TIMEOUT;
         } else if (ZBADARGUMENTS == code) {
           return kERR_ZOO_BAD_ARGUMENTS;
         } else if (ZINVALIDSTATE == code) {
           return kERR_ZOO_INVALID_STATE;
         } else if (ZAPIERROR == code) {
           return kERR_ZOO_API_ERROR;
         } else if (ZNONODE == code) {
           return kERR_ZOO_NO_NODE;
         } else if (ZNOAUTH == code) {
           return kERR_ZOO_NO_AUTH;
         } else if (ZBADVERSION == code) {
           return kERR_ZOO_BAD_VERSION;
         } else if (ZNOCHILDRENFOREPHEMERALS == code) {
           return kERR_ZOO_NO_CHILDREN_FOR_EPHEMERAL;
         } else if (ZNODEEXISTS == code) {
           return kERR_ZOO_NODE_EXISTS;
         } else if (ZNOTEMPTY == code) {
           return kERR_ZOO_NOT_EMPTY;
         } else if (ZSESSIONEXPIRED == code) {
           return kERR_ZOO_SESSION_EXPIRED;
         } else if (ZINVALIDCALLBACK == code) {
           return kERR_ZOO_INVALID_CALLBACK;
         } else if (ZINVALIDACL == code) {
           return kERR_ZOO_INVALID_ACL;
         } else if (ZAUTHFAILED == code) {
           return kERR_ZOO_AUTH_FAILED;
         } else if (ZCLOSING == code) {
           return kERR_ZOO_CLOSING;
         } else if (ZNOTHING == code) {
           return kERR_ZOO_NOTHING;
         } else if (ZSESSIONMOVED == code) {
           return kERR_ZOO_SESSION_MOVED;
         }

         return kERR_ZOO_UNKNOWN;
       }
    };
  }  // namespace dist
}  // namespace lib

#endif  // COMMLIB_DIST_INC_ERR_H_