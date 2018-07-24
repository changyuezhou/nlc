#include "commlib/dist/inc/election_single_master.h"

namespace lib {
  namespace dist {
    INT32 ElectionSingleMaster::Initial() {
      master_id_ = "";
      is_master_ = FALSE;
      slave_list_.clear();

      if (!zoo_.IsConnected()) {
        LIB_DIST_LOG_DEBUG("ElectionSingleMaster initial zookeeper connection .............");
        INT32 result = InitialZoo(reinterpret_cast<VOID *>(this), REGISTER_MASTER);
        if (0 != result) {
          LIB_DIST_LOG_ERROR("ElectionSingleMaster initial zookeeper connection failed result:" << result);
          return result;
        }

        return 0;
      }

      INT32 result = WatchingNode(master_node_id_, ElectionSingleMaster::Watcher,
                                             reinterpret_cast<VOID *>(this));
      if (0 != result) {
        LIB_DIST_LOG_ERROR("ElectionSingleMaster watching master node:" << master_node_id_ << " failed result:"
                                                                        << result);

        return result;
      }

      LIB_DIST_LOG_DEBUG("ElectionSingleMaster watching master node:" << master_node_id_ << " success ...");

      result = WatchingNode(slave_parent_node_id_, ElectionSingleMaster::Watcher,
                                       reinterpret_cast<VOID *>(this));
      if (0 != result) {
        LIB_DIST_LOG_ERROR("ElectionSingleMaster watching slave parent node:" << slave_parent_node_id_
                                                                              << " failed result:"
                                                                              << result);

        return result;
      }

      LIB_DIST_LOG_DEBUG("ElectionSingleMaster watching slave parent node:" << slave_parent_node_id_ << " success ..");

      result = WatchingChildNodeList(slave_parent_node_id_, ElectionSingleMaster::Watcher,
                            reinterpret_cast<VOID *>(this));
      if (0 != result) {
        LIB_DIST_LOG_ERROR("ElectionSingleMaster watching slave parent node:" << slave_parent_node_id_
                                                                              << " children failed result:"
                                                                              << result);

        return result;
      }

      LIB_DIST_LOG_DEBUG("ElectionSingleMaster watching slave parent node:" << slave_parent_node_id_
                                                                            << " children success ..");

      return 0;
    }

    INT32 ElectionSingleMaster::MasterNodeRegister() {
      if (!zoo_.IsConnected()) {
        LIB_DIST_LOG_DEBUG("ElectionSingleMaster initial zookeeper connection .............");
        INT32 result = InitialZoo(reinterpret_cast<VOID *>(this), REGISTER_MASTER);
        if (0 != result) {
          LIB_DIST_LOG_ERROR("ElectionMultiMaster initial zookeeper connection failed result:" << result);
          return result;
        }

        return 0;
      }

      RegisterCompletionData * reg_data = new RegisterCompletionData();
      reg_data->op_ = REGISTER_MASTER;
      reg_data->election_ = reinterpret_cast<VOID *>(this);

      INT32 result = zoo_.ACreate(master_node_id_.c_str(), node_id_.c_str(), node_id_.length(),
                                  &ZOO_OPEN_ACL_UNSAFE, ZOO_EPHEMERAL,
                                  ElectionSingleMaster::RegisterCompletion, reinterpret_cast<VOID *>(reg_data));

      if (0 != result) {
        LIB_DIST_LOG_ERROR("ElectionSingleMaster register master node:" << master_node_id_
                                                                        << " failed result:" << result);

        return result;
      }

      LIB_DIST_LOG_DEBUG("ElectionSingleMaster register master node:" << master_node_id_ << " node id:" << node_id_);

      return 0;
    }

    INT32 ElectionSingleMaster::SlaveNodeRegister() {
      if (!zoo_.IsConnected()) {
        LIB_DIST_LOG_DEBUG("ElectionSingleMaster initial zookeeper connection .............");
        INT32 result = InitialZoo(reinterpret_cast<VOID *>(this), REGISTER_SLAVE);
        if (0 != result) {
          LIB_DIST_LOG_ERROR("ElectionMultiMaster initial zookeeper connection failed result:" << result);
          return result;
        }

        return 0;
      }

      if (!IsNodeExists(slave_parent_node_id_)) {
        INT32 result = SlaveParentNodeRegister();
        if (0 != result) {
          LIB_DIST_LOG_ERROR("ElectionMultiMaster register slave parent node:" << slave_parent_node_id_
                                                                               << " failed result:" << result);
        }
        LIB_DIST_LOG_DEBUG("ElectionMultiMaster register slave parent node:" << slave_parent_node_id_ << " .");

        return 0;
      }

      RegisterCompletionData * reg_data = new RegisterCompletionData();
      reg_data->op_ = REGISTER_SLAVE;
      reg_data->election_ = reinterpret_cast<VOID *>(this);
      const string child_node = slave_parent_node_id_ + "/" + node_id_;
      INT32 result = zoo_.ACreate(child_node.c_str(), node_id_.c_str(), node_id_.length(),
                                  &ZOO_OPEN_ACL_UNSAFE, ZOO_EPHEMERAL,
                                  ElectionSingleMaster::RegisterCompletion, reinterpret_cast<VOID *>(reg_data));

      if (0 != result) {
        LIB_DIST_LOG_ERROR("ElectionSingleMaster register slave node:" << child_node
                                                                       << " failed result:" << result);

        return result;
      }

      LIB_DIST_LOG_DEBUG("ElectionSingleMaster register slave node:" << child_node);

      return 0;
    }

    INT32 ElectionSingleMaster::SlaveParentNodeRegister() {
      if (!zoo_.IsConnected()) {
        LIB_DIST_LOG_DEBUG("ElectionSingleMaster initial zookeeper connection .............");
        INT32 result = InitialZoo(reinterpret_cast<VOID *>(this), REGISTER_SLAVE);
        if (0 != result) {
          LIB_DIST_LOG_ERROR("ElectionMultiMaster initial zookeeper connection failed result:" << result);
          return result;
        }

        return 0;
      }

      RegisterCompletionData * reg_data = new RegisterCompletionData();
      reg_data->op_ = REGISTER_SLAVE_PARENT;
      reg_data->election_ = reinterpret_cast<VOID *>(this);

      INT32 result = zoo_.ACreate(slave_parent_node_id_.c_str(), "", 0, &ZOO_OPEN_ACL_UNSAFE, 0,
                                  ElectionSingleMaster::RegisterCompletion, reinterpret_cast<VOID *>(reg_data));

      if (0 != result) {
        LIB_DIST_LOG_ERROR("ElectionSingleMaster register slave parent node:" << slave_parent_node_id_
                                                                              << " failed result:" << result);

        return result;
      }

      LIB_DIST_LOG_DEBUG("ElectionSingleMaster register slave parent node:" << slave_parent_node_id_);

      return 0;
    }

    INT32 ElectionSingleMaster::ReadMasterNodeData() {
      CHAR buffer[10*1024] = {0};
      INT32 size = sizeof(buffer);
      INT32 result = ReadNode(master_node_id_, 0, buffer, &size);
      if (0 != result) {
        LIB_DIST_LOG_ERROR("ElectionSingleMaster node:" << master_node_id_ << " exists "
                                                        << " and get node data failed result:" << result);
        return result;
      }

      master_id_ = buffer;
      INT32 master_id_len = master_id_.length();
      INT32 node_id_len = node_id_.length();
      if (master_id_len == node_id_len && 0 == ::strncasecmp(master_id_.c_str(), node_id_.c_str(), node_id_len)) {
        is_master_ = TRUE;
      } else {
        is_master_ = FALSE;
      }

      return 0;
    }

    BOOL ElectionSingleMaster::IsSlaveParentChild() {
      const string child_node = slave_parent_node_id_ + "/" + node_id_;
      return IsNodeExists(child_node);
    }

    INT32 ElectionSingleMaster::ReadSlaveParentChildren() {
      struct String_vector strings;
      memset(&strings, 0x00, sizeof(strings));
      INT32 result = GetChildren(slave_parent_node_id_, 0, &strings);
      if (0 != result) {
        LIB_DIST_LOG_ERROR("ElectionSingleMaster node:" << slave_parent_node_id_ << " exists "
                                                        << " and read children failed result:" << result);
      }

      slave_list_.clear();
      for (INT32 i = 0; i < strings.count; ++i) {
        LIB_DIST_LOG_DEBUG("index:" << i << " node:" << strings.data[i] << " #################################");
        slave_list_.push_back(strings.data[i]);
      }

      if (is_master_ && (find(slave_list_.begin(), slave_list_.end(), node_id_) != slave_list_.end())) {
        LIB_DIST_LOG_WARN("ElectionSingleMaster node:" << node_id_ << " is master and find in slave list ");
        result = ReadMasterNodeData();
        if (0 != result) {
          LIB_DIST_LOG_ERROR("ElectionSingleMaster read master node:" << master_node_id_ << " failed result:" << result);

          return result;
        }
      }

      if (!is_master_ && (find(slave_list_.begin(), slave_list_.end(), node_id_) == slave_list_.end())) {
        LIB_DIST_LOG_DEBUG("ElectionSingleMaster node:" << node_id_ << " is slave and not found in slave list ........");
        result = SlaveNodeRegister();
        if (0 != result) {
          LIB_DIST_LOG_ERROR("ElectionSingleMaster register slave node:" << node_id_ << " failed result:" << result);

          return result;
        }
      }

      return 0;
    }

    INT32 ElectionSingleMaster::Working(VOID * parameter) {
      INT64 millisecond = reinterpret_cast<INT64>(parameter);
      LIB_DIST_LOG_DEBUG("ElectionSingleMaster node:" << node_id_ << " register master node delay:" << millisecond);
      ::usleep(millisecond*1000);

      if (IsNodeExists(master_node_id_)) {
        LIB_DIST_LOG_DEBUG("ElectionSingleMaster node:" << node_id_ << " do not need register master and "
                                                        << " master is exists.........");
        return 0;
      }

      INT32 result = MasterNodeRegister();
      if (0 != result) {
        LIB_DIST_LOG_ERROR("ElectionSingleMaster register master node:" << master_node_id_
                                                                        << " delay failed result:" << result);

        return result;
      }

      LIB_DIST_LOG_DEBUG("ElectionSingleMaster register master node:" << master_node_id_
                                                                      << " node id:"
                                                                      << node_id_ << " delay .....");

      return 0;
    }

    INT32 ElectionSingleMaster::RegisterMasterDelay(INT32 millisecond) {
      INT32 result = Running(reinterpret_cast<VOID *>(millisecond), 32*1024);
      if (0 != result) {
        LIB_DIST_LOG_ERROR("ElectionSingleMaster register master node delay:" << millisecond
                                                                              << " failed result:" << result);

        return result;
      }

      return 0;
    }

    VOID ElectionSingleMaster::Dump() {
      LIB_DIST_LOG_DEBUG("**************************************************************************************");
      LIB_DIST_LOG_DEBUG("hosts:" << hosts_ << " timeout:" << timeout_ << " node:" << node_id_);
      LIB_DIST_LOG_DEBUG("is master:" << is_master_ << " master node id:" << master_id_);
      INT32 slave_size = slave_list_.size();
      for (INT32 i = 0; i < slave_size; ++i) {
        LIB_DIST_LOG_DEBUG("index:" << i << " slave node id:" << slave_list_[i]);
      }
      LIB_DIST_LOG_DEBUG("**************************************************************************************");
    }

    VOID ElectionSingleMaster::MasterRegCompletion(INT32 rc) {
      LIB_DIST_LOG_DEBUG("ElectionSingleMaster MasterRegCompletion rc:" << rc);
      if (ZOK == rc) {
        is_master_ = TRUE;
        master_id_ = node_id_;

        if (!IsSlaveParentChild()) {
          return;
        }

        const string child_node = slave_parent_node_id_ + "/" + node_id_;
        INT32 result = DeleteNode(child_node, -1);
        if (0 != result) {
          LIB_DIST_LOG_ERROR("ElectionSingleMaster delete node:" << child_node << " failed result:" << result);
        } else {
          LIB_DIST_LOG_DEBUG("ElectionSingleMaster MasterRegCompletion delete slave node:" << child_node << " success");
        }

        return;
      }

      INT32 result = ReadMasterNodeData();
      if (0 != result) {
        LIB_DIST_LOG_ERROR("ElectionSingleMaster read master node:" << master_node_id_ << " failed result:"
                                                                    << result << " .....");
        return;
      }

      if (is_master_) {
        LIB_DIST_LOG_DEBUG("ElectionSingleMaster MasterRegCompletion node:" << node_id_ << " is master ........");
        return ;
      }

      const string child_node = slave_parent_node_id_ + "/" + node_id_;
      if (IsNodeExists(child_node)) {
        LIB_DIST_LOG_DEBUG("ElectionSingleMaster MasterRegCompletion node:" << node_id_
                                                                            << " is slave do not need register ..");
        return;
      }

      result = SlaveNodeRegister();
      if (0 != result) {
        LIB_DIST_LOG_ERROR("ElectionSingleMaster register slave node:" << node_id_ << " failed result:" << result);
      }

      LIB_DIST_LOG_DEBUG("ElectionSingleMaster register node:" << node_id_ << " being slave .....");
    }

    VOID ElectionSingleMaster::SlaveParentRegCompletion(INT32 rc) {
      LIB_DIST_LOG_DEBUG("ElectionSingleMaster SlaveParentRegCompletion rc:" << rc);
      if (is_master_) {
        LIB_DIST_LOG_DEBUG("ElectionSingleMaster node:" << node_id_ << " is master ....");

        return;
      }
      INT32 result = SlaveNodeRegister();
      if (0 != result) {
        LIB_DIST_LOG_ERROR("ElectionSingleMaster register slave node:" << node_id_ << " failed result:" << result);
      }

      LIB_DIST_LOG_DEBUG("ElectionSingleMaster register node:" << node_id_ << " being slave .....");
    }

    VOID ElectionSingleMaster::SlaveNodeRegCompletion(INT32 rc) {
      LIB_DIST_LOG_DEBUG("ElectionSingleMaster SlaveNodeRegCompletion rc:" << rc);
      if (ZNODEEXISTS == rc) {
        LIB_DIST_LOG_WARN("ElectionSingleMaster register node:" << node_id_ << " is exists .....");
      } else if (ZOK == rc) {
        LIB_DIST_LOG_DEBUG("ElectionSingleMaster register node:" << node_id_ << " success .....");
      }

      INT32 result = ReadSlaveParentChildren();
      if (0 != result) {
        LIB_DIST_LOG_ERROR("ElectionSingleMaster read slave parent node:" << slave_parent_node_id_
                                                                          << " children failed result:" << result);
      }
      LIB_DIST_LOG_DEBUG("ElectionSingleMaster slave node:" << slave_parent_node_id_ << " register success ........");
    }

    VOID ElectionSingleMaster::RegCompletion(INT32 rc, OPERATION op) {
      if (REGISTER_MASTER == op) {
        MasterRegCompletion(rc);
      } else if (REGISTER_SLAVE_PARENT == op) {
        SlaveParentRegCompletion(rc);
      } else if (REGISTER_SLAVE == op) {
        SlaveNodeRegCompletion(rc);
      } else {
        LIB_DIST_LOG_WARN("ElectionSingleMaster register op:" << op << " is undefined .....");
      }
    }

    VOID ElectionSingleMaster::MasterWatcher(INT32 type, INT32 state) {
      LIB_DIST_LOG_DEBUG("ElectionSingleMaster master watcher type:" << type << " state:" << state);
      INT32 result = WatchingNode(master_node_id_, ElectionSingleMaster::Watcher, reinterpret_cast<VOID *>(this));
      if (0 != result) {
        LIB_DIST_LOG_ERROR("ElectionSingleMaster watching master node:" << master_node_id_
                                                                        << " failed result:" << result);
      }

      if (ZOO_CREATED_EVENT == type) {
        LIB_DIST_LOG_DEBUG("ElectionSingleMaster master node:" << master_node_id_ << " has be created .............");
        result = ReadMasterNodeData();
        if (0 != result) {
          LIB_DIST_LOG_ERROR("ElectionSingleMaster read master node:" << master_node_id_ << " failed result:"
                                                                      << result << " .....");
          return;
        }

        if (!is_master_) {
          return;
        }

        if (!IsSlaveParentChild()) {
          return;
        }

        const string child_node = slave_parent_node_id_ + "/" + node_id_;
        result = DeleteNode(child_node, -1);
        if (0 != result) {
          LIB_DIST_LOG_ERROR("ElectionSingleMaster delete node:" << child_node << " failed result:" << result);
        }
      } else if (ZOO_DELETED_EVENT == type) {
        LIB_DIST_LOG_DEBUG("ElectionSingleMaster event is master node:" << master_node_id_ << " has be deleted");
        if (is_master_) {
          result = MasterNodeRegister();
          if (0 != result) {
            LIB_DIST_LOG_ERROR("ElectionSingleMaster register master node:" << master_node_id_
                                                                            << " failed result:" << result);
          }
          LIB_DIST_LOG_DEBUG("ElectionSingleMaster node:" << master_node_id_ << " register master ................");

          return;
        }

        result = RegisterMasterDelay(1000);
        if (0 != result) {
          LIB_DIST_LOG_ERROR("ElectionSingleMaster register master node:" << master_node_id_
                                                                          << " delay failed result:" << result);
        }
      } else if (ZOO_CHANGED_EVENT == type) {
        LIB_DIST_LOG_DEBUG("ElectionSingleMaster master node:" << master_node_id_ << " has be changed .....");
        result = ReadMasterNodeData();
        if (0 != result) {
          LIB_DIST_LOG_DEBUG("ElectionSingleMaster read master node:" << master_node_id_ << " failed result:"
                                                                      << result << " .....");
          return;
        }
        if (!is_master_) {
          return;
        }

        if (!IsSlaveParentChild()) {
          return;
        }

        const string child_node = slave_parent_node_id_ + "/" + node_id_;
        result = DeleteNode(child_node, -1);
        if (0 != result) {
          LIB_DIST_LOG_ERROR("ElectionSingleMaster delete node:" << child_node << " failed result:" << result);
        }
      } else if (ZOO_CHILD_EVENT == type) {
        LIB_DIST_LOG_WARN("ElectionSingleMaster master node:" << master_node_id_ << " children has be changed .");
      } else if (ZOO_SESSION_EVENT == type) {
        LIB_DIST_LOG_WARN("ElectionSingleMaster master node:" << master_node_id_ << " session has lost .....");
        zoo_.Destroy();
        result = Initial();
        if (0 != result) {
          LIB_DIST_LOG_ERROR("ElectionSingleMaster initial failed result:" << result);
        }
        LIB_DIST_LOG_DEBUG("ElectionSingleMaster initial success ......................");
      } else if (ZOO_NOTWATCHING_EVENT == type) {
        LIB_DIST_LOG_WARN("ElectionSingleMaster master node:" << master_node_id_ << " watcher is removed ....");
      } else {
        LIB_DIST_LOG_ERROR("ElectionSingleMaster master node:" << master_node_id_ << " type:" << type
                                                               << " state:" << state);
      }
    }

    VOID ElectionSingleMaster::SlaveParentWatcher(INT32 type, INT32 state) {
      LIB_DIST_LOG_DEBUG("ElectionSingleMaster slave parent watcher type:" << type << " state:" << state);
      INT32 result = WatchingNode(slave_parent_node_id_, ElectionSingleMaster::Watcher, reinterpret_cast<VOID *>(this));
      if (0 != result) {
        LIB_DIST_LOG_ERROR("ElectionSingleMaster watching slave parent node:" << slave_parent_node_id_
                                                                              << " failed result:" << result);
      }

      result = WatchingChildNodeList(slave_parent_node_id_, ElectionSingleMaster::Watcher,
                                     reinterpret_cast<VOID *>(this));
      if (0 != result) {
        LIB_DIST_LOG_ERROR("ElectionSingleMaster watching slave parent node:" << slave_parent_node_id_
                                                                              << " children failed result:"
                                                                              << result);
      }

      if (ZOO_CREATED_EVENT == type) {
        LIB_DIST_LOG_DEBUG("ElectionSingleMaster slave parent node:" << slave_parent_node_id_ << " has be created ....");
        if (is_master_) {
          LIB_DIST_LOG_DEBUG("ElectionSingleMaster node:" << node_id_ << " is master ....");
          return;
        }

        result = SlaveNodeRegister();
        if (0 != result) {
          LIB_DIST_LOG_ERROR("ElectionSingleMaster slave node:" << node_id_ << " register failed "
                                                                << " result:" << result);
        }
        LIB_DIST_LOG_DEBUG("ElectionSingleMaster slave node:" << node_id_ << " register ........");
      } else if (ZOO_DELETED_EVENT == type) {
        LIB_DIST_LOG_WARN("ElectionSingleMaster slave parent node has be deleted .......................");
        result = SlaveParentNodeRegister();
        if (0 != result) {
          LIB_DIST_LOG_ERROR("ElectionSingleMaster register slave parent node:" << slave_parent_node_id_
                                                                                << " failed result:" << result);
        }
        LIB_DIST_LOG_DEBUG("ElectionSingleMaster slave parent node:" << slave_parent_node_id_ << " register ........");
      } else if (ZOO_CHANGED_EVENT == type) {
        LIB_DIST_LOG_DEBUG("ElectionSingleMaster slave parent node:" << slave_parent_node_id_ << " has be changed .....");
        result = ReadSlaveParentChildren();
        if (0 != result) {
          LIB_DIST_LOG_ERROR("ElectionSingleMaster read slave list node:" << slave_parent_node_id_
                                                                          << " failed result:" << result);
        }
        LIB_DIST_LOG_DEBUG("ElectionSingleMaster read slave parent node:" << slave_parent_node_id_
                                                                          << " children list size:"
                                                                          << slave_list_.size());
      } else if (ZOO_CHILD_EVENT == type) {
        LIB_DIST_LOG_WARN("ElectionSingleMaster slave parent node:" << slave_parent_node_id_ << " child has be changed ");
        result = ReadSlaveParentChildren();
        if (0 != result) {
          LIB_DIST_LOG_ERROR("ElectionSingleMaster read slave list node:" << slave_parent_node_id_
                                                                          << " failed result:" << result);
        }
        LIB_DIST_LOG_DEBUG("ElectionSingleMaster read slave parent node:" << slave_parent_node_id_
                                                                          << " children list size:"
                                                                          << slave_list_.size());
      } else if (ZOO_SESSION_EVENT == type) {
        LIB_DIST_LOG_WARN("ElectionSingleMaster slave parent node:" << slave_parent_node_id_ << " session has lost ..");
        zoo_.Destroy();
        result = Initial();
        if (0 != result) {
          LIB_DIST_LOG_ERROR("ElectionSingleMaster initial failed result:" << result);
        }
        LIB_DIST_LOG_DEBUG("ElectionSingleMaster initial success ......................");
      } else if (ZOO_NOTWATCHING_EVENT == type) {
        LIB_DIST_LOG_WARN("ElectionSingleMaster slave parent node:" << slave_parent_node_id_ << " watcher is removed .");
      } else {
        LIB_DIST_LOG_ERROR("ElectionSingleMaster slave parent node:" << slave_parent_node_id_ << " type:" << type
                                                                     << " state:" << state);
      }
    }

    VOID ElectionSingleMaster::Watcher(INT32 type, INT32 state, const CHAR * node) {
      INT32 node_len = ::strlen(node);
      INT32 master_node_id_len = master_node_id_.length();
      INT32 slave_parent_node_len = slave_parent_node_id_.length();
      if (node_len == master_node_id_len && 0 == ::strncasecmp(node, master_node_id_.c_str(), node_len)) {
        MasterWatcher(type, state);
      } else if (node_len == slave_parent_node_len &&
                 0 == ::strncasecmp(node, slave_parent_node_id_.c_str(), node_len)) {
        SlaveParentWatcher(type, state);
      } else {
        LIB_DIST_LOG_ERROR("ElectionSingleMaster node watching error node:" << node << " is invalid ....");
      }
    }

    VOID ElectionSingleMaster::Watcher(zhandle_t* zh, INT32 type, INT32 state, const CHAR * node, VOID * watcher_ctx) {
      LIB_DIST_LOG_DEBUG("ElectionSingleMaster Watcher zookeeper handler:" << zh << " type:" << type
                                                                           << " state:" << state
                                                                           << " node:" << node);
      ElectionSingleMaster * election = reinterpret_cast<ElectionSingleMaster *>(const_cast<VOID *>(watcher_ctx));
      election->Watcher(type, state, node);
      election->Dump();
    }

    VOID ElectionSingleMaster::RegisterCompletion(INT32 rc, const CHAR * node, const VOID * data) {
      LIB_DIST_LOG_DEBUG("ElectionSingleMaster RegisterCompletion rc:" << rc << " node:" << node);
      RegisterCompletionData * reg_data = reinterpret_cast<RegisterCompletionData *>(const_cast<VOID *>(data));
      if (OP_UNKNOWN == reg_data->op_ || NULL == reg_data->election_) {
        LIB_DIST_LOG_ERROR("ElectionSingleMaster RegisterCompletion register parameters is invalid ........");
        return;
      }

      ElectionSingleMaster * election = reinterpret_cast<ElectionSingleMaster *>(const_cast<VOID *>(reg_data->election_));

      if (ZNOAUTH == rc) {
        LIB_DIST_LOG_ERROR("ElectionSingleMaster RegisterCompletion rc:" << rc << " node:" << node << " auth failed");
      } else if (ZOK != rc && ZNODEEXISTS != rc) {
        LIB_DIST_LOG_ERROR("ElectionSingleMaster RegisterCompletion rc:" << rc << " node:" << node);
      } else {
        election->RegCompletion(rc, reg_data->op_);
      }

      election->Dump();

      delete reg_data;
    }
  }  // namespace dist
}  // namespace lib