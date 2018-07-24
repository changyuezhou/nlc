// Copyright (c) 2013 zhou chang yue. All rights reserved.

#ifndef COMMLIB_DIST_INC_ELECTION_H_
#define COMMLIB_DIST_INC_ELECTION_H_

#include <stdlib.h>
#include <string>
#include <map>
#include <vector>
#include "commlib/public/inc/type.h"
#include "commlib/dist/inc/err.h"
#include "commlib/dist/inc/log.h"
#include "commlib/dist/inc/zoo.h"

namespace lib {
  namespace dist {
    using std::string;
    using std::map;
    using std::vector;

    class Election {
     public:
       typedef vector<string> MasterList;
       typedef vector<string> SlaveList;
       typedef string Master;

     public:
       enum OPERATION {
         REGISTER_MASTER = 1,
         REGISTER_SLAVE = 2,
         REGISTER_SLAVE_PARENT = 3,
         WATCHING_MASTER = 4,
         WATCHING_SLAVE = 5,
         DELETE_SLAVE = 6,
         READ_MASTER_SLAVE_LIST = 7,
         OP_UNKNOWN = 99
       };

     public:
       typedef struct _register_completion_data {
         OPERATION op_;
         VOID * election_;

         _register_completion_data() {
           op_ = OP_UNKNOWN;
           election_ = NULL;
         }
       } RegisterCompletionData;

     public:
       Election(const string & hosts, INT32 timeout, const string & path): path_(path), hosts_(hosts),
                                                                           timeout_(timeout), is_master_(FALSE),
                                                                           master_id_(""),
                                                                           success_op_(OP_UNKNOWN),
                                                                           failed_op_(OP_UNKNOWN) {}
       virtual ~Election() { Destroy(); }

     public:
       virtual INT32 MasterNodeRegister() = 0;
       virtual INT32 SlaveNodeRegister() = 0;
       virtual INT32 Initial() = 0;
       virtual INT32 ReadMasterAndSlaveList() = 0;

     public:
       INT32 InitialZoo(VOID * data, OPERATION success_op);
       BOOL  IsNodeExists(const string & node);
       INT32 WatchingNode(const string & node, watcher_fn watcher, VOID * watcher_ctx);
       INT32 WatchingChildNodeList(const string & node, watcher_fn watcher, VOID * watcher_ctx);
       INT32 ReadNode(const string & node, INT32 watcher, CHAR * buffer, INT32 * size);
       INT32 SetNode(const string & node, INT32 watcher, CHAR * buffer, INT32 size);
       INT32 GetChildren(const string & node, INT32 watcher, struct String_vector * strings);
       INT32 DeleteNode(const string & node, INT32 version);

     public:
       virtual VOID Dump() = 0;
       virtual const string GetNodeID() = 0;
       virtual const string GetMasterNodeID() = 0;
       virtual const string GetSlaveParentNodeID() = 0;
       virtual const string GetSlaveNodeID() = 0;

     public:
       BOOL IsMaster() { return is_master_; }

     public:
       const string GetMasterID() { return master_id_; }
       const SlaveList GetSlaveList() { return slave_list_; }

     public:
       VOID InitialZookeeper(INT32 type, INT32 state);

     public:
       static VOID InitialZookeeperWatcher(zhandle_t* zh, INT32 type, INT32 state, const CHAR * path, VOID * watcher_ctx);

     public:
       VOID Destroy();

     protected:
       string path_;
       string hosts_;
       INT32 timeout_;
       BOOL is_master_;
       Master master_id_;
       OPERATION success_op_;
       OPERATION failed_op_;
       SlaveList slave_list_;
       Zoo zoo_;
    };
  }  // namespace dist
}  // namespace lib

#endif  // COMMLIB_DIST_INC_ELECTION_H_