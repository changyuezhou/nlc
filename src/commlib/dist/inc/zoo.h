// Copyright (c) 2013 zhou chang yue. All rights reserved.

#ifndef COMMLIB_DIST_INC_ZOO_H_
#define COMMLIB_DIST_INC_ZOO_H_

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <map>
#include <vector>
#include <utility>
#include <zookeeper/zookeeper.h>
#include "commlib/public/inc/type.h"
#include "commlib/dist/inc/err.h"
#include "commlib/dist/inc/log.h"

namespace lib {
  namespace dist {
    using std::string;

    class Zoo {
     public:
       Zoo():zk_handle_(NULL) {}
       ~Zoo() { Destroy(); }

     public:
       INT32 Initial(const string & hosts, watcher_fn fn, INT32 receive_timeout,
                     const clientid_t * client_id, VOID * context, INT32 flag);
       zhandle_t * Handle() { return zk_handle_; }

     public:
       INT32 AddAuth(const CHAR * scheme,
                     const CHAR * cert, INT32 cert_len,
                     void_completion_t completion, const VOID * data);
       INT32 Create(const CHAR * path,
                    const CHAR * value, INT32 value_len,
                    const struct ACL_vector * acl, INT32 flags,
                    CHAR * path_buffer, INT32 path_buffer_len);
       INT32 Delete(const CHAR * path, INT32 version);
       INT32 Exists(const CHAR * path, INT32 watch, struct Stat * stat);
       INT32 WExists(const CHAR * path, watcher_fn watcher, VOID * watcher_ctx, struct Stat * stat);
       INT32 Get(const CHAR * path, INT32 watch, CHAR * buffer, INT32 * buffer_len, struct Stat * stat);
       INT32 WGet(const CHAR * path, watcher_fn watcher, VOID * watcher_ctx, CHAR * buffer,
                  INT32 * buffer_len, struct Stat * stat);
       INT32 Set(const CHAR * path, const CHAR * buffer, INT32 buf_len, INT32 version);
       INT32 Set2(const CHAR * path, const CHAR * buffer, INT32 buf_len, INT32 version, struct Stat * stat);
       INT32 GetChildren(const CHAR * path, INT32 watch, struct String_vector * strings);
       INT32 WGetChildren(const CHAR * path, watcher_fn watcher, VOID * watcher_ctx, struct String_vector * strings);
       INT32 GetChildren2(const CHAR * path, INT32 watch, struct String_vector * strings, struct Stat * stat);
       INT32 WGetChildren2(const CHAR * path, watcher_fn watcher, VOID * watcher_ctx, struct String_vector * strings,
                           struct Stat * stat);
       INT32 GetACL(const CHAR * path, struct ACL_vector * acl, struct Stat * stat);
       INT32 SetACL(const CHAR * path, INT32 version, const struct ACL_vector * acl);
       INT32 Multi(INT32 count, const zoo_op_t * ops, zoo_op_result_t * results);

     public:
       INT32 ACreate(const string & path,
                     const CHAR * value, INT32 value_len,
                     const struct ACL_vector * acl, INT32 flags,
                     string_completion_t completion, const VOID * data);
       INT32 ADelete(const CHAR * path, INT32 version, void_completion_t completion, const VOID * data);
       INT32 AExists(const CHAR * path, INT32 watch, stat_completion_t completion, const VOID * data);
       INT32 AWExists(const CHAR * path, watcher_fn watcher, VOID * watcher_ctx, stat_completion_t completion,
                      const VOID * data);
       INT32 AGet(const CHAR * path, INT32 watch, data_completion_t completion, const VOID * data);
       INT32 AWGet(const CHAR * path, watcher_fn watcher, VOID * watcher_ctx, data_completion_t completion,
                   const VOID * data);
       INT32 ASet(const CHAR * path, const CHAR * buffer, INT32 buf_len, INT32 version, stat_completion_t completion,
                  const VOID * data);
       INT32 AGetChildren(const CHAR * path, INT32 watch, strings_completion_t completion, const VOID * data);
       INT32 AWGetChildren(const CHAR * path, watcher_fn watcher, VOID * watcher_ctx, strings_completion_t completion,
                           const VOID * data);
       INT32 AGetChildren2(const CHAR * path, INT32 watch, strings_stat_completion_t completion, const VOID * data);
       INT32 AWGetChildren2(const CHAR * path, watcher_fn watcher, VOID * watcher_ctx, strings_stat_completion_t completion,
                            const VOID * data);
       INT32 ASync(const CHAR * path, string_completion_t completion, const VOID * data);
       INT32 AGetACL(const CHAR * path, acl_completion_t completion, const VOID * data);
       INT32 ASetACL(const CHAR * path, INT32 version, struct ACL_vector * acl,
                     void_completion_t completion, const VOID * data);
       INT32 AMulti(INT32 count, const zoo_op_t * ops, zoo_op_result_t * results,
                    void_completion_t completion, const VOID * data);

     public:
       INT32 GetReceiveTimeout();
       VOID SetContext(VOID * context);
       const VOID * GetContext();
       watcher_fn SetWatcher(watcher_fn new_fn);
       struct sockaddr * GetConnectedHost(struct sockaddr *addr, socklen_t * addr_len);
       const clientid_t * GetClientID();
       INT32 Interest(INT32 * fd, INT32 * interest, struct timeval * tv);
       INT32 Process(INT32 events);
       INT32 State();
       const CHAR * ErrMsg(INT32 err);
       BOOL IsUnrecoverable();
       VOID DeterministicConnOrder(INT32 yes_or_no);
       BOOL IsConnected();

     public:
       VOID CreateOpInit(zoo_op_t * op, const CHAR * path, const CHAR * value,
                         INT32 value_len, const struct ACL_vector * acl,
                         INT32 flags, CHAR * path_buffer, INT32 path_buffer_len);
       VOID DeleteOpInit(zoo_op_t * op, const CHAR * path, INT32 version);
       VOID SetOpInit(zoo_op_t * op, const CHAR * path,
                           const CHAR * buffer, INT32 buf_len, INT32 version,
                           struct Stat *stat);
       VOID CheckOpInit(zoo_op_t * op, const CHAR * path, INT32 version);

     public:
       VOID SetLogStream(FILE * log_stream);
       VOID SetDebugLevel(ZooLogLevel log_level);

     public:
       VOID Destroy();

     protected:
       VOID DumpStat(struct Stat * stat);

     private:
       zhandle_t * zk_handle_;
    };
  }  // namespace dist
}  // namespace lib

#endif  // COMMLIB_DIST_INC_ZOO_H_