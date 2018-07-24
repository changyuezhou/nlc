#ifndef COMMLIB_DIST_INC_ELECTION_CLI_H_
#define COMMLIB_DIST_INC_ELECTION_CLI_H_

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

    class ElectionCli : public Election {
     public:
       ElectionCli(const string & hosts, INT32 timeout,
                   const string & path) : Election(hosts, timeout, path),
                                          master_node_id_(path + "_master"),
                                          slave_parent_node_id_(path + "_slave_list") {}
       virtual ~ElectionCli() {}

     public:
       virtual INT32 MasterNodeRegister() { return 0; };
       virtual INT32 SlaveNodeRegister() { return 0; };
       virtual INT32 Initial();
       virtual INT32 ReadMasterAndSlaveList();

     public:
       virtual VOID Dump();
       virtual const string GetNodeID() { return ""; }
       virtual const string GetMasterNodeID() { return master_node_id_; }
       virtual const string GetSlaveParentNodeID() { return slave_parent_node_id_; }
       virtual const string GetSlaveNodeID() { return ""; }

     public:
       INT32 ReadMasterNode();
       INT32 ReadSlaveParentList();

     public:
       VOID Watcher(INT32 type, INT32 state, const CHAR * node);
       VOID MasterWatcher(INT32 type, INT32 state);
       VOID SlaveParentWatcher(INT32 type, INT32 state);

     public:
       static VOID Watcher(zhandle_t* zh, INT32 type, INT32 state, const CHAR * node, VOID * watcher_ctx);

     private:
       string master_node_id_;
       string slave_parent_node_id_;
    };
  }
}

#endif  // COMMLIB_DIST_INC_ELECTION_CLI_H_