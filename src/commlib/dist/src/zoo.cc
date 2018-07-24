// Copyright (c) 2013 zhou chang yue. All rights reserved.

#include "commlib/dist/inc/zoo.h"
#include "commlib/dist/inc/log.h"

namespace lib {
  namespace dist {
    INT32 Zoo::Initial(const string & hosts, watcher_fn fn, INT32 receive_timeout,
                       const clientid_t * client_id, VOID * context, INT32 flag) {
      zk_handle_ = zookeeper_init(hosts.c_str(), fn, receive_timeout, client_id, context, flag);
      if (NULL == zk_handle_) {
        LIB_DIST_LOG_ERROR("zookeeper initial failed ............................");
        return Err::kERR_ZOO_INITIAL_FAILED;
      }

      return 0;
    }

    // sync operation **********************************************************************************************
    INT32 Zoo::AddAuth(const CHAR * scheme,
                       const CHAR * cert, INT32 cert_len,
                       void_completion_t completion, const VOID * data) {
      if (NULL == zk_handle_) {
        LIB_DIST_LOG_ERROR("zookeeper handle is invalid ............................");
        return Err::kERR_ZOO_HANDLE_INVALID;
      }

      INT32 result = zoo_add_auth(zk_handle_, scheme, cert, cert_len, completion, data);
      if (ZOK != result) {
        LIB_DIST_LOG_ERROR("zookeeper add auth failed result:" << Err::ZooErrStatus(result)
                                                               << " msg:" << ErrMsg(result) << " ..........");
        return Err::ZooErrStatus(result);
      }

      return 0;
    }

    INT32 Zoo::Create(const CHAR * path,
                      const CHAR * value, INT32 value_len,
                      const struct ACL_vector * acl, INT32 flags,
                      CHAR * path_buffer, INT32 path_buffer_len) {
      if (NULL == zk_handle_) {
        LIB_DIST_LOG_ERROR("zookeeper handle is invalid ............................");
        return Err::kERR_ZOO_HANDLE_INVALID;
      }

      INT32 result = zoo_create(zk_handle_, path, value, value_len, acl, flags, path_buffer, path_buffer_len);
      if (ZOK != result) {
        LIB_DIST_LOG_ERROR("zookeeper create sync failed result:" << Err::ZooErrStatus(result)
                                                                  << " msg:" << ErrMsg(result) << " ..........");
        return Err::ZooErrStatus(result);
      }

      LIB_DIST_LOG_DEBUG("zookeeper set path:" << path << " value:" << value << " len:" << value_len << " success ........");

      return 0;
    }

    INT32 Zoo::Delete(const CHAR * path, INT32 version) {
      if (NULL == zk_handle_) {
        LIB_DIST_LOG_ERROR("zookeeper handle is invalid ............................");
        return Err::kERR_ZOO_HANDLE_INVALID;
      }

      INT32 result = zoo_delete(zk_handle_, path, version);
      if (ZOK != result) {
        LIB_DIST_LOG_ERROR("zookeeper delete failed result:" << Err::ZooErrStatus(result)
                                                             << " msg:" << ErrMsg(result) << " ..........");
        return Err::ZooErrStatus(result);
      }

      return 0;
    }

    INT32 Zoo::Exists(const CHAR * path, INT32 watch, struct Stat * stat) {
      if (NULL == zk_handle_) {
        LIB_DIST_LOG_ERROR("zookeeper handle is invalid ............................");
        return Err::kERR_ZOO_HANDLE_INVALID;
      }

      INT32 result = zoo_exists(zk_handle_, path, watch, stat);
      if (ZNONODE == result) {
        return Err::ZooErrStatus(result);
      }

      if (ZOK != result) {
        LIB_DIST_LOG_ERROR("zookeeper exists failed result:" << Err::ZooErrStatus(result)
                                                             << " msg:" << ErrMsg(result)
                                                             << " path:" << path
                                                             << " ..........");
        return Err::ZooErrStatus(result);
      }

      return 0;
    }

    INT32 Zoo::WExists(const CHAR * path, watcher_fn watcher, VOID * watcher_ctx, struct Stat * stat) {
      if (NULL == zk_handle_) {
        LIB_DIST_LOG_ERROR("zookeeper handle is invalid ............................");
        return Err::kERR_ZOO_HANDLE_INVALID;
      }

      INT32 result = zoo_wexists(zk_handle_, path, watcher, watcher_ctx, stat);
      if (ZNONODE == result) {
        return Err::ZooErrStatus(result);
      }

      if (ZOK != result) {
        LIB_DIST_LOG_ERROR("zookeeper watcher exists failed result:" << Err::ZooErrStatus(result)
                                                                     << " msg:" << ErrMsg(result)
                                                                     << " path:" << path
                                                                     << " ..........");
        return Err::ZooErrStatus(result);
      }

      return 0;
    }

    INT32 Zoo::Get(const CHAR * path, INT32 watch, CHAR * buffer, INT32 * buffer_len, struct Stat * stat) {
      if (NULL == zk_handle_) {
        LIB_DIST_LOG_ERROR("zookeeper handle is invalid ............................");
        return Err::kERR_ZOO_HANDLE_INVALID;
      }

      INT32 result = zoo_get(zk_handle_, path, watch, buffer, buffer_len, stat);
      if (ZOK != result) {
        LIB_DIST_LOG_ERROR("zookeeper get failed result:" << Err::ZooErrStatus(result)
                                                          << " msg:" << ErrMsg(result)
                                                          << " path:" << path
                                                          << " ..........");
        return Err::ZooErrStatus(result);
      }

      LIB_DIST_LOG_DEBUG("zookeeper get path:" << path << " value:" << buffer << " len:" << *buffer_len << " success .");

      DumpStat(stat);

      return 0;
    }

    INT32 Zoo::WGet(const CHAR * path, watcher_fn watcher, VOID * watcher_ctx, CHAR * buffer,
               INT32 * buffer_len, struct Stat * stat) {
      if (NULL == zk_handle_) {
        LIB_DIST_LOG_ERROR("zookeeper handle is invalid ............................");
        return Err::kERR_ZOO_HANDLE_INVALID;
      }

      INT32 result = zoo_wget(zk_handle_, path, watcher, watcher_ctx, buffer, buffer_len, stat);
      if (ZOK != result) {
        LIB_DIST_LOG_ERROR("zookeeper watcher get failed result:" << Err::ZooErrStatus(result)
                                                                  << " msg:" << ErrMsg(result)
                                                                  << " path:" << path
                                                                  << " ..........");
        return Err::ZooErrStatus(result);
      }

      return 0;
    }

    INT32 Zoo::Set(const CHAR * path, const CHAR * buffer, INT32 buf_len, INT32 version) {
      if (NULL == zk_handle_) {
        LIB_DIST_LOG_ERROR("zookeeper handle is invalid ............................");
        return Err::kERR_ZOO_HANDLE_INVALID;
      }

      INT32 result = zoo_set(zk_handle_, path, buffer, buf_len, version);
      if (ZOK != result) {
        LIB_DIST_LOG_ERROR("zookeeper set failed result:" << Err::ZooErrStatus(result)
                                                          << " msg:" << ErrMsg(result)
                                                          << " path:" << path
                                                          << " ..........");
        return Err::ZooErrStatus(result);
      }

      return 0;
    }

    INT32 Zoo::Set2(const CHAR * path, const CHAR * buffer, INT32 buf_len, INT32 version, struct Stat * stat) {
      if (NULL == zk_handle_) {
        LIB_DIST_LOG_ERROR("zookeeper handle is invalid ............................");
        return Err::kERR_ZOO_HANDLE_INVALID;
      }

      INT32 result = zoo_set2(zk_handle_, path, buffer, buf_len, version, stat);
      if (ZOK != result) {
        LIB_DIST_LOG_ERROR("zookeeper set2 failed result:" << Err::ZooErrStatus(result)
                                                           << " msg:" << ErrMsg(result)
                                                           << " path:" << path
                                                           << " ..........");
        return Err::ZooErrStatus(result);
      }

      return 0;
    }

    INT32 Zoo::GetChildren(const CHAR * path, INT32 watch, struct String_vector * strings) {
      if (NULL == zk_handle_) {
        LIB_DIST_LOG_ERROR("zookeeper handle is invalid ............................");
        return Err::kERR_ZOO_HANDLE_INVALID;
      }

      INT32 result = zoo_get_children(zk_handle_, path, watch, strings);
      if (ZOK != result) {
        LIB_DIST_LOG_ERROR("zookeeper get children failed result:" << Err::ZooErrStatus(result)
                                                                   << " msg:" << ErrMsg(result)
                                                                   << " path:" << path
                                                                   << " ..........");
        return Err::ZooErrStatus(result);
      }

      return 0;
    }

    INT32 Zoo::WGetChildren(const CHAR * path, watcher_fn watcher, VOID * watcher_ctx, struct String_vector * strings) {
      if (NULL == zk_handle_) {
        LIB_DIST_LOG_ERROR("zookeeper handle is invalid ............................");
        return Err::kERR_ZOO_HANDLE_INVALID;
      }

      INT32 result = zoo_wget_children(zk_handle_, path, watcher, watcher_ctx, strings);
      if (ZOK != result) {
        LIB_DIST_LOG_ERROR("zookeeper watcher get children failed result:" << Err::ZooErrStatus(result)
                                                                           << " msg:" << ErrMsg(result)
                                                                           << " path:" << path
                                                                           << " ..........");
        return Err::ZooErrStatus(result);
      }

      return 0;
    }

    INT32 Zoo::GetChildren2(const CHAR * path, INT32 watch, struct String_vector * strings, struct Stat * stat) {
      if (NULL == zk_handle_) {
        LIB_DIST_LOG_ERROR("zookeeper handle is invalid ............................");
        return Err::kERR_ZOO_HANDLE_INVALID;
      }

      INT32 result = zoo_get_children2(zk_handle_, path, watch, strings, stat);
      if (ZOK != result) {
        LIB_DIST_LOG_ERROR("zookeeper get children2 failed result:" << Err::ZooErrStatus(result)
                                                                    << " msg:" << ErrMsg(result)
                                                                    << " path:" << path
                                                                    << " ..........");
        return Err::ZooErrStatus(result);
      }

      return 0;
    }

    INT32 Zoo::WGetChildren2(const CHAR * path, watcher_fn watcher, VOID * watcher_ctx, struct String_vector * strings,
                             struct Stat * stat) {
      if (NULL == zk_handle_) {
        LIB_DIST_LOG_ERROR("zookeeper handle is invalid ............................");
        return Err::kERR_ZOO_HANDLE_INVALID;
      }

      INT32 result = zoo_wget_children2(zk_handle_, path, watcher, watcher_ctx, strings, stat);
      if (ZOK != result) {
        LIB_DIST_LOG_ERROR("zookeeper watcher get children2 failed result:" << Err::ZooErrStatus(result)
                                                                            << " msg:" << ErrMsg(result)
                                                                            << " path:" << path
                                                                            << " .....");
        return Err::ZooErrStatus(result);
      }

      return 0;
    }

    INT32 Zoo::GetACL(const CHAR * path, struct ACL_vector * acl, struct Stat * stat) {
      if (NULL == zk_handle_) {
        LIB_DIST_LOG_ERROR("zookeeper handle is invalid ............................");
        return Err::kERR_ZOO_HANDLE_INVALID;
      }

      INT32 result = zoo_get_acl(zk_handle_, path, acl, stat);
      if (ZOK != result) {
        LIB_DIST_LOG_ERROR("zookeeper get acl failed result:" << Err::ZooErrStatus(result)
                                                              << " msg:" << ErrMsg(result)
                                                              << " path:" << path
                                                              << " .....");
        return Err::ZooErrStatus(result);
      }

      return 0;
    }

    INT32 Zoo::SetACL(const CHAR * path, INT32 version, const struct ACL_vector * acl) {
      if (NULL == zk_handle_) {
        LIB_DIST_LOG_ERROR("zookeeper handle is invalid ............................");
        return Err::kERR_ZOO_HANDLE_INVALID;
      }

      INT32 result = zoo_set_acl(zk_handle_, path, version, acl);
      if (ZOK != result) {
        LIB_DIST_LOG_ERROR("zookeeper set acl failed result:" << Err::ZooErrStatus(result)
                                                              << " msg:" << ErrMsg(result)
                                                              << " path:" << path
                                                              << " .....");
        return Err::ZooErrStatus(result);
      }

      return 0;
    }

    INT32 Zoo::Multi(INT32 count, const zoo_op_t * ops, zoo_op_result_t * results) {
      if (NULL == zk_handle_) {
        LIB_DIST_LOG_ERROR("zookeeper handle is invalid ............................");
        return Err::kERR_ZOO_HANDLE_INVALID;
      }

      INT32 result = zoo_multi(zk_handle_, count, ops, results);
      if (ZOK != result) {
        LIB_DIST_LOG_ERROR("zookeeper set multi operation failed result:" << Err::ZooErrStatus(result)
                                                                          << " msg:" << ErrMsg(result)
                                                                          << " .....");
        return Err::ZooErrStatus(result);
      }

      return 0;
    }

    // async operation **********************************************************************************************
    INT32 Zoo::ACreate(const string & path,
                       const CHAR * value, INT32 value_len,
                       const struct ACL_vector * acl, INT32 flags,
                       string_completion_t completion, const VOID * data) {
      if (NULL == zk_handle_) {
        LIB_DIST_LOG_ERROR("zookeeper handle is invalid ............................");
        return Err::kERR_ZOO_HANDLE_INVALID;
      }

      INT32 result = zoo_acreate(zk_handle_, path.c_str(), value, value_len, acl, flags, completion, data);
      if (ZOK != result) {
        LIB_DIST_LOG_ERROR("zookeeper node create async failed result:" << Err::ZooErrStatus(result)
                                                                        << " msg:" << ErrMsg(result)
                                                                        << " path:" << path
                                                                        << " ..........");
        return Err::ZooErrStatus(result);
      }

      return 0;
    }

    INT32 Zoo::ADelete(const CHAR * path, INT32 version, void_completion_t completion, const VOID * data) {
      if (NULL == zk_handle_) {
        LIB_DIST_LOG_ERROR("zookeeper handle is invalid ............................");
        return Err::kERR_ZOO_HANDLE_INVALID;
      }

      INT32 result = zoo_adelete(zk_handle_, path, version, completion, data);
      if (ZOK != result) {
        LIB_DIST_LOG_ERROR("zookeeper node delete async failed result:" << Err::ZooErrStatus(result)
                                                                        << " msg:" << ErrMsg(result)
                                                                        << " path:" << path
                                                                        << " ..........");
        return Err::ZooErrStatus(result);
      }

      return 0;
    }

    INT32 Zoo::AExists(const CHAR * path, INT32 watch, stat_completion_t completion, const VOID * data) {
      if (NULL == zk_handle_) {
        LIB_DIST_LOG_ERROR("zookeeper handle is invalid ............................");
        return Err::kERR_ZOO_HANDLE_INVALID;
      }

      INT32 result = zoo_aexists(zk_handle_, path, watch, completion, data);
      if (ZOK != result) {
        LIB_DIST_LOG_ERROR("zookeeper node exists async failed result:" << Err::ZooErrStatus(result)
                                                                        << " msg:" << ErrMsg(result)
                                                                        << " path:" << path
                                                                        << " ..........");
        return Err::ZooErrStatus(result);
      }

      return 0;
    }

    INT32 Zoo::AWExists(const CHAR * path, watcher_fn watcher, VOID * watcher_ctx, stat_completion_t completion,
                        const VOID * data) {
      if (NULL == zk_handle_) {
        LIB_DIST_LOG_ERROR("zookeeper handle is invalid ............................");
        return Err::kERR_ZOO_HANDLE_INVALID;
      }

      INT32 result = zoo_awexists(zk_handle_, path, watcher, watcher_ctx, completion, data);
      if (ZOK != result) {
        LIB_DIST_LOG_ERROR("zookeeper watcher exists async failed result:" << Err::ZooErrStatus(result)
                                                                           << " msg:" << ErrMsg(result)
                                                                           << " path:" << path
                                                                           << " ..........");
        return Err::ZooErrStatus(result);
      }

      return 0;
    }

    INT32 Zoo::AGet(const CHAR * path, INT32 watch, data_completion_t completion, const VOID * data) {
      if (NULL == zk_handle_) {
        LIB_DIST_LOG_ERROR("zookeeper handle is invalid ............................");
        return Err::kERR_ZOO_HANDLE_INVALID;
      }

      INT32 result = zoo_aget(zk_handle_, path, watch, completion, data);
      if (ZOK != result) {
        LIB_DIST_LOG_ERROR("zookeeper get async failed result:" << Err::ZooErrStatus(result)
                                                                << " msg:" << ErrMsg(result)
                                                                << " path:" << path
                                                                << " ..........");
        return Err::ZooErrStatus(result);
      }

      return 0;
    }

    INT32 Zoo::AWGet(const CHAR * path, watcher_fn watcher, VOID * watcher_ctx, data_completion_t completion,
                     const VOID * data) {
      if (NULL == zk_handle_) {
        LIB_DIST_LOG_ERROR("zookeeper handle is invalid ............................");
        return Err::kERR_ZOO_HANDLE_INVALID;
      }

      INT32 result = zoo_awget(zk_handle_, path, watcher, watcher_ctx, completion, data);
      if (ZOK != result) {
        LIB_DIST_LOG_ERROR("zookeeper watcher get async failed result:" << Err::ZooErrStatus(result)
                                                                        << " msg:" << ErrMsg(result)
                                                                        << " path:" << path
                                                                        << " ..........");
        return Err::ZooErrStatus(result);
      }

      return 0;
    }

    INT32 Zoo::ASet(const CHAR * path, const CHAR * buffer, INT32 buf_len, INT32 version, stat_completion_t completion,
                    const VOID * data) {
      if (NULL == zk_handle_) {
        LIB_DIST_LOG_ERROR("zookeeper handle is invalid ............................");
        return Err::kERR_ZOO_HANDLE_INVALID;
      }

      INT32 result = zoo_aset(zk_handle_, path, buffer, buf_len, version, completion, data);
      if (ZOK != result) {
        LIB_DIST_LOG_ERROR("zookeeper set async failed result:" << Err::ZooErrStatus(result)
                                                                << " msg:" << ErrMsg(result)
                                                                << " path:" << path
                                                                << " ..........");
        return Err::ZooErrStatus(result);
      }

      return 0;
    }

    INT32 Zoo::AGetChildren(const CHAR * path, INT32 watch, strings_completion_t completion, const VOID * data) {
      if (NULL == zk_handle_) {
        LIB_DIST_LOG_ERROR("zookeeper handle is invalid ............................");
        return Err::kERR_ZOO_HANDLE_INVALID;
      }

      INT32 result = zoo_aget_children(zk_handle_, path, watch, completion, data);
      if (ZOK != result) {
        LIB_DIST_LOG_ERROR("zookeeper get children async failed result:" << Err::ZooErrStatus(result)
                                                                         << " msg:" << ErrMsg(result)
                                                                         << " path:" << path
                                                                         << " ..........");
        return Err::ZooErrStatus(result);
      }

      return 0;
    }

    INT32 Zoo::AWGetChildren(const CHAR * path, watcher_fn watcher, VOID * watcher_ctx, strings_completion_t completion,
                        const VOID * data) {
      if (NULL == zk_handle_) {
        LIB_DIST_LOG_ERROR("zookeeper handle is invalid ............................");
        return Err::kERR_ZOO_HANDLE_INVALID;
      }

      INT32 result = zoo_awget_children(zk_handle_, path, watcher, watcher_ctx, completion, data);
      if (ZOK != result) {
        LIB_DIST_LOG_ERROR("zookeeper watcher get node:" << path << " children async failed result:"
                                                         << Err::ZooErrStatus(result)
                                                         << " msg:" << ErrMsg(result) << " ..");
        return Err::ZooErrStatus(result);
      }

      return 0;
    }

    INT32 Zoo::AGetChildren2(const CHAR * path, INT32 watch, strings_stat_completion_t completion, const VOID * data) {
      if (NULL == zk_handle_) {
        LIB_DIST_LOG_ERROR("zookeeper handle is invalid ............................");
        return Err::kERR_ZOO_HANDLE_INVALID;
      }

      INT32 result = zoo_aget_children2(zk_handle_, path, watch, completion, data);
      if (ZOK != result) {
        LIB_DIST_LOG_ERROR("zookeeper get children 2 async failed result:" << Err::ZooErrStatus(result)
                                                                           << " msg:" << ErrMsg(result)
                                                                           << " path:" << path
                                                                           << " ..");
        return Err::ZooErrStatus(result);
      }

      return 0;
    }

    INT32 Zoo::AWGetChildren2(const CHAR * path, watcher_fn watcher, VOID * watcher_ctx, strings_stat_completion_t completion,
                              const VOID * data) {
      if (NULL == zk_handle_) {
        LIB_DIST_LOG_ERROR("zookeeper handle is invalid ............................");
        return Err::kERR_ZOO_HANDLE_INVALID;
      }

      INT32 result = zoo_awget_children2(zk_handle_, path, watcher, watcher_ctx, completion, data);
      if (ZOK != result) {
        LIB_DIST_LOG_ERROR("zookeeper watcher get children 2 async failed result:" << Err::ZooErrStatus(result)
                                                                                   << " msg:" << ErrMsg(result)
                                                                                   << " path:" << path
                                                                                   << " .");
        return Err::ZooErrStatus(result);
      }

      return 0;
    }

    INT32 Zoo::ASync(const CHAR * path, string_completion_t completion, const VOID * data) {
      if (NULL == zk_handle_) {
        LIB_DIST_LOG_ERROR("zookeeper handle is invalid ............................");
        return Err::kERR_ZOO_HANDLE_INVALID;
      }

      INT32 result = zoo_async(zk_handle_, path, completion, data);
      if (ZOK != result) {
        LIB_DIST_LOG_ERROR("zookeeper async failed result:" << Err::ZooErrStatus(result)
                                                            << " msg:" << ErrMsg(result)
                                                            << " path:" << path
                                                            << " .");
        return Err::ZooErrStatus(result);
      }

      return 0;
    }

    INT32 Zoo::AGetACL(const CHAR * path, acl_completion_t completion, const VOID * data) {
      if (NULL == zk_handle_) {
        LIB_DIST_LOG_ERROR("zookeeper handle is invalid ............................");
        return Err::kERR_ZOO_HANDLE_INVALID;
      }

      INT32 result = zoo_aget_acl(zk_handle_, path, completion, data);
      if (ZOK != result) {
        LIB_DIST_LOG_ERROR("zookeeper get acl async failed result:" << Err::ZooErrStatus(result)
                                                                    << " msg:" << ErrMsg(result)
                                                                    << " path:" << path
                                                                    << " .");
        return Err::ZooErrStatus(result);
      }

      return 0;
    }

    INT32 Zoo::ASetACL(const CHAR * path, INT32 version, struct ACL_vector * acl,
                       void_completion_t completion, const VOID * data) {
      if (NULL == zk_handle_) {
        LIB_DIST_LOG_ERROR("zookeeper handle is invalid ............................");
        return Err::kERR_ZOO_HANDLE_INVALID;
      }

      INT32 result = zoo_aset_acl(zk_handle_, path, version, acl, completion, data);
      if (ZOK != result) {
        LIB_DIST_LOG_ERROR("zookeeper set acl async failed result:" << Err::ZooErrStatus(result)
                                                                    << " msg:" << ErrMsg(result)
                                                                    << " path:" << path
                                                                    << " ....");
        return Err::ZooErrStatus(result);
      }

      return 0;
    }

    INT32 Zoo::AMulti(INT32 count, const zoo_op_t * ops, zoo_op_result_t * results,
                      void_completion_t completion, const VOID * data) {
      if (NULL == zk_handle_) {
        LIB_DIST_LOG_ERROR("zookeeper handle is invalid ............................");
        return Err::kERR_ZOO_HANDLE_INVALID;
      }

      INT32 result = zoo_amulti(zk_handle_, count, ops, results, completion, data);
      if (ZOK != result) {
        LIB_DIST_LOG_ERROR("zookeeper set multi operation async failed result:" << Err::ZooErrStatus(result)
                                                                                << " msg:" << ErrMsg(result)
                                                                                << " ....");
        return Err::ZooErrStatus(result);
      }

      return 0;
    }

    INT32 Zoo::GetReceiveTimeout() {
      if (NULL == zk_handle_) {
        LIB_DIST_LOG_ERROR("zookeeper handle is invalid ............................");
        return Err::kERR_ZOO_HANDLE_INVALID;
      }

      return zoo_recv_timeout(zk_handle_);
    }

    watcher_fn Zoo::SetWatcher(watcher_fn new_fn) {
      if (NULL == zk_handle_) {
        LIB_DIST_LOG_ERROR("zookeeper handle is invalid ............................");
        return NULL;
      }

      return zoo_set_watcher(zk_handle_, new_fn);
    }

    const clientid_t * Zoo::GetClientID() {
      if (NULL == zk_handle_) {
        LIB_DIST_LOG_ERROR("zookeeper handle is invalid ............................");
        return NULL;
      }

      return zoo_client_id(zk_handle_);
    }

    const VOID * Zoo::GetContext() {
      if (NULL == zk_handle_) {
        LIB_DIST_LOG_ERROR("zookeeper handle is invalid ............................");
        return NULL;
      }

      return zoo_get_context(zk_handle_);
    }

    VOID Zoo::SetContext(VOID * context) {
      if (NULL == zk_handle_) {
        LIB_DIST_LOG_ERROR("zookeeper handle is invalid ............................");
        return;
      }

      zoo_set_context(zk_handle_, context);
    }

    struct sockaddr * Zoo::GetConnectedHost(struct sockaddr *addr, socklen_t * addr_len) {
      if (NULL == zk_handle_) {
        LIB_DIST_LOG_ERROR("zookeeper handle is invalid ............................");
        return NULL;
      }

      return zookeeper_get_connected_host(zk_handle_, addr, addr_len);
    }

    INT32 Zoo::Interest(INT32 * fd, INT32 * interest, struct timeval *tv) {
      if (NULL == zk_handle_) {
        LIB_DIST_LOG_ERROR("zookeeper handle is invalid ............................");
        return Err::kERR_ZOO_HANDLE_INVALID;
      }

      INT32 result = zookeeper_interest(zk_handle_, fd, interest, tv);

      if (ZOK != result) {
        LIB_DIST_LOG_ERROR("zookeeper interest failed result:" << Err::ZooErrStatus(result)
                                                               << " msg:" << ErrMsg(result) << " ..........");
        return Err::ZooErrStatus(result);
      }

      return 0;
    }

    INT32 Zoo::Process(INT32 events) {
      if (NULL == zk_handle_) {
        LIB_DIST_LOG_ERROR("zookeeper handle is invalid ............................");
        return Err::kERR_ZOO_HANDLE_INVALID;
      }

      INT32 result = zookeeper_process(zk_handle_, events);

      if (ZOK != result) {
        LIB_DIST_LOG_ERROR("zookeeper process failed result:" << Err::ZooErrStatus(result)
                                                              << " msg:" << ErrMsg(result) << " ..........");
        return Err::ZooErrStatus(result);
      }

      return 0;
    }

    BOOL Zoo::IsUnrecoverable() {
      if (NULL == zk_handle_) {
        LIB_DIST_LOG_ERROR("zookeeper handle is invalid ............................");
        return FALSE;
      }

      INT32 result = is_unrecoverable(zk_handle_);

      if (ZOK != result) {
        LIB_DIST_LOG_ERROR("zookeeper process failed result:" << Err::ZooErrStatus(result)
                                                              << " msg:" << ErrMsg(result) << " ..........");
        return FALSE;
      }

      return TRUE;
    }

    VOID Zoo::DeterministicConnOrder(INT32 yes_or_no) {
      if (NULL == zk_handle_) {
        LIB_DIST_LOG_ERROR("zookeeper handle is invalid ............................");
        return;
      }

      zoo_deterministic_conn_order(yes_or_no);
    }

    INT32 Zoo::State() {
      if (NULL == zk_handle_) {
        LIB_DIST_LOG_ERROR("zookeeper handle is invalid ............................");
        return Err::kERR_ZOO_HANDLE_INVALID;
      }

      return zoo_state(zk_handle_);
    }

    BOOL Zoo::IsConnected() {
      return (NULL != zk_handle_);
    }

    VOID Zoo::CreateOpInit(zoo_op_t * op, const CHAR * path, const CHAR * value,
                           INT32 value_len, const struct ACL_vector * acl,
                           INT32 flags, CHAR * path_buffer, INT32 path_buffer_len) {
      zoo_create_op_init(op, path, value, value_len, acl, flags, path_buffer, path_buffer_len);
    }

    VOID Zoo::DeleteOpInit(zoo_op_t * op, const CHAR * path, INT32 version) {
      zoo_delete_op_init(op, path, version);
    }

    VOID Zoo::SetOpInit(zoo_op_t * op, const CHAR * path,
                   const CHAR * buffer, INT32 buf_len, INT32 version,
                   struct Stat * stat) {
      zoo_set_op_init(op, path, buffer, buf_len, version, stat);
    }

    VOID Zoo::CheckOpInit(zoo_op_t * op, const CHAR * path, INT32 version) {
      zoo_check_op_init(op, path, version);
    }

    const CHAR * Zoo::ErrMsg(INT32 err) {
      return zerror(err);
    }

    VOID Zoo::SetLogStream(FILE * log_stream) {
      zoo_set_log_stream(log_stream);
    }

    VOID Zoo::SetDebugLevel(ZooLogLevel log_level) {
      zoo_set_debug_level(log_level);
    }

    VOID Zoo::DumpStat(struct Stat * stat) {
      if (!stat) {
        return;
      }

      CHAR tctimes[1024] = {0};
      CHAR tmtimes[1024] = {0};
      time_t tctime;
      time_t tmtime;

      tctime = stat->ctime/1000;
      tmtime = stat->mtime/1000;

      ::ctime_r(&tmtime, tmtimes);
      ::ctime_r(&tctime, tctimes);
      LIB_DIST_LOG_DEBUG("ctime:" << tctimes << " czxid:" << stat->czxid << " mtime:" << tmtime << " mzxid:"
                                  << stat->mzxid << " version:" << stat->version
                                  << " owner:" << stat->ephemeralOwner);
    }

    VOID Zoo::Destroy() {
      if (NULL != zk_handle_) {
        INT32 result = zookeeper_close(zk_handle_);
        if (ZOK != result) {
          LIB_DIST_LOG_ERROR("zookeeper object destroy failed result:" << result << " ............................");
        }

        zk_handle_ = NULL;
      }
    }
  }  // namespace dist
}  // namespace lib