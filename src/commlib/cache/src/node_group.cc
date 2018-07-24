// Copyright (c) 2013 zhou chang yue. All rights reserved.

#include <string.h>
#include "commlib/cache/inc/node_group.h"
#include "commlib/cache/inc/log.h"
#include "commlib/magic/inc/str.h"
#include "commlib/magic/inc/mutex.h"
#include "commlib/magic/inc/scopeLock.h"
#include "commlib/magic/inc/timeFormat.h"
#include "commlib/magic/inc/md5.h"

namespace lib {
  namespace cache {
    using lib::magic::TimeFormat;
    using lib::magic::Mutex;
    using lib::magic::ScopeLock;
    using lib::magic::String;
    using lib::magic::MD5;

    INT32 NodeGroup::LoadNode(ChunkInfo * chunk_info) {
      if (NULL == node_mem_info_ || NULL == chunk_info) {
        LIB_CACHE_LOG_ERROR("NodeGroup load node failed node memory object is empty");

        return Err::kERR_NODE_GROUP_INFO_OBJECT_EMPTY;
      }

      pt_malloc_ = new PTMalloc(chunk_info);
      if (NULL == pt_malloc_) {
        LIB_CACHE_LOG_ERROR("NodeGroup load node failed pt malloc object is empty");

        return Err::kERR_NODE_GROUP_INFO_OBJECT_EMPTY;
      }

      if (NodeMemInfo::NODE_GROUP_USABLE != node_mem_info_->flag_) {
        LIB_CACHE_LOG_DEBUG("NodeGroup is not usable must initial node group information ................. ");
        INT32 result = InitialNodeGroupInfo();
        if (0 != result) {
          return result;
        }
      }

      LIB_CACHE_LOG_DEBUG("NodeGroup initial ptmalloc and node group success .............................");

      INT32 hash_size = (node_mem_info_->total_group_count_*NodeMemInfo::NODES_PER_GROUP)/4 - 1;
      if (keys_hash_ < hash_size) {
        keys_hash_ = hash_size;
      }
      hash_bucket_ = reinterpret_cast<HashNode **>(::malloc(keys_hash_*sizeof(HashNode *)));
      if (NULL == hash_bucket_) {
        LIB_CACHE_LOG_DEBUG("NodeGroup load node failed hash bucket object is empty keys hash:" << keys_hash_);

        return Err::kERR_NODE_GROUP_INFO_OBJECT_EMPTY;
      }
      for (INT32 i = 0; i < keys_hash_; ++i) {
        hash_bucket_[i] = NULL;
      }

      LIB_CACHE_LOG_DEBUG("NodeGroup initial hash bucket success keys_hash:" << keys_hash_
                                                                             << " hash size:" << hash_size
                                                                             << " .................");

      INT32 result = LoadNodeFromUsedList();
      if (0 != result) {
        return result;
      }

      LIB_CACHE_LOG_DEBUG("NodeGroup load from used list success .............................");

      DumpHashBucket();

      DumpLRULink();

      return 0;
    }

    INT32 NodeGroup::InitialNodeGroupInfo() {
      if (NULL == node_mem_info_) {
        LIB_CACHE_LOG_ERROR("NodeGroup initial group info failed node memory object is empty");

        return Err::kERR_NODE_GROUP_INFO_OBJECT_EMPTY;
      }

      memset(node_mem_info_, 0x00, sizeof(NodeMemInfo));
      if (0 != InitialLock(&node_mem_info_->mutex_)) {
        LIB_CACHE_LOG_ERROR("NodeGroup initial node memory info lock failed");
        return Err::kERR_NODE_GROUP_NODE_INITIAL_LOCK_FAILED;
      }

      Mutex mutex(&node_mem_info_->mutex_);
      if (0 == mutex.TryLock()) {
        node_mem_info_->used_group_link_head_ = NodeMemInfo::LINK_TAIL;
        node_mem_info_->used_node_link_head_ = NodeMemInfo::LINK_TAIL;
        node_mem_info_->free_group_link_head_ = NodeMemInfo::LINK_TAIL;
        node_mem_info_->free_node_link_head_ = NodeMemInfo::LINK_TAIL;

        node_mem_info_->flag_ = NodeMemInfo::NODE_GROUP_USABLE;
        mutex.UnLock();
      }

      LIB_CACHE_LOG_DEBUG("NodeGroup initial group info success ..........................");

      return AllocateNodeGroup();
    }

    INT32 NodeGroup::LoadNodeFromUsedList() {
      if (NULL == hash_bucket_) {
        LIB_CACHE_LOG_ERROR("NodeGroup load node failed hash bucket object is empty");

        return Err::kERR_NODE_GROUP_INFO_OBJECT_EMPTY;
      }

      if (NodeMemInfo::LINK_TAIL == node_mem_info_->used_group_link_head_ ||
              NodeMemInfo::LINK_TAIL == node_mem_info_->used_node_link_head_) {
        LIB_CACHE_LOG_DEBUG("NodeGroup load node to hash bucket has no node to loading ............");
        return 0;
      }

      INT32 group_index = node_mem_info_->used_group_link_head_;
      INT32 node_index = node_mem_info_->used_node_link_head_;

      while ((NodeMemInfo::LINK_TAIL < group_index && NodeMemInfo::MAX_GROUP_COUNT > group_index) &&
              (NodeMemInfo::LINK_TAIL < node_index && NodeMemInfo::NODES_PER_GROUP > node_index)) {
        NodeMemInfo::Node * node = Offset2Node(group_index, node_index);
        if (NULL == node) {
          LIB_CACHE_LOG_ERROR("NodeGroup offset to node failed group index:" << group_index
                                                                             << " node index:" << node_index);
          return Err::kERR_NODE_GROUP_INFO_OBJECT_EMPTY;
        }

        LIB_CACHE_LOG_DEBUG("NodeGroup load node key:" << node->key_ << " last access timestamp:"
                                                       << node->last_access_timestamp_);

        INT32 result = AddNode2Hash(node);
        if (0 != result) {
          return result;
        }

        LIB_CACHE_LOG_DEBUG("NodeGroup load node key:" << node->key_ << " last access timestamp:"
                                                       << node->last_access_timestamp_
                                                       << " add to hash success ....");

        result = AddNode2LRU(GetHashNodeFromHash(node->key_));
        if (0 != result) {
          return result;
        }

        LIB_CACHE_LOG_DEBUG("NodeGroup load node key:" << node->key_ << " last access timestamp:"
                                                       << node->last_access_timestamp_
                                                       << " add to lru success ....");

        group_index = node->next_used_group_index_;
        node_index = node->next_used_node_index_;
      }

      return 0;
    }

    INT32 NodeGroup::AddNode2LRU(HashNode * hash_node) {
      if (NULL == hash_node || NULL == hash_node->node_) {
        LIB_CACHE_LOG_DEBUG("NodeGroup add hash node to lru failed node object is empty");

        return Err::kERR_NODE_GROUP_INFO_OBJECT_EMPTY;
      }

      LIB_CACHE_LOG_DEBUG("NodeGroup add node to lru key:" << hash_node->node_->key_ << " last access timestamp:"
                                                           << hash_node->node_->last_access_timestamp_
                                                           << " ............");

      ScopeLock<Mutex> scope(&mutex_);
      if (NULL == lru_node_head_) {
        LIB_CACHE_LOG_DEBUG("NodeGroup add node to lru key:" << hash_node->node_->key_ << " last access timestamp:"
                                                             << hash_node->node_->last_access_timestamp_
                                                             << " link head is empty .............. ");
        lru_node_head_ = hash_node;
        hash_node->lru_next_ = lru_node_head_;
        hash_node->lru_prev_ = lru_node_head_;

        LIB_CACHE_LOG_DEBUG("NodeGroup add node to lru link head *********************************************");
        DumpNode(lru_node_head_->node_);
        LIB_CACHE_LOG_DEBUG("NodeGroup add node to lru link head *********************************************");

        return 0;
      }

      HashNode * p = lru_node_head_;
      while (lru_node_head_ != p->lru_next_) {
        if (p->node_->last_access_timestamp_ <= hash_node->node_->last_access_timestamp_) {
          p = p->lru_prev_;

          break;
        }
        p = p->lru_next_;
      }

      hash_node->lru_next_ = p->lru_next_;
      hash_node->lru_prev_ = p;
      p->lru_next_->lru_prev_ = hash_node;
      p->lru_next_ = hash_node;

      if (hash_node->node_->last_access_timestamp_ >= lru_node_head_->node_->last_access_timestamp_) {
        lru_node_head_ = hash_node;
      }

      LIB_CACHE_LOG_DEBUG("NodeGroup add node to lru link head *********************************************");
      DumpNode(lru_node_head_->node_);
      LIB_CACHE_LOG_DEBUG("NodeGroup add node to lru link head *********************************************");

      return 0;
    }

    INT32 NodeGroup::DelNodeFromLRU(HashNode * hash_node) {
      if (NULL == hash_node || NULL == hash_node->node_) {
        LIB_CACHE_LOG_DEBUG("NodeGroup delete hash node from lru link failed node object is empty");

        return Err::kERR_NODE_GROUP_INFO_OBJECT_EMPTY;
      }

      ScopeLock<Mutex> scope(&mutex_);

      if (hash_node == lru_node_head_) {
        lru_node_head_ = lru_node_head_->lru_next_;
      }

      if (NULL != hash_node->lru_prev_) {
        hash_node->lru_prev_->lru_next_ = hash_node->lru_next_;
      }

      if (NULL != hash_node->lru_next_) {
        hash_node->lru_next_->lru_prev_ = hash_node->lru_prev_;
      }

      hash_node->lru_prev_ = NULL;
      hash_node->lru_next_ = NULL;

      return 0;
    }

    INT32 NodeGroup::UpdateNode2LRU(HashNode * hash_node) {
      if (NULL == hash_node || NULL == hash_node->node_) {
        LIB_CACHE_LOG_DEBUG("NodeGroup update hash node from lru link failed node object is empty");

        return Err::kERR_NODE_GROUP_INFO_OBJECT_EMPTY;
      }

      if (hash_node == lru_node_head_) {
        hash_node->node_->last_access_timestamp_ = TimeFormat::GetCurTimestampLong();
        LIB_CACHE_LOG_DEBUG("NodeGroup update hash node is the head of link ...............");
        return 0;
      }

      INT32 result = DelNodeFromLRU(hash_node);
      if (0 != result) {
        return result;
      }

      return AddNode2LRU(hash_node);
    }

    BOOL NodeGroup::IsInHashBucket(const string & key) {
      if (NULL == hash_bucket_) {
        LIB_CACHE_LOG_DEBUG("NodeGroup is in hash bucket hash_bucket_ object is empty");
        return FALSE;
      }

      UINT32 hash_id = DefHash(key);
      HashNode * p = hash_bucket_[hash_id/keys_hash_];
      while (NULL != p) {
        if ((NULL != p->node_) && (0 == ::strncasecmp(key.c_str(), p->node_->key_, key.length()))) {
          return TRUE;
        }
        p = p->hash_next_;
      }

      return FALSE;
    }

    INT32 NodeGroup::DelNodeFromBucket(HashNode * hash_node) {
      if (NULL == hash_node || NULL == hash_bucket_ || NULL == hash_node->node_) {
        LIB_CACHE_LOG_DEBUG("NodeGroup delete node from hash bucket failed node or hash_bucket_ object is empty");

        return 0;
      }

      if (!IsInHashBucket(hash_node->node_->key_)) {
        LIB_CACHE_LOG_DEBUG("NodeGroup delete node from hash bucket failed key:" << hash_node->node_->key_
                                                                                 << " is not in bucket");
        return 0;
      }

      UINT32 hash_id = DefHash(hash_node->node_->key_);

      if (hash_node == hash_bucket_[hash_id/keys_hash_]) {
        hash_bucket_[hash_id/keys_hash_] = hash_node->hash_next_;

        return 0;
      }

      HashNode * p = hash_bucket_[hash_id/keys_hash_];
      while ((NULL != p) && (p->hash_next_ != hash_node)) {
        p = p->hash_next_;
      }

      if (NULL != p) {
        p->hash_next_ = hash_node->hash_next_;
      }

      return 0;
    }

    INT32 NodeGroup::AddNode2Hash(HashNode * hash_node) {
      if (NULL == hash_node || NULL == hash_node->node_ || NULL == hash_bucket_) {
        LIB_CACHE_LOG_DEBUG("NodeGroup add node to hash bucket failed node or hash_bucket_ object is empty");

        return Err::kERR_NODE_GROUP_INFO_OBJECT_EMPTY;
      }

      if (IsInHashBucket(hash_node->node_->key_)) {
        LIB_CACHE_LOG_DEBUG("NodeGroup add node to hash bucket node key:" << hash_node->node_->key_ << " is exists");
        return 0;
      }

      UINT32 hash_id = DefHash(hash_node->node_->key_);

      if (NULL != hash_bucket_[hash_id/keys_hash_]) {
        hash_node->hash_next_ = hash_bucket_[hash_id/keys_hash_];
      }

      hash_bucket_[hash_id/keys_hash_] = hash_node;

      LIB_CACHE_LOG_DEBUG("NodeGroup add node to hash bucket success node key:" << hash_node->node_->key_);

      return 0;
    }

    INT32 NodeGroup::AddNode2Hash(NodeMemInfo::Node * node) {
      if (NULL == node || NULL == hash_bucket_) {
        LIB_CACHE_LOG_DEBUG("NodeGroup add node to hash bucket failed node or hash_bucket_ object is empty");

        return Err::kERR_NODE_GROUP_INFO_OBJECT_EMPTY;
      }

      if (IsInHashBucket(node->key_)) {
        LIB_CACHE_LOG_WARN("NodeGroup add node to hash bucket failed hash node key:" << node->key_ << " exists");
        return Err::kERR_NODE_GROUP_NODE_KEY_EXISTS;
      }

      HashNode * node_hash = new HashNode();
      if (NULL == node_hash) {
        LIB_CACHE_LOG_DEBUG("NodeGroup add node to hash bucket failed hash node object is empty");
        return Err::kERR_NODE_GROUP_INFO_OBJECT_EMPTY;
      }

      node_hash->node_ = node;

      return AddNode2Hash(node_hash);
    }

    HashNode * NodeGroup::GetHashNodeFromHash(const string & key) {
      UINT32 hash_id = DefHash(key);
      LIB_CACHE_LOG_DEBUG("NodeGroup get key:" << key << " hash id:" << hash_id
                                               << " keys_hash:" << keys_hash_
                                               << " bucket:" << (hash_id/keys_hash_)
                                               << " from hash bucket .......................");
      HashNode * node_hash = hash_bucket_[hash_id/keys_hash_];
      while (NULL != node_hash) {
        if (NULL != node_hash->node_ && (::strlen(node_hash->node_->key_) == key.length()) &&
                (0 == ::strncasecmp(node_hash->node_->key_, key.c_str(),
                                    node_hash->node_->key_length_))) {
          LIB_CACHE_LOG_DEBUG("NodeGroup get node from hash bucket success key:" << key);

          return node_hash;
        }
        node_hash = node_hash->hash_next_;
      }

      return NULL;
    }

    NodeMemInfo::Node * NodeGroup::Offset2Node(INT32 group_offset_index, INT32 node_index) {
      if (0 > group_offset_index || NodeMemInfo::MAX_GROUP_COUNT <= group_offset_index) {
        LIB_CACHE_LOG_WARN("NodeGroup offset to point group index:" << group_offset_index << " is invalid");

        return NULL;
      }

      return Offset2Node(node_mem_info_->node_group_offset_[group_offset_index], node_index);
    }

    NodeGroup::NodeGroupMem * NodeGroup::Offset2Group(INT64 group_offset) {
      if (0 >= group_offset) {
        LIB_CACHE_LOG_WARN("NodeGroup offset to point group offset:" << group_offset);

        return NULL;
      }

      return reinterpret_cast<NodeGroupMem *>(reinterpret_cast<CHAR *>(node_mem_info_) + group_offset);
    }

    NodeMemInfo::Node * NodeGroup::Offset2Node(INT64 group_offset, INT32 node_index) {
      NodeGroupMem * group = reinterpret_cast<NodeGroupMem *>(reinterpret_cast<CHAR *>(node_mem_info_) + group_offset);
      if (0 > node_index || NodeMemInfo::NODES_PER_GROUP <= node_index) {
        LIB_CACHE_LOG_WARN("NodeGroup offset to point node index:" << node_index << " is invalid");

        return NULL;
      }

      return &group->node_list_[node_index];
    }

    CHAR * NodeGroup::Offset2DataPoint(INT64 offset) {
      return reinterpret_cast<CHAR *>(reinterpret_cast<CHAR *>(node_mem_info_) + offset);
    }

    INT64 NodeGroup::CharPoint2Offset(CHAR * address) {
      return reinterpret_cast<INT64>(address - reinterpret_cast<CHAR *>(node_mem_info_));
    }

    UINT64 NodeGroup::DefHash(const string & id) {
      if (0 >= String::Trim(id).length()) {
        return 0;
      }

      LONG hash = 0;
      MD5 md5(id);
      const string digest = md5.hex_digest();

      for(INT32 i = 0; i < 4; ++i) {
        hash += ((LONG)(digest.c_str()[i*4 + 3]&0xFF) << 24)
                | ((LONG)(digest.c_str()[i*4 + 2]&0xFF) << 16)
                | ((LONG)(digest.c_str()[i*4 + 1]&0xFF) <<  8)
                | ((LONG)(digest.c_str()[i*4 + 0]&0xFF));
      }

      return hash;
    }

    INT32 NodeGroup::InitialLock(pthread_mutex_t * mutex) {
      if (NULL == mutex) {
        LIB_CACHE_LOG_ERROR("NodeGroup initial mutex failed .................");

        return Err::kERR_NODE_GROUP_INFO_OBJECT_EMPTY;
      }
      pthread_mutexattr_t attr;
      ::pthread_mutexattr_init(&attr);
      if (0 != ::pthread_mutex_init(mutex, &attr)) {
        return -1;
      }
      ::pthread_mutexattr_destroy(&attr);

      return 0;
    }

    INT32 NodeGroup::AllocateNodeGroup() {
      if (NULL == pt_malloc_) {
        LIB_CACHE_LOG_ERROR("NodeGroup allocate node group failed ptmalloc object is empty");

        return Err::kERR_NODE_GROUP_INFO_OBJECT_EMPTY;
      }

      if (node_mem_info_->total_group_count_ >= NodeMemInfo::MAX_GROUP_COUNT) {
        LIB_CACHE_LOG_ERROR("NodeGroup allocate node group failed total group count:" << node_mem_info_->total_group_count_);
        return Err::kERR_NODE_GROUP_COUNT_IS_TOO_MANY;
      }

      NodeGroupMem * node_group = reinterpret_cast<NodeGroupMem *>(pt_malloc_->malloc(sizeof(NodeGroupMem)));
      if (NULL == node_group) {
        LIB_CACHE_LOG_WARN("NodeGroup allocate node group failed maybe has no chunk plz check ptmalloc log .........");

        INT32 result = FreeExpiredNode(sizeof(NodeGroupMem));
        if (0 != result) {
          LIB_CACHE_LOG_ERROR("NodeGroup free expired node failed result:" << result);

          return result;
        }

        node_group = reinterpret_cast<NodeGroupMem *>(pt_malloc_->malloc(sizeof(NodeGroupMem)));
        if (NULL == node_group) {
          LIB_CACHE_LOG_ERROR("NodeGroup allocate node group failed maybe has no chunk plz check ptmalloc log .........");
          return Err::kERR_NODE_GROUP_INFO_OBJECT_EMPTY;
        }

        result = Running(NULL, 32*1024);
        if (0 != result) {
          LIB_CACHE_LOG_ERROR("NodeGroup start free expired node job failed result:" << result);

          return result;
        }
      }

      Mutex mutex(&node_mem_info_->mutex_);
      ScopeLock<Mutex> scope(&mutex);
      node_mem_info_->node_group_offset_[node_mem_info_->total_group_count_++] = CharPoint2Offset(reinterpret_cast<CHAR *>(node_group));

      LIB_CACHE_LOG_DEBUG("NodeGroup allocate group info success group count:" << node_mem_info_->total_group_count_
                                                                               << " ..........................");

      return InitialNodeGroup(node_group);
    }

    INT32 NodeGroup::InitialNodeGroup(NodeGroupMem * node_group) {
      if (NULL == node_group || NULL == node_mem_info_) {
        LIB_CACHE_LOG_ERROR("NodeGroup initial node group failed .................");

        return Err::kERR_NODE_GROUP_INFO_OBJECT_EMPTY;
      }

      node_group->flag_ = NodeMemInfo::NODE_GROUP_NOT_INITIAL;
      node_group->next_group_offset_ = 0;
      memset(node_group->node_list_, 0x00, sizeof(node_group->node_list_));
      for (INT32 i = (NodeMemInfo::NODES_PER_GROUP - 1); i >= 0; --i) {
        node_group->node_list_[i].prev_used_group_index_ = NodeMemInfo::LINK_TAIL;
        node_group->node_list_[i].prev_used_node_index_ = NodeMemInfo::LINK_TAIL;
        node_group->node_list_[i].next_used_group_index_ = NodeMemInfo::LINK_TAIL;
        node_group->node_list_[i].next_used_node_index_ = NodeMemInfo::LINK_TAIL;
        node_group->node_list_[i].next_free_group_index_ = node_mem_info_->free_group_link_head_;
        node_group->node_list_[i].next_free_node_index_ = node_mem_info_->free_node_link_head_;

        if (0 != InitialLock(&node_group->node_list_[i].mutex_)) {
          return Err::kERR_NODE_GROUP_NODE_INITIAL_LOCK_FAILED;
        }

        node_mem_info_->free_group_link_head_ = (node_mem_info_->total_group_count_ - 1);
        node_mem_info_->free_node_link_head_ = i;
      }

      if (0 != InitialLock(&node_group->mutex_)) {
        return Err::kERR_NODE_GROUP_NODE_INITIAL_LOCK_FAILED;
      }
      node_group->flag_ = NodeMemInfo::NODE_GROUP_USABLE;

      LIB_CACHE_LOG_DEBUG("NodeGroup initial group success free group head:" << node_mem_info_->free_group_link_head_
                                                                             << " free node head:"
                                                                             << node_mem_info_->free_node_link_head_);

      return 0;
    }

    NodeMemInfo::Node * NodeGroup::GetFreeNode() {
      if (NodeMemInfo::LINK_TAIL == node_mem_info_->free_group_link_head_ ||
              NodeMemInfo::LINK_TAIL == node_mem_info_->free_node_link_head_) {
        if (0 != AllocateNodeGroup() || NodeMemInfo::LINK_TAIL == node_mem_info_->free_group_link_head_ ||
                NodeMemInfo::LINK_TAIL == node_mem_info_->free_node_link_head_) {
          LIB_CACHE_LOG_ERROR("NodeGroup get free node failed .................");
          return NULL;
        }
      }

      Mutex mutex(&node_mem_info_->mutex_);
      ScopeLock<Mutex> scope(&mutex);
      INT32 group_index = node_mem_info_->free_group_link_head_;
      INT32 node_index = node_mem_info_->free_node_link_head_;

      NodeMemInfo::Node * node = Offset2Node(group_index, node_index);
      node_mem_info_->free_group_link_head_ = node->next_free_group_index_;
      node_mem_info_->free_node_link_head_ = node->next_free_node_index_;

      node->next_used_group_index_ = node_mem_info_->used_group_link_head_;
      node->next_used_node_index_ = node_mem_info_->used_node_link_head_;
      if (NodeMemInfo::LINK_TAIL != node_mem_info_->used_group_link_head_ &&
              NodeMemInfo::LINK_TAIL != node_mem_info_->used_node_link_head_) {
        NodeMemInfo::Node * head_used_node = Offset2Node(node_mem_info_->used_group_link_head_,
                                                   node_mem_info_->used_node_link_head_);

        if (NodeMemInfo::LINK_TAIL != head_used_node->prev_used_group_index_ &&
                NodeMemInfo::LINK_TAIL != head_used_node->prev_used_node_index_) {
          NodeMemInfo::Node * prev_used_node = Offset2Node(head_used_node->prev_used_group_index_,
                                                      head_used_node->prev_used_node_index_);
          prev_used_node->next_used_group_index_ = group_index;
          prev_used_node->next_used_node_index_ = node_index;
        }
        head_used_node->prev_used_group_index_ = group_index;
        head_used_node->prev_used_node_index_ = node_index;
      }

      node_mem_info_->used_group_link_head_ = group_index;
      node_mem_info_->used_node_link_head_ = node_index;
      node->next_free_group_index_ = NodeMemInfo::LINK_TAIL;
      node->next_free_node_index_ = NodeMemInfo::LINK_TAIL;

      return node;
    }

    INT32 NodeGroup::ClearNode(NodeMemInfo::Node * node) {
      if (NULL == node) {
        return 0;
      }

      Mutex mutex(&node->mutex_);
      ScopeLock<Mutex> scope(&mutex);

      memset(node->key_, 0x00, sizeof(node->key_));
      node->key_length_ = 0;
      node->flag_ = 0;
      if (0 < node->data_size_ && 0 < node->data_offset_) {
        if (NULL != pt_malloc_) {
          pt_malloc_->free(reinterpret_cast<VOID *>(Offset2DataPoint(node->data_offset_)));
        }
      }
      node->data_offset_ = 0;
      node->data_size_ = 0;
      node->data_capacity_ = 0;
      node->last_access_timestamp_ = 0;
      node->next_used_group_index_ = NodeMemInfo::LINK_TAIL;
      node->next_used_node_index_ = NodeMemInfo::LINK_TAIL;
      node->prev_used_group_index_ = NodeMemInfo::LINK_TAIL;
      node->prev_used_node_index_ = NodeMemInfo::LINK_TAIL;
      node->next_free_group_index_ = NodeMemInfo::LINK_TAIL;
      node->next_free_node_index_ = NodeMemInfo::LINK_TAIL;

      return 0;
    }

    INT32 NodeGroup::RecycleNode(NodeMemInfo::Node * node) {
      if (NULL == node) {
        return 0;
      }

      INT32 prev_group_index = node->prev_used_group_index_;
      INT32 prev_node_index = node->prev_used_node_index_;
      INT32 next_group_index = node->next_used_group_index_;
      INT32 next_node_index = node->next_used_node_index_;
      INT32 cur_group_index = NodeMemInfo::LINK_TAIL;
      INT32 cur_node_index = NodeMemInfo::LINK_TAIL;

      INT32 result = ClearNode(node);
      if (0 != result) {
        return result;
      }

      if (NodeMemInfo::LINK_TAIL != prev_group_index || NodeMemInfo::LINK_TAIL != prev_node_index) {
        NodeMemInfo::Node *prev_node = Offset2Node(prev_group_index, prev_node_index);
        Mutex prev_mutex(&prev_node->mutex_);
        prev_mutex.Lock();
        cur_group_index = prev_node->next_used_group_index_;
        cur_node_index = prev_node->next_used_node_index_;
        prev_node->next_used_group_index_ = next_group_index;
        prev_node->next_used_node_index_ = next_node_index;
        prev_mutex.UnLock();
      }

      if (NodeMemInfo::LINK_TAIL != next_group_index || NodeMemInfo::LINK_TAIL != next_node_index) {
        NodeMemInfo::Node *next_node = Offset2Node(next_group_index, next_node_index);
        Mutex next_mutex(&next_node->mutex_);
        next_mutex.Lock();
        cur_group_index = next_node->prev_used_group_index_;
        cur_node_index = next_node->prev_used_node_index_;
        next_node->prev_used_group_index_ = prev_group_index;
        next_node->prev_used_node_index_ = prev_node_index;
        next_mutex.UnLock();
      }

      Mutex mutex(&node_mem_info_->mutex_);
      ScopeLock<Mutex> scope(&mutex);
      NodeMemInfo::Node * head_node = Offset2Node(node_mem_info_->used_group_link_head_,
                                                  node_mem_info_->used_node_link_head_);
      if (head_node == node) {
        cur_group_index = node_mem_info_->used_group_link_head_;
        cur_node_index = node_mem_info_->used_node_link_head_;

        node_mem_info_->used_group_link_head_ = next_group_index;
        node_mem_info_->used_node_link_head_ = next_node_index;
      }

      node->next_free_group_index_ = node_mem_info_->free_group_link_head_;
      node->next_free_node_index_ = node_mem_info_->free_node_link_head_;

      node_mem_info_->free_group_link_head_ = cur_group_index;
      node_mem_info_->free_node_link_head_ = cur_node_index;

      return 0;
    }

    INT32 NodeGroup::Set(const string & key) {
      LIB_CACHE_LOG_DEBUG("NodeGroup set key:" << key << " ....................");
      HashNode * hash_node = GetHashNodeFromHash(key);
      if (NULL == hash_node) {
        LIB_CACHE_LOG_DEBUG("NodeGroup set key:" << key << " has not in hash bucket ....................");
        hash_node = new HashNode();
        if (NULL == hash_node) {
          LIB_CACHE_LOG_ERROR("NodeGroup set key: " << key << " failed "
                                                    << " hash node object empty ..");
          return Err::kERR_NODE_GROUP_INFO_OBJECT_EMPTY;
        }

        LIB_CACHE_LOG_DEBUG("NodeGroup set key:" << key << " allocate hash node success ....................");
        if (NULL == (hash_node->node_ = GetFreeNode())) {
          LIB_CACHE_LOG_ERROR("NodeGroup set key: " << key << " failed .................");
          return Err::kERR_NODE_GROUP_HAS_NO_FREE_NODE;
        }
      } else {
        LIB_CACHE_LOG_DEBUG("NodeGroup set key:" << key << " is in hash bucket ....................");
      }

      Mutex mutex(&hash_node->node_->mutex_);
      ScopeLock<Mutex> scope(&mutex);

      NodeMemInfo::Node * node = hash_node->node_;
      node->flag_ = DIRTY;

      if (0 < node->data_capacity_ && 0 < node->data_offset_) {
        LIB_CACHE_LOG_DEBUG("NodeGroup set key:" << key << " size:" << node->data_size_
                                                 << " need free old and allocate new chunk  ....................");
        if (NULL == pt_malloc_) {
          LIB_CACHE_LOG_ERROR("NodeGroup free chunk failed ptmalloc object is empty");

          return Err::kERR_NODE_GROUP_INFO_OBJECT_EMPTY;
        }

        pt_malloc_->free(reinterpret_cast<VOID *>(Offset2DataPoint(node->data_offset_)));
      }

      node->data_capacity_ = 0;
      node->data_offset_ = 0;
      node->data_size_ = 0;
      memcpy(node->key_, key.c_str(), key.length());
      node->key_length_ = key.length();
      node->last_access_timestamp_ = TimeFormat::GetCurTimestampLong();

      atomic_add(1, &(node_mem_info_->total_keys_));

      LIB_CACHE_LOG_DEBUG("NodeGroup set key:" << key << " set data to buffer success  .........");
      DumpNode(node);
      INT32 result = AddNode2Hash(hash_node);
      if (0 != result) {
        return result;
      }

      LIB_CACHE_LOG_DEBUG("NodeGroup set key:" << key << " add to hash bucket success ..........");

      return UpdateNode2LRU(hash_node);
    }

    INT32 NodeGroup::Set(const string & key, const CHAR * data, INT32 size) {
      if (0 >= size) {
        return Set(key);
      }

      LIB_CACHE_LOG_DEBUG("NodeGroup set key:" << key << " size:" << size << " ....................");
      HashNode * hash_node = GetHashNodeFromHash(key);
      if (NULL == hash_node) {
        LIB_CACHE_LOG_DEBUG("NodeGroup set key:" << key << " has not in hash bucket ....................");
        hash_node = new HashNode();
        if (NULL == hash_node) {
          LIB_CACHE_LOG_ERROR("NodeGroup set key: " << key << " failed size:" << size
                                                    << " hash node object empty ..");
          return Err::kERR_NODE_GROUP_INFO_OBJECT_EMPTY;
        }

        LIB_CACHE_LOG_DEBUG("NodeGroup set key:" << key << " allocate hash node success ....................");
        if (NULL == (hash_node->node_ = GetFreeNode())) {
          LIB_CACHE_LOG_ERROR("NodeGroup set key: " << key << " failed size:" << size << " .................");
          return Err::kERR_NODE_GROUP_HAS_NO_FREE_NODE;
        }
      } else {
        LIB_CACHE_LOG_DEBUG("NodeGroup set key:" << key << " is in hash bucket ....................");
      }

      Mutex mutex(&hash_node->node_->mutex_);
      ScopeLock<Mutex> scope(&mutex);

      NodeMemInfo::Node * node = hash_node->node_;
      node->flag_ = CLEAN;
      if (node->data_capacity_ >= size) {
        LIB_CACHE_LOG_DEBUG("NodeGroup set key:" << key << " capacity size:" << node->data_capacity_
                                                 << " size:" << size << " .........");
        if (0 >= node->data_offset_) {
          LIB_CACHE_LOG_ERROR("NodeGroup set key: " << key << " failed size:" << size
                                                    << " node data offset:" << node->data_offset_
                                                    << " capacity:" << node->data_capacity_
                                                    << " data size:" << node->data_size_);

          return Err::kERR_NODE_GROUP_NODE_OFFSET_INVALID;
        }
        ::memcpy(Offset2DataPoint(node->data_offset_), data, size);
        node->data_size_ = size;

        LIB_CACHE_LOG_DEBUG("NodeGroup set key: " << key << " size:"
                                                  << size << " success not allocate chunk .................");

        atomic_add(1, &(node_mem_info_->total_keys_));

        return UpdateNode2LRU(hash_node);
      }

      if (0 < node->data_capacity_ && 0 < node->data_offset_) {
        LIB_CACHE_LOG_DEBUG("NodeGroup set key:" << key << " size:" << size
                                                 << " need free old and allocate new chunk  ....................");
        if (NULL == pt_malloc_) {
          LIB_CACHE_LOG_ERROR("NodeGroup free chunk failed ptmalloc object is empty");

          return Err::kERR_NODE_GROUP_INFO_OBJECT_EMPTY;
        }

        pt_malloc_->free(reinterpret_cast<VOID *>(Offset2DataPoint(node->data_offset_)));
        node->data_capacity_ = 0;
        node->data_offset_ = 0;
        node->data_size_ = 0;
      }

      CHAR * buf = reinterpret_cast<CHAR *>(pt_malloc_->malloc(size));
      if (NULL == buf) {
        LIB_CACHE_LOG_WARN("NodeGroup allocate chunk size:" << size
                                                            << " failed maybe has no chunk "
                                                            << "plz check ptmalloc log .......");
        INT32 result = FreeExpiredNode(size);
        if (0 != result) {
          LIB_CACHE_LOG_ERROR("NodeGroup free expired node failed result:" << result << " size:" << size);

          return result;
        }

        buf = reinterpret_cast<CHAR *>(pt_malloc_->malloc(size));
        if (NULL == buf) {
          LIB_CACHE_LOG_ERROR("NodeGroup allocate size:" << size << " failed maybe has no chunk plz check ptmalloc log");
          return Err::kERR_NODE_GROUP_INFO_OBJECT_EMPTY;
        }

        result = Running(NULL, 32*1024);
        if (0 != result) {
          LIB_CACHE_LOG_ERROR("NodeGroup start free expired node job failed result:" << result);

          return result;
        }
      }

      ::memcpy(buf, data, size);
      node->data_size_ = size;
      node->data_capacity_ = size;
      node->data_offset_ = CharPoint2Offset(buf);

      LIB_CACHE_LOG_DEBUG("NodeGroup set key:" << key << " data size:" << node->data_size_
                                                 << " data capacity:" << node->data_capacity_
                                                 << " data offset:" << node->data_offset_
                                                 << " allocate new chunk .........");

      memcpy(node->key_, key.c_str(), key.length());
      node->key_length_ = key.length();
      node->last_access_timestamp_ = TimeFormat::GetCurTimestampLong();

      LIB_CACHE_LOG_DEBUG("NodeGroup set key:" << key << " size:" << size << " set data to buffer success  .........");

      atomic_add(1, &(node_mem_info_->total_keys_));
      
      DumpNode(node);
      INT32 result = AddNode2Hash(hash_node);
      if (0 != result) {
        return result;
      }

      LIB_CACHE_LOG_DEBUG("NodeGroup set key:" << key << " size:" << size << " add to hash bucket success ..........");

      return UpdateNode2LRU(hash_node);
    }

    INT32 NodeGroup::Get(const string & key, CHAR * data, INT32 * max_size) {
      LIB_CACHE_LOG_DEBUG("NodeGroup get key:" << key << " max size:" << *max_size << " ....................");
      HashNode * hash_node = GetHashNodeFromHash(key);
      if ((NULL == hash_node) || (NULL == hash_node->node_)) {
        LIB_CACHE_LOG_DEBUG("NodeGroup get key:" << key << " is not exists ..............................");
        return KEY_NOT_EXISTS;
      }

      LIB_CACHE_LOG_DEBUG("NodeGroup get key:" << key << " is exists ....................");
      Mutex mutex(&hash_node->node_->mutex_);
      ScopeLock<Mutex> scope(&mutex);

      NodeMemInfo::Node * node = hash_node->node_;
      if (0 >= node->data_offset_ || 0 >= node->data_size_) {
        LIB_CACHE_LOG_DEBUG("NodeGroup get key:" << key << " data is not in cache ..............................");
        return DIRTY;
      }

      if (DIRTY == node->flag_) {
        LIB_CACHE_LOG_DEBUG("NodeGroup get key:" << key << " is dirty ....................");
        return DIRTY;
      }

      if (*max_size < node->data_size_) {
        LIB_CACHE_LOG_DEBUG("NodeGroup get key:" << key << " data size: " << node->data_size_ << " max size:" << *max_size << " ......");
        *max_size = node->data_size_;
        return Err::kERR_NODE_GROUP_GET_BUFFER_IS_TOO_SMALL;
      }

      ::memcpy(data, Offset2DataPoint(node->data_offset_), node->data_size_);
      *max_size = node->data_size_;
      node->last_access_timestamp_ = TimeFormat::GetCurTimestampLong();

      LIB_CACHE_LOG_DEBUG("NodeGroup get key:" << key << " size:" << *max_size << " ....................");

      return UpdateNode2LRU(hash_node);
    }

    INT32 NodeGroup::Status(const string & key) {
      LIB_CACHE_LOG_DEBUG("NodeGroup get key:" << key << " status ....................");
      HashNode * hash_node = GetHashNodeFromHash(key);
      if ((NULL == hash_node) || (NULL == hash_node->node_)) {
        LIB_CACHE_LOG_DEBUG("NodeGroup get key status key:" << key << " is not exists ..............................");
        return KEY_NOT_EXISTS;
      }

      LIB_CACHE_LOG_DEBUG("NodeGroup get key status key:" << key << " is exists ....................");
      Mutex mutex(&hash_node->node_->mutex_);
      ScopeLock<Mutex> scope(&mutex);

      NodeMemInfo::Node * node = hash_node->node_;
      if (0 >= node->data_offset_ || 0 >= node->data_size_) {
        LIB_CACHE_LOG_DEBUG("NodeGroup get key status key:" << key << " data is not in cache ..............................");
        return DIRTY;
      }

      if (DIRTY == node->flag_) {
        LIB_CACHE_LOG_DEBUG("NodeGroup get key status key:" << key << " is dirty ....................");
        return DIRTY;
      }

      return IN_CACHE;
    }

    INT32 NodeGroup::Del(const string & key) {
      LIB_CACHE_LOG_DEBUG("NodeGroup delete key:" << key << " ....................");
      HashNode * hash_node = GetHashNodeFromHash(key);
      if ((NULL == hash_node) || (NULL == hash_node->node_)) {
        LIB_CACHE_LOG_DEBUG("NodeGroup del key:" << key << " is not exists ..............................");
        return 0;
      }

      LIB_CACHE_LOG_DEBUG("NodeGroup delete key:" << key << " is exists ....................");
      INT32 result = DelNodeFromBucket(hash_node);
      if (0 != result) {
        return result;
      }

      LIB_CACHE_LOG_DEBUG("NodeGroup delete key:" << key << " delete from hash bucket success ....................");

      result = DelNodeFromLRU(hash_node);
      if (0 != result) {
        return result;
      }

      NodeMemInfo::Node * node = hash_node->node_;
      delete hash_node;

      LIB_CACHE_LOG_DEBUG("NodeGroup delete key:" << key << " delete from lru success ....................");
      /*
      Mutex mutex(&node->mutex_);
      ScopeLock<Mutex> scope(&mutex);
      */
      atomic_sub(1, &(node_mem_info_->total_keys_));

      return RecycleNode(node);
    }

    INT32 NodeGroup::Touch(const string & key) {
      LIB_CACHE_LOG_DEBUG("NodeGroup touch key:" << key << " ....................");
      HashNode * hash_node = GetHashNodeFromHash(key);
      if ((NULL == hash_node) || (NULL == hash_node->node_)) {
        LIB_CACHE_LOG_DEBUG("NodeGroup get key:" << key << " is not exists ..............................");
        return KEY_NOT_EXISTS;
      }

      LIB_CACHE_LOG_DEBUG("NodeGroup delete key:" << key << " is exists ....................");

      Mutex mutex(&hash_node->node_->mutex_);
      ScopeLock<Mutex> scope(&mutex);

      hash_node->node_->flag_ = DIRTY;
      hash_node->node_->last_access_timestamp_ = TimeFormat::GetCurTimestampLong();

      LIB_CACHE_LOG_DEBUG("NodeGroup delete key:" << key << " touch success ....................");

      return UpdateNode2LRU(hash_node);
    }

    UINT64 NodeGroup::LastAccessTimestamp() {
      if (NULL == lru_node_head_) {
        return 0;
      }

      return lru_node_head_->node_->last_access_timestamp_;
    }

    INT32 NodeGroup::FreeExpiredNode(INT32 size) {
      UINT64 now = TimeFormat::GetCurTimestampLong();
      ScopeLock<Mutex> scope(&mutex_);

      INT32 free_size = 0;
      HashNode * p = lru_node_head_;
      while (NULL != p && lru_node_head_ != p->lru_prev_) {
        NodeMemInfo::Node * node = p->node_;

        if ((NULL != node) && ((now - node->last_access_timestamp_)/1000 > 30) &&
                (0 < node->data_capacity_) && (0 < node->data_size_)) {
          Mutex mutex(&node->mutex_);
          ScopeLock<Mutex> scope_node(&mutex);

          CHAR * data_start = Offset2DataPoint(node->data_offset_);
          pt_malloc_->free(reinterpret_cast<VOID *>(data_start));
          free_size += node->data_size_;

          node->data_capacity_ = 0;
          node->data_offset_ = 0;
          node->data_size_ = 0;

          if (free_size >= size) {
            return 0;
          }
        }

        p = p->lru_prev_;
      }

      return 0;
    }

    INT32 NodeGroup::Working(VOID * parameter) {
      LIB_CACHE_LOG_DEBUG("NodeGroup free expired node start ...........................................");

      INT32 ONCE_FREE = 1024*1024;
      INT32 need_free_count = 1;
      for (INT32 i = 0; i < need_free_count; ++i) {
        INT32 result = FreeExpiredNode(ONCE_FREE);
        if (0 != result) {
          LIB_CACHE_LOG_ERROR("NodeGroup free expired node failed result:" << result);
        }
      }

      pt_malloc_->MergeFragment();

      return 0;
    }

    VOID NodeGroup::Dump() {
      LIB_CACHE_LOG_DEBUG("Dump all nodes ***********************************************************************");
      DumpHead();
      for (INT32 i = 0; i < node_mem_info_->total_group_count_; ++i) {
        NodeGroupMem * group = Offset2Group(node_mem_info_->node_group_offset_[i]);
        if (NULL == group) {
          continue;
        }
        for (INT32 j = 0; j < NodeMemInfo::NODES_PER_GROUP; ++j) {
          if (FREE != group->node_list_[j].flag_) {
            DumpNode(&group->node_list_[j]);
          }
        }
      }

      DumpLRULink();

      DumpHashBucket();
    }

    VOID NodeGroup::DumpHead() {
      LIB_CACHE_LOG_DEBUG("Node Group Head B ***********************************************************************");
      LIB_CACHE_LOG_DEBUG("flag:" << node_mem_info_->flag_);
      LIB_CACHE_LOG_DEBUG("total group count:" << node_mem_info_->total_group_count_);
      LIB_CACHE_LOG_DEBUG("used group link head:" << node_mem_info_->used_group_link_head_);
      LIB_CACHE_LOG_DEBUG("used node link head:" << node_mem_info_->used_node_link_head_);
      LIB_CACHE_LOG_DEBUG("free group link head:" << node_mem_info_->free_group_link_head_);
      LIB_CACHE_LOG_DEBUG("free node link head:" << node_mem_info_->free_node_link_head_);
      LIB_CACHE_LOG_DEBUG("Node Group Head B ***********************************************************************");
    }

    VOID NodeGroup::DumpLRULink() {
      if (NULL == lru_node_head_) {
        LIB_CACHE_LOG_DEBUG("LRU link has no nodes *****************************************************************");
        return;
      }

      LIB_CACHE_LOG_DEBUG("LRU link nodes ***********************************************************************");

      HashNode * p = lru_node_head_;
      while (NULL != p) {
        DumpNode(p->node_);
        if (p->lru_next_ == lru_node_head_) {
          break;
        }

        p = p->lru_next_;
      }
    }

    VOID NodeGroup::DumpHashBucket() {
      if (NULL == hash_bucket_) {
        LIB_CACHE_LOG_DEBUG("Hash Bucket has no nodes ***************************************************************");
        return;
      }

      LIB_CACHE_LOG_DEBUG("Hash Bucket info B ***********************************************************************");
      for (INT32 i = 0; i < keys_hash_; ++i) {
        if (NULL == hash_bucket_[i]) {
          continue;
        }

        LIB_CACHE_LOG_DEBUG("Bucket index:" << i << "****************************************************************");

        HashNode * p = hash_bucket_[i];
        while (NULL != p) {
          DumpNode(p->node_);
          p = p->hash_next_;
        }
      }
      LIB_CACHE_LOG_DEBUG("Hash Bucket info E ***********************************************************************");
    }

    VOID NodeGroup::DumpNode(const NodeMemInfo::Node * node) {
      if (NULL == node) {
        return;
      }

      LIB_CACHE_LOG_DEBUG("Node info B *****************************************************************************");
      LIB_CACHE_LOG_DEBUG("key:" << node->key_);
      LIB_CACHE_LOG_DEBUG("key length:" << node->key_length_);
      LIB_CACHE_LOG_DEBUG("flag:" << node->flag_);
      LIB_CACHE_LOG_DEBUG("data offset:" << node->data_offset_);
      LIB_CACHE_LOG_DEBUG("data size:" << node->data_size_);
      LIB_CACHE_LOG_DEBUG("data capacity:" << node->data_capacity_);
      LIB_CACHE_LOG_DEBUG("last access timestamp:" << node->last_access_timestamp_);
      LIB_CACHE_LOG_DEBUG("prev used group index:" << node->prev_used_group_index_);
      LIB_CACHE_LOG_DEBUG("prev used node index:" << node->prev_used_node_index_);
      LIB_CACHE_LOG_DEBUG("next used group index:" << node->next_used_group_index_);
      LIB_CACHE_LOG_DEBUG("next used node index:" << node->next_used_node_index_);
      LIB_CACHE_LOG_DEBUG("next free group index:" << node->next_free_group_index_);
      LIB_CACHE_LOG_DEBUG("next free node index:" << node->next_free_node_index_);
      LIB_CACHE_LOG_DEBUG("Node info E *****************************************************************************");
    }

    VOID NodeGroup::Destroy() {
      if (hash_bucket_) {
        for (INT32 i = 0; i < keys_hash_; ++i) {
          if (NULL == hash_bucket_[i]) {
            continue;
          }
          HashNode * p = hash_bucket_[i];
          while (NULL != p) {
            HashNode * del = p;
            p = p->hash_next_;
            delete del;
          }
          hash_bucket_[i] = NULL;
        }
        ::free(hash_bucket_);
        hash_bucket_ = NULL;
      }

      if (pt_malloc_) {
        delete pt_malloc_;
        pt_malloc_ = NULL;
      }
    }
  }  // namespace cache
}  // namespace lib
