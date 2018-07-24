#ifndef COMMLIB_DIST_INC_ELECTION_SINGLE_MASTER_H_
#define COMMLIB_DIST_INC_ELECTION_SINGLE_MASTER_H_

#include <stdlib.h>
#include <string>
#include <map>
#include <vector>
#include <algorithm>
#include "commlib/public/inc/type.h"
#include "commlib/dist/inc/err.h"
#include "commlib/dist/inc/log.h"
#include "commlib/dist/inc/election.h"
#include "commlib/thread/inc/thread.h"

namespace lib {
  namespace dist {
    using std::string;
    using std::map;
    using std::vector;
    using std::find;

    using lib::thread::Thread;

    class ElectionSingleMaster : public Election, public Thread {
     public:
       ElectionSingleMaster(const string & hosts, INT32 timeout,
                            const string & path, const string & node_id):Election(hosts, timeout, path),
                                                                         node_id_(node_id),
                                                                         master_node_id_(path + "_master"),
                                                                         slave_parent_node_id_(path + "_slave_list") {}
       virtual ~ElectionSingleMaster() {}

     public:
       virtual INT32 MasterNodeRegister();
       virtual INT32 SlaveNodeRegister();
       virtual INT32 Initial();
       virtual INT32 ReadMasterAndSlaveList() { return 0; }

     public:
       virtual VOID Dump();
       virtual const string GetNodeID() { return node_id_; }
       virtual const string GetMasterNodeID() { return master_node_id_; }
       virtual const string GetSlaveParentNodeID() { return slave_parent_node_id_; }
       virtual const string GetSlaveNodeID() { return (slave_parent_node_id_ + "/" + node_id_); }

     public:
       virtual INT32 Working(VOID * parameter);

     public:
       INT32 ReadMasterNodeData();
       INT32 ReadSlaveParentChildren();
       INT32 SlaveParentNodeRegister();
       INT32 RegisterMasterDelay(INT32 millisecond);

     public:
       BOOL IsSlaveParentChild();

     public:
       VOID Watcher(INT32 type, INT32 state, const CHAR * node);
       VOID MasterWatcher(INT32 type, INT32 state);
       VOID SlaveParentWatcher(INT32 type, INT32 state);

     public:
       VOID MasterRegCompletion(INT32 rc);
       VOID SlaveParentRegCompletion(INT32 rc);
       VOID SlaveNodeRegCompletion(INT32 rc);
       VOID RegCompletion(INT32 rc, OPERATION op);

     public:
       static VOID Watcher(zhandle_t* zh, INT32 type, INT32 state, const CHAR * node, VOID * watcher_ctx);
       static VOID RegisterCompletion(INT32 rc, const CHAR * node, const VOID * data);

     private:
       string node_id_;
       string master_node_id_;
       string slave_parent_node_id_;
    };
  }  // namespace dist
}  // namespace lib

#endif  // COMMLIB_DIST_INC_ELECTION_SINGLE_MASTER_H_