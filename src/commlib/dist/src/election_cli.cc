#include "commlib/dist/inc/election_cli.h"

namespace lib {
  namespace dist {
    INT32 ElectionCli::Initial() {
      master_id_ = "";
      is_master_ = FALSE;
      slave_list_.clear();

      if (!zoo_.IsConnected()) {
        LIB_DIST_LOG_DEBUG("ElectionCli initial zookeeper connection .............");
        INT32 result = InitialZoo(reinterpret_cast<VOID *>(this), READ_MASTER_SLAVE_LIST);
        if (0 != result) {
          LIB_DIST_LOG_ERROR("ElectionCli initial zookeeper connection failed result:" << result);
          return result;
        }

        return 0;
      }

      INT32 result = WatchingNode(master_node_id_, ElectionCli::Watcher,
                                  reinterpret_cast<VOID *>(this));
      if (0 != result) {
        LIB_DIST_LOG_ERROR("ElectionCli watching master node:" << master_node_id_ << " failed result:"
                                                                        << result);

        return result;
      }

      LIB_DIST_LOG_DEBUG("ElectionCli watching master node:" << master_node_id_ << " success ...");

      result = WatchingNode(slave_parent_node_id_, ElectionCli::Watcher,
                            reinterpret_cast<VOID *>(this));
      if (0 != result) {
        LIB_DIST_LOG_ERROR("ElectionCli watching slave parent node:" << slave_parent_node_id_
                                                                              << " failed result:"
                                                                              << result);

        return result;
      }

      LIB_DIST_LOG_DEBUG("ElectionCli watching slave parent node:" << slave_parent_node_id_ << " success ..");

      result = WatchingChildNodeList(slave_parent_node_id_, ElectionCli::Watcher,
                                     reinterpret_cast<VOID *>(this));
      if (0 != result) {
        LIB_DIST_LOG_ERROR("ElectionCli watching slave parent node:" << slave_parent_node_id_
                                                                              << " children failed result:"
                                                                              << result);

        return result;
      }

      LIB_DIST_LOG_DEBUG("ElectionCli watching slave parent node:" << slave_parent_node_id_
                                                                   << " children success ..");

      return 0;
    }

    INT32 ElectionCli::ReadMasterAndSlaveList() {
      INT32 result = ReadMasterNode();
      if (0 != result) {
        LIB_DIST_LOG_ERROR("ElectionCli read master node:" << master_node_id_ << " failed .......");
        return result;
      }

      result = ReadSlaveParentList();
      if (0 != result) {
        LIB_DIST_LOG_ERROR("ElectionCli read slave parent node:" << slave_parent_node_id_ << " list failed .......");
        return result;
      }

      Dump();

      return 0;
    }

    INT32 ElectionCli::ReadMasterNode() {
      if (!IsNodeExists(master_node_id_)) {
        LIB_DIST_LOG_WARN("ElectionCli read master node:" << master_node_id_ << " node is not exists .......");
        return 0;
      }

      CHAR buffer[10*1024] = {0};
      INT32 size = sizeof(buffer);
      INT32 result = ReadNode(master_node_id_, 0, buffer, &size);
      if (0 != result) {
        LIB_DIST_LOG_ERROR("ElectionCli node:" << master_node_id_ << " exists "
                                               << " and get node data failed result:" << result);
        return result;
      }

      master_id_ = buffer;

      return 0;
    }

    INT32 ElectionCli::ReadSlaveParentList() {
      if (!IsNodeExists(slave_parent_node_id_)) {
        LIB_DIST_LOG_WARN("ElectionCli read slave parent:" << slave_parent_node_id_ << " node not exists ......");

        return 0;
      }

      struct String_vector strings;
      memset(&strings, 0x00, sizeof(strings));
      INT32 result = GetChildren(slave_parent_node_id_, 0, &strings);
      if (0 != result) {
        LIB_DIST_LOG_ERROR("ElectionCli node:" << slave_parent_node_id_ << " exists "
                                               << " and read children failed result:" << result);
      }

      slave_list_.clear();
      for (INT32 i = 0; i < strings.count; ++i) {
        slave_list_.push_back(strings.data[i]);
      }

      return 0;
    }

    VOID ElectionCli::Dump() {
      LIB_DIST_LOG_DEBUG("**************************************************************************************");
      LIB_DIST_LOG_DEBUG("hosts:" << hosts_ << " timeout:" << timeout_ << " node:" << "");
      LIB_DIST_LOG_DEBUG("is master:" << is_master_ << " master node id:" << master_id_);
      INT32 slave_size = slave_list_.size();
      for (INT32 i = 0; i < slave_size; ++i) {
        LIB_DIST_LOG_DEBUG("index:" << i << " slave node id:" << slave_list_[i]);
      }
      LIB_DIST_LOG_DEBUG("**************************************************************************************");
    }

    VOID ElectionCli::Watcher(INT32 type, INT32 state, const CHAR * node) {
      INT32 node_len = ::strlen(node);
      INT32 master_node_id_len = master_node_id_.length();
      INT32 slave_parent_node_len = slave_parent_node_id_.length();
      if (node_len == master_node_id_len && 0 == ::strncasecmp(node, master_node_id_.c_str(), node_len)) {
        MasterWatcher(type, state);
      } else if (node_len == slave_parent_node_len &&
                 0 == ::strncasecmp(node, slave_parent_node_id_.c_str(), node_len)) {
        SlaveParentWatcher(type, state);
      } else {
        LIB_DIST_LOG_ERROR("ElectionCli node watching error node:" << node << " is invalid ....");
      }
    }

    VOID ElectionCli::MasterWatcher(INT32 type, INT32 state) {
      LIB_DIST_LOG_DEBUG("ElectionCli master watcher type:" << type << " state:" << state);
      INT32 result = WatchingNode(master_node_id_, ElectionCli::Watcher, reinterpret_cast<VOID *>(this));
      if (0 != result) {
        LIB_DIST_LOG_ERROR("ElectionCli watching master node:" << master_node_id_
                                                               << " failed result:" << result);
      }

      if (ZOO_CREATED_EVENT == type) {
        LIB_DIST_LOG_DEBUG("ElectionCli master node:" << master_node_id_ << " has be created .............");
        result = ReadMasterNode();
        if (0 != result) {
          LIB_DIST_LOG_ERROR("ElectionCli read master node:" << master_node_id_ << " failed result:"
                                                             << result << " .....");
        }
      } else if (ZOO_DELETED_EVENT == type) {
        LIB_DIST_LOG_DEBUG("ElectionCli master node:" << master_node_id_ << " has be deleted .............");
        master_id_ = "";
      } else if (ZOO_CHANGED_EVENT == type) {
        LIB_DIST_LOG_DEBUG("ElectionCli master node:" << master_node_id_ << " has be changed .....");
        result = ReadMasterNode();
        if (0 != result) {
          LIB_DIST_LOG_ERROR("ElectionCli read master node:" << master_node_id_ << " failed result:"
                                                             << result << " .....");
        }
      } else if (ZOO_CHILD_EVENT == type) {
        LIB_DIST_LOG_WARN("ElectionCli master node:" << master_node_id_ << " children has be changed .");
      } else if (ZOO_SESSION_EVENT == type) {
        LIB_DIST_LOG_WARN("ElectionCli master node:" << master_node_id_ << " session has lost .....");
        result = Initial();
        LIB_DIST_LOG_DEBUG("ElectionCli node:" << master_node_id_ << " register master ................");
      } else if (ZOO_NOTWATCHING_EVENT == type) {
        LIB_DIST_LOG_WARN("ElectionCli master node:" << master_node_id_ << " watcher is removed ....");
      } else {
        LIB_DIST_LOG_ERROR("ElectionCli master node:" << master_node_id_ << " type:" << type
                                                      << " state:" << state);
      }
    }

    VOID ElectionCli::SlaveParentWatcher(INT32 type, INT32 state) {
      LIB_DIST_LOG_DEBUG("ElectionCli slave parent watcher type:" << type << " state:" << state);
      INT32 result = WatchingNode(slave_parent_node_id_, ElectionCli::Watcher, reinterpret_cast<VOID *>(this));
      if (0 != result) {
        LIB_DIST_LOG_ERROR("ElectionCli watching slave parent node:" << slave_parent_node_id_
                                                                     << " failed result:" << result);
      }

      result = WatchingChildNodeList(slave_parent_node_id_, ElectionCli::Watcher,
                                     reinterpret_cast<VOID *>(this));
      if (0 != result) {
        LIB_DIST_LOG_ERROR("ElectionCli watching slave parent node:" << slave_parent_node_id_
                                                                     << " children failed result:"
                                                                     << result);
      }

      if (ZOO_CREATED_EVENT == type) {
        LIB_DIST_LOG_DEBUG("ElectionCli slave parent node:" << slave_parent_node_id_ << " has be created ....");
        result = ReadSlaveParentList();
        if (0 != result) {
          LIB_DIST_LOG_ERROR("ElectionCli read slave node:" << slave_parent_node_id_ << " list failed "
                                                            << " result:" << result);
        }
        LIB_DIST_LOG_DEBUG("ElectionCli read slave node:" << slave_parent_node_id_ << " list success .....");
      } else if (ZOO_DELETED_EVENT == type) {
        LIB_DIST_LOG_WARN("ElectionCli slave parent node has be deleted .......................");
        slave_list_.clear();
      } else if (ZOO_CHANGED_EVENT == type) {
        LIB_DIST_LOG_DEBUG("ElectionCli slave parent node:" << slave_parent_node_id_ << " has be changed .....");
        result = ReadSlaveParentList();
        if (0 != result) {
          LIB_DIST_LOG_ERROR("ElectionCli read slave node:" << slave_parent_node_id_ << " list failed "
                                                            << " result:" << result);
        }
        LIB_DIST_LOG_DEBUG("ElectionCli read slave node:" << slave_parent_node_id_ << " list success .....");
      } else if (ZOO_CHILD_EVENT == type) {
        LIB_DIST_LOG_WARN("ElectionSingleMaster slave parent node:" << slave_parent_node_id_ << " child has be changed ");
        result = ReadSlaveParentList();
        if (0 != result) {
          LIB_DIST_LOG_ERROR("ElectionCli read slave node:" << slave_parent_node_id_ << " list failed "
                                                            << " result:" << result);
        }
        LIB_DIST_LOG_DEBUG("ElectionCli read slave node:" << slave_parent_node_id_ << " list success .....");
      } else if (ZOO_SESSION_EVENT == type) {
        LIB_DIST_LOG_WARN("ElectionCli slave parent node:" << slave_parent_node_id_ << " session has lost ..");

        zoo_.Destroy();
        result = Initial();
        if (0 != result) {
          LIB_DIST_LOG_ERROR("ElectionCli initial failed result:" << result);
        }
        LIB_DIST_LOG_DEBUG("ElectionCli initial success ......................");
      } else if (ZOO_NOTWATCHING_EVENT == type) {
        LIB_DIST_LOG_WARN("ElectionCli slave parent node:" << slave_parent_node_id_ << " watcher is removed .");
      } else {
        LIB_DIST_LOG_ERROR("ElectionCli slave parent node:" << slave_parent_node_id_ << " type:" << type
                                                            << " state:" << state);
      }
    }

    VOID ElectionCli::Watcher(zhandle_t* zh, INT32 type, INT32 state, const CHAR * node, VOID * watcher_ctx) {
      LIB_DIST_LOG_DEBUG("ElectionCli Watcher zookeeper handler:" << zh << " type:" << type
                                                                  << " state:" << state
                                                                  << " node:" << node);
      ElectionCli * election = reinterpret_cast<ElectionCli *>(const_cast<VOID *>(watcher_ctx));
      election->Watcher(type, state, node);
      election->Dump();
    }
  }  // namespace dist
}  // namespace lib