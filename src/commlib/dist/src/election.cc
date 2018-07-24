#include "commlib/dist/inc/election.h"

namespace lib {
  namespace dist {
    INT32 Election::InitialZoo(VOID * data, OPERATION success_op) {
      INT32 result = zoo_.Initial(hosts_.c_str(), Election::InitialZookeeperWatcher, timeout_, NULL, data, 0);
      if (0 != result) {
        LIB_DIST_LOG_ERROR("Election initial zookeeper failed result:" << result);

        return result;
      }

      success_op_ = success_op;

      LIB_DIST_LOG_DEBUG("Election connect to service:" << hosts_ << " timeout:"
                                                        << timeout_ << " success .");

      return 0;
    }

    BOOL Election::IsNodeExists(const string & node) {
      if (!zoo_.IsConnected()) {
        LIB_DIST_LOG_ERROR("Election node:" << node << " exists failed connection lost ........");
        return FALSE;
      }

      struct Stat stat;
      memset(&stat, 0x00, sizeof(stat));
      INT32 result = zoo_.Exists(node.c_str(), 0, &stat);
      if (0 != result) {
        if (Err::kERR_ZOO_NO_NODE != result) {
          LIB_DIST_LOG_ERROR("Election node:" << node << " exists failed result:" << result << " ........");
        }

        return FALSE;
      }

      return TRUE;
    }

    INT32 Election::WatchingNode(const string & node, watcher_fn watcher, VOID * watcher_ctx) {
      if (!zoo_.IsConnected()) {
        LIB_DIST_LOG_ERROR("Election node:" << node << " watching failed connection lost ........");
        return Err::kERR_ZOO_CONNECTION_LOST;
      }

      struct Stat stat;
      memset(&stat, 0x00, sizeof(stat));
      INT32 result = zoo_.WExists(node.c_str(), watcher, watcher_ctx, &stat);
      if (0 != result && Err::kERR_ZOO_NO_NODE != result) {
        LIB_DIST_LOG_ERROR("Election node:" << node << " watching failed result:" << result << " ........");
        return result;
      }

      return 0;
    }

    INT32 Election::WatchingChildNodeList(const string & node, watcher_fn watcher, VOID * watcher_ctx) {
      if (!zoo_.IsConnected()) {
        LIB_DIST_LOG_ERROR("Election node:" << node << " watching failed connection lost ........");
        return Err::kERR_ZOO_CONNECTION_LOST;
      }

      struct String_vector strings;
      memset(&strings, 0x00, sizeof(strings));
      INT32 result = zoo_.WGetChildren(node.c_str(), watcher, watcher_ctx, &strings);
      if (0 != result && Err::kERR_ZOO_NO_NODE != result) {
        LIB_DIST_LOG_ERROR("Election node:" << node << " watching children failed result:" << result << " ........");
        return result;
      }

      return 0;
    }

    INT32 Election::ReadNode(const string & node, INT32 watcher, CHAR * buffer, INT32 * size) {
      if (!zoo_.IsConnected()) {
        LIB_DIST_LOG_ERROR("Election read node:" << node << " data failed connection lost ........");
        return Err::kERR_ZOO_CONNECTION_LOST;
      }

      struct Stat stat;
      memset(&stat, 0x00, sizeof(stat));
      INT32 result = zoo_.Get(node.c_str(), watcher, buffer, size, &stat);
      if (0 != result) {
        LIB_DIST_LOG_ERROR("Election read node:" << node << " data failed result:" << result);
        return result;
      }

      return 0;
    }

    INT32 Election::SetNode(const string & node, INT32 watcher, CHAR * buffer, INT32 size) {
      if (!zoo_.IsConnected()) {
        LIB_DIST_LOG_ERROR("Election set node:" << node << " data failed connection lost ........");
        return Err::kERR_ZOO_CONNECTION_LOST;
      }

      INT32 result = zoo_.Set(node.c_str(), buffer, size, -1);
      if (0 != result) {
        LIB_DIST_LOG_ERROR("Election set node:" << node << " data:" << buffer << " failed result:" << result);
        return result;
      }

      return 0;
    }

    INT32 Election::GetChildren(const string & node, INT32 watcher, struct String_vector * strings) {
      if (!zoo_.IsConnected()) {
        LIB_DIST_LOG_ERROR("Election get node:" << node << " children failed connection lost ........");
        return Err::kERR_ZOO_CONNECTION_LOST;
      }

      INT32 result = zoo_.GetChildren(node.c_str(), watcher, strings);
      if (0 != result) {
        LIB_DIST_LOG_ERROR("Election get node:" << node << "'s children failed result:" << result);
        return result;
      }

      return 0;
    }

    INT32 Election::DeleteNode(const string & node, INT32 version) {
      if (!zoo_.IsConnected()) {
        LIB_DIST_LOG_ERROR("Election delete node:" << node << " failed connection lost ........");
        return Err::kERR_ZOO_CONNECTION_LOST;
      }

      INT32 result = zoo_.Delete(node.c_str(), version);
      if (0 != result) {
        LIB_DIST_LOG_ERROR("Election delete node:" << node << " version:" << version << " failed result:" << result);
        return result;
      }

      return 0;
    }

    VOID Election::InitialZookeeper(INT32 type, INT32 state) {
      if (ZOO_SESSION_EVENT == type && ZOO_CONNECTED_STATE == state) {
        INT32 result = Initial();
        if (0 != result) {
          LIB_DIST_LOG_ERROR("Election initial failed result:" << result);

          return;
        }
        LIB_DIST_LOG_DEBUG("Election initial callback success ................");

        if (REGISTER_MASTER == success_op_) {
          result = MasterNodeRegister();
          if (0 != result) {
            LIB_DIST_LOG_ERROR("Election register master node:" << GetMasterNodeID()
                                                                << " failed result:" << result);
          }
          LIB_DIST_LOG_DEBUG("Election node:" << GetMasterNodeID() << " register master ................");
        } else if (REGISTER_SLAVE == success_op_) {
          result = SlaveNodeRegister();
          if (0 != result) {
            LIB_DIST_LOG_ERROR("Election register slave node:" << GetSlaveNodeID()
                                                               << " failed result:" << result);
          }
          LIB_DIST_LOG_DEBUG("Election slave node:" << GetSlaveNodeID() << " register ................");
        } else if (READ_MASTER_SLAVE_LIST == success_op_) {
          result = ReadMasterAndSlaveList();
          if (0 != result) {
            LIB_DIST_LOG_ERROR("Election read master:" << GetMasterNodeID() << " and slave:" << GetSlaveNodeID()
                                                       << " list failed result:" << result);
          }
          LIB_DIST_LOG_DEBUG("Election read master:" << GetMasterNodeID() << " and slave:" << GetSlaveNodeID()
                                                     << " list success ............");
          Dump();
        } else {
          LIB_DIST_LOG_WARN("Election success op:" << success_op_ << " not defined ................");
        }
      } else {
        LIB_DIST_LOG_WARN("Election type:" << type << " state:" << state << " is not capture ................");
        INT32 result = InitialZoo(reinterpret_cast<VOID *>(this), success_op_);
        if (0 != result) {
          LIB_DIST_LOG_ERROR("Election initial zookeeper failed result:" << result);
        }
      }
    }

    VOID Election::Destroy() {
      slave_list_.clear();
    }

    VOID Election::InitialZookeeperWatcher(zhandle_t* zh, INT32 type, INT32 state,
                                                      const CHAR * path, VOID * watcher_ctx) {
      LIB_DIST_LOG_DEBUG("Election InitialZookeeperWatcher zookeeper handler:" << zh << " type:" << type
                                                                               << " state:" << state
                                                                               << " path:" << path);
      Election * election = reinterpret_cast<Election *>(const_cast<VOID *>(zoo_get_context(zh)));
      election->InitialZookeeper(type, state);
    }
  }  // namespace dist
}  // namespace lib