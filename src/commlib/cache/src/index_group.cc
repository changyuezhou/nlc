// Copyright (c) 2013 zhou chang yue. All rights reserved.

#include <string.h>
#include "commlib/cache/inc/index_group.h"
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

    INT32 IndexGroup::Initial(IndexMemInfo * index_mem_info, ChunkInfo * chunk_info) {
      if (NULL == index_mem_info || NULL == chunk_info) {
        LIB_CACHE_LOG_ERROR("IndexGroup initial failed node memory object is empty");

        return Err::kERR_INDEX_GROUP_INFO_OBJECT_EMPTY;
      }
      index_mem_info_ = index_mem_info;

      return Initial(chunk_info);
    }

    INT32 IndexGroup::Initial(ChunkInfo * chunk_info) {
      if (NULL == index_mem_info_ || NULL == chunk_info) {
        LIB_CACHE_LOG_ERROR("IndexGroup initial failed node memory object is empty");

        return Err::kERR_INDEX_GROUP_INFO_OBJECT_EMPTY;
      }

      pt_malloc_ = new PTMalloc(chunk_info);
      if (NULL == pt_malloc_) {
        LIB_CACHE_LOG_ERROR("IndexGroup initial failed pt malloc object is empty");

        return Err::kERR_INDEX_GROUP_INFO_OBJECT_EMPTY;
      }

      if (IndexMemInfo::INDEX_GROUP_USABLE != index_mem_info_->flag_) {
        LIB_CACHE_LOG_DEBUG("IndexGroup is not usable must initial index group information ................. ");
        INT32 result = InitialIndexGroupInfo();
        if (0 != result) {
          return result;
        }
      }

      INT32 result = index_nodes_lists_.InitialTree();
      if (0 != result) {
        LIB_CACHE_LOG_ERROR("IndexGroup initial node tree list failed result:" << result << " ............");
        return result;
      }

      result = LoadIndexNode2Tree();
      if (0 != result) {
        LIB_CACHE_LOG_ERROR("IndexGroup load node into tree list failed result:" << result << " ............");
        return result;
      }

      LIB_CACHE_LOG_DEBUG("IndexGroup initial group success ..........................");

      return 0;
    }

    INT32 IndexGroup::InitialIndexGroupInfo() {
      if (NULL == index_mem_info_) {
        LIB_CACHE_LOG_ERROR("IndexGroup initial group info failed index memory object is empty");

        return Err::kERR_INDEX_GROUP_INFO_OBJECT_EMPTY;
      }

      memset(index_mem_info_, 0x00, sizeof(IndexMemInfo));
      if (0 != InitialLock(&index_mem_info_->mutex_)) {
        LIB_CACHE_LOG_ERROR("IndexGroup initial index memory info lock failed");
        return Err::kERR_INDEX_GROUP_INITIAL_MUTEX_FAILED;
      }

      Mutex mutex(&index_mem_info_->mutex_);
      if (0 == mutex.TryLock()) {
        index_mem_info_->used_group_link_head_ = IndexMemInfo::LINK_TAIL;
        index_mem_info_->used_node_link_head_ = IndexMemInfo::LINK_TAIL;
        index_mem_info_->free_group_link_head_ = IndexMemInfo::LINK_TAIL;
        index_mem_info_->free_node_link_head_ = IndexMemInfo::LINK_TAIL;

        index_mem_info_->free_key_node_head_ = IndexMemInfo::LINK_TAIL;

        index_mem_info_->flag_ = IndexMemInfo::INDEX_GROUP_USABLE;
        mutex.UnLock();
      }

      INT32 result = AllocateIndexGroup();
      if (0 != result) {
        return result;
      }

      LIB_CACHE_LOG_DEBUG("IndexGroup initial group info success ..........................");

      return 0;
    }

    INT32 IndexGroup::LoadIndexNode2Tree() {
      if (NULL == index_mem_info_) {
        LIB_CACHE_LOG_ERROR("IndexGroup load index node to tree failed memory info object is empty");

        return Err::kERR_INDEX_GROUP_INFO_OBJECT_EMPTY;
      }

      if (IndexMemInfo::LINK_TAIL == index_mem_info_->used_group_link_head_ ||
              IndexMemInfo::LINK_TAIL == index_mem_info_->used_node_link_head_) {
        LIB_CACHE_LOG_DEBUG("IndexGroup load index node to tree has no node to loading ............");
        return 0;
      }

      INT32 group_index = index_mem_info_->used_group_link_head_;
      INT32 node_index = index_mem_info_->used_node_link_head_;

      while ((IndexMemInfo::LINK_TAIL < group_index && IndexMemInfo::MAX_GROUP_COUNT > group_index) &&
             (IndexMemInfo::LINK_TAIL < node_index && IndexMemInfo::NODES_PER_GROUP > node_index)) {
        IndexNode * node = Offset2Node(group_index, node_index);
        if (NULL == node) {
          LIB_CACHE_LOG_ERROR("IndexGroup offset to node failed group index:" << group_index
                                                                             << " node index:" << node_index);
          return Err::kERR_INDEX_GROUP_INFO_OBJECT_EMPTY;
        }

        index_nodes_lists_.Insert(node->index_key_, node);

        LIB_CACHE_LOG_DEBUG("IndexGroup load node key:" << node->index_key_ << " success ....");

        group_index = node->next_used_group_index_;
        node_index = node->next_used_node_index_;
      }

      return 0;
    }

    INT32 IndexGroup::AllocateIndexGroup() {
      if (NULL == pt_malloc_ || NULL == index_mem_info_) {
        LIB_CACHE_LOG_ERROR("IndexGroup allocate index group failed ptmalloc object is empty");

        return Err::kERR_INDEX_GROUP_INFO_OBJECT_EMPTY;
      }

      if (index_mem_info_->total_group_count_ >= NodeMemInfo::MAX_GROUP_COUNT) {
        LIB_CACHE_LOG_ERROR("IndexGroup allocate index group failed total group count:" << index_mem_info_->total_group_count_);
        return Err::kERR_NODE_GROUP_COUNT_IS_TOO_MANY;
      }

      IndexGroupMem * index_group = reinterpret_cast<IndexGroupMem *>(pt_malloc_->malloc(sizeof(IndexGroupMem)));
      if (NULL == index_group) {
        LIB_CACHE_LOG_ERROR("IndexGroup allocate index group failed maybe has no chunk plz check ptmalloc log .........");

        return Err::kERR_INDEX_GROUP_INFO_OBJECT_EMPTY;
      }

      Mutex mutex(&index_mem_info_->mutex_);
      ScopeLock<Mutex> scope(&mutex);
      index_mem_info_->index_group_offset_[index_mem_info_->total_group_count_++] = CharPoint2Offset(reinterpret_cast<CHAR *>(index_group));

      LIB_CACHE_LOG_DEBUG("IndexGroup allocate index group info success group count:" << index_mem_info_->total_group_count_
                                                                               << " ..........................");

      return InitialIndexGroup(index_group);
    }

    INT32 IndexGroup::InitialIndexGroup(IndexGroupMem * index_group) {
      if (NULL == index_group || NULL == index_mem_info_) {
        LIB_CACHE_LOG_ERROR("IndexGroup initial index group failed .................");

        return Err::kERR_INDEX_GROUP_INFO_OBJECT_EMPTY;
      }

      index_group->flag_ = IndexMemInfo::INDEX_GROUP_NOT_INITIAL;
      memset(index_group->nodes_, 0x00, sizeof(index_group->nodes_));
      for (INT32 i = (IndexMemInfo::NODES_PER_GROUP - 1); i >= 0; --i) {
        index_group->nodes_[i].prev_used_group_index_ = IndexMemInfo::LINK_TAIL;
        index_group->nodes_[i].prev_used_node_index_ = IndexMemInfo::LINK_TAIL;
        index_group->nodes_[i].next_used_group_index_ = IndexMemInfo::LINK_TAIL;
        index_group->nodes_[i].next_used_node_index_ = IndexMemInfo::LINK_TAIL;
        index_group->nodes_[i].key_node_head_ = IndexMemInfo::LINK_TAIL;
        index_group->nodes_[i].next_free_group_index_ = index_mem_info_->free_group_link_head_;
        index_group->nodes_[i].next_free_node_index_ = index_mem_info_->free_node_link_head_;

        if (0 != InitialLock(&index_group->nodes_[i].mutex_)) {
          return Err::kERR_NODE_GROUP_NODE_INITIAL_LOCK_FAILED;
        }

        index_mem_info_->free_group_link_head_ = (index_mem_info_->total_group_count_ - 1);
        index_mem_info_->free_node_link_head_ = i;
      }

      if (0 != InitialLock(&index_group->mutex_)) {
        return Err::kERR_NODE_GROUP_NODE_INITIAL_LOCK_FAILED;
      }
      index_group->flag_ = IndexMemInfo::INDEX_GROUP_USABLE;

      LIB_CACHE_LOG_DEBUG("IndexGroup initial group success free group head:" << index_mem_info_->free_group_link_head_
                                                                             << " free node head:"
                                                                             << index_mem_info_->free_node_link_head_);

      return 0;
    }

    INT64 IndexGroup::GetFreeIndexNodeOffset() {
      if (IndexMemInfo::LINK_TAIL == index_mem_info_->free_group_link_head_ ||
          IndexMemInfo::LINK_TAIL == index_mem_info_->free_node_link_head_) {
        if (0 != AllocateIndexGroup() || IndexMemInfo::LINK_TAIL == index_mem_info_->free_group_link_head_ ||
            IndexMemInfo::LINK_TAIL == index_mem_info_->free_node_link_head_) {
          LIB_CACHE_LOG_ERROR("IndexGroup get free index node failed .................");
          return Err::kERR_INDEX_GROUP_HAS_NO_FREE_INDEX_NODE;
        }
      }

      Mutex mutex(&index_mem_info_->mutex_);
      ScopeLock<Mutex> scope(&mutex);
      INT32 group_index = index_mem_info_->free_group_link_head_;
      INT32 node_index = index_mem_info_->free_node_link_head_;

      IndexNode * node = Offset2Node(group_index, node_index);
      index_mem_info_->free_group_link_head_ = node->next_free_group_index_;
      index_mem_info_->free_node_link_head_ = node->next_free_node_index_;

      return GetOffsetFromIndex(group_index, node_index);
    }

    INT32 IndexGroup::InsertIndexNode2LinkList(INT64 position, INT64 node) {
      INT32 node_group = GetIndexNodeGroupIndex(node);
      INT32 node_index = GetIndexNodeIndex(node);
      if (0 > node_group || IndexMemInfo::MAX_GROUP_COUNT <= node_group ||
              0 > node_index || IndexMemInfo::MAX_GROUP_COUNT <= node_index) {
        LIB_CACHE_LOG_ERROR("IndexGroup insert node failed position :" << position
                                                                       << " node group:"
                                                                       << node_group
                                                                       << " node index:"
                                                                       << node_index << " .................");

        return Err::kERR_INDEX_GROUP_INSERT_INDEX_NODE_OFFSET_INVALID;
      }
      Mutex mutex(&index_mem_info_->mutex_);
      ScopeLock<Mutex> scope(&mutex);

      IndexNode * cur_node = Offset2Node(node_group, node_index);
      if (IndexMemInfo::LINK_HEAD == position) {  // add to link head
        if (IndexMemInfo::LINK_TAIL != index_mem_info_->used_group_link_head_ &&
                IndexMemInfo::LINK_TAIL != index_mem_info_->used_node_link_head_) {
          IndexNode * head_node = Offset2Node(index_mem_info_->used_group_link_head_, index_mem_info_->used_node_link_head_);
          head_node->prev_used_group_index_ = node_group;
          head_node->prev_used_node_index_ = node_index;
        }
        cur_node->next_used_group_index_ = index_mem_info_->used_group_link_head_;
        cur_node->next_used_node_index_ = index_mem_info_->used_node_link_head_;

        index_mem_info_->used_group_link_head_ = node_group;
        index_mem_info_->used_node_link_head_ = node_index;

        return 0;
      }

      INT32 position_group = GetIndexNodeGroupIndex(position);
      INT32 position_index = GetIndexNodeIndex(position);
      IndexNode * pos = Offset2Node(position_group, position_index);

      cur_node->next_used_group_index_ = pos->next_used_group_index_;
      cur_node->next_used_node_index_ = pos->next_used_node_index_;

      if (IndexMemInfo::LINK_TAIL != pos->next_used_group_index_ &&
          IndexMemInfo::LINK_TAIL != pos->next_used_node_index_) {
        IndexNode * next_node = Offset2Node(pos->next_used_group_index_, pos->next_used_node_index_);
        next_node->prev_used_group_index_ = node_group;
        next_node->next_used_node_index_ = node_index;
      }
      pos->next_used_group_index_ = node_group;
      pos->next_used_node_index_ = node_index;

      cur_node->prev_used_group_index_ = position_group;
      cur_node->prev_used_node_index_ = position_index;

      return 0;
    }

    VOID IndexGroup::SetNode(IndexNode * node, const string & index_key) {
      if (NULL == node) {
        return;
      }

      ::memcpy(node->index_key_, index_key.c_str(), index_key.length());
      node->index_key_length_ = index_key.length();
      node->key_node_head_ = IndexMemInfo::LINK_TAIL;
      node->prev_used_group_index_ = IndexMemInfo::LINK_TAIL;
      node->prev_used_node_index_ = IndexMemInfo::LINK_TAIL;
      node->next_used_group_index_ = IndexMemInfo::LINK_TAIL;
      node->next_used_node_index_ = IndexMemInfo::LINK_TAIL;
      node->next_free_group_index_ = IndexMemInfo::LINK_TAIL;
      node->next_free_node_index_ = IndexMemInfo::LINK_TAIL;
    }

    INT32 IndexGroup::ClearNode(IndexNode * node) {
      if (NULL == node) {
        return 0;
      }

      Mutex mutex(&node->mutex_);
      ScopeLock<Mutex> scope(&mutex);

      memset(node->index_key_, 0x00, sizeof(node->index_key_));
      node->index_key_length_ = 0;

      while (IndexMemInfo::LINK_TAIL != node->key_node_head_) {
        INT32 result = DelKeyNode(&node->key_node_head_, Offset2KeyNode(node->key_node_head_));
        if (0 != result) {
          return result;
        }
      }

      node->key_node_head_ = IndexMemInfo::LINK_TAIL;

      return 0;
    }

    INT64 IndexGroup::DelIndexNodeFromLinkList(IndexNode * node) {
      if (NULL == node) {
        return 0;
      }

      INT32 prev_group_index = node->prev_used_group_index_;
      INT32 prev_node_index = node->prev_used_node_index_;
      INT32 next_group_index = node->next_used_group_index_;
      INT32 next_node_index = node->next_used_node_index_;
      INT32 cur_group_index = IndexMemInfo::LINK_TAIL;
      INT32 cur_node_index = IndexMemInfo::LINK_TAIL;

      if (IndexMemInfo::LINK_TAIL != prev_group_index || IndexMemInfo::LINK_TAIL != prev_node_index) {
        IndexNode *prev_node = Offset2Node(prev_group_index, prev_node_index);
        Mutex prev_mutex(&prev_node->mutex_);
        prev_mutex.Lock();
        cur_group_index = prev_node->next_used_group_index_;
        cur_node_index = prev_node->next_used_node_index_;
        prev_node->next_used_group_index_ = next_group_index;
        prev_node->next_used_node_index_ = next_node_index;
        prev_mutex.UnLock();
      }

      if (IndexMemInfo::LINK_TAIL != next_group_index || IndexMemInfo::LINK_TAIL != next_node_index) {
        IndexNode *next_node = Offset2Node(next_group_index, next_node_index);
        Mutex next_mutex(&next_node->mutex_);
        next_mutex.Lock();
        cur_group_index = next_node->prev_used_group_index_;
        cur_node_index = next_node->prev_used_node_index_;
        next_node->prev_used_group_index_ = prev_group_index;
        next_node->prev_used_node_index_ = prev_node_index;
        next_mutex.UnLock();
      }

      INT32 result = ClearNode(node);
      if (0 != result) {
        return result;
      }

      Mutex mutex(&index_mem_info_->mutex_);
      ScopeLock<Mutex> scope(&mutex);
      IndexNode * head_node = Offset2Node(index_mem_info_->used_group_link_head_,
                                                   index_mem_info_->used_node_link_head_);
      if (head_node == node) {
        cur_group_index = index_mem_info_->used_group_link_head_;
        cur_node_index = index_mem_info_->used_node_link_head_;

        index_mem_info_->used_group_link_head_ = next_group_index;
        index_mem_info_->used_node_link_head_ = next_node_index;
      }

      node->next_free_group_index_ = index_mem_info_->free_group_link_head_;
      node->next_free_node_index_ = index_mem_info_->free_node_link_head_;

      index_mem_info_->free_group_link_head_ = cur_group_index;
      index_mem_info_->free_node_link_head_ = cur_node_index;

      return 0;
    }

    INT32 IndexGroup::AllocateKeyNodeGroup() {
      if (NULL == pt_malloc_ || NULL == index_mem_info_) {
        LIB_CACHE_LOG_ERROR("IndexGroup allocate key node group failed ptmalloc or index_mem_info object is empty");

        return Err::kERR_INDEX_GROUP_INFO_OBJECT_EMPTY;
      }

      KeyNodeGroup * key_node_group = reinterpret_cast<KeyNodeGroup *>(pt_malloc_->malloc(sizeof(KeyNodeGroup)));
      if (NULL == key_node_group) {
        LIB_CACHE_LOG_ERROR("IndexGroup allocate key node group failed maybe has no chunk plz check ptmalloc log .........");

        return Err::kERR_INDEX_GROUP_INFO_OBJECT_EMPTY;
      }

      INT64 offset = CharPoint2Offset(reinterpret_cast<CHAR *>(key_node_group));

      Mutex mutex(&index_mem_info_->mutex_);
      ScopeLock<Mutex> scope(&mutex);
      if (0 >= index_mem_info_->key_nodes_group_head_offset_) {
        index_mem_info_->key_nodes_group_head_offset_ = offset;
      } else {
        KeyNodeGroup * tail_group = Offset2KeyNodeGroup(index_mem_info_->key_nodes_group_tail_offset_);
        tail_group->next_group_offset_ = offset;
      }
      index_mem_info_->key_nodes_group_tail_offset_ = offset;

      LIB_CACHE_LOG_DEBUG("IndexGroup allocate key node group info success head:"
                                  << index_mem_info_->key_nodes_group_head_offset_
                                  << " tail:" << index_mem_info_->key_nodes_group_tail_offset_
                                  << " offset:" << offset << " ..........................");

      return InitialKeyNodeGroup(key_node_group);
    }

    INT32 IndexGroup::InitialKeyNodeGroup(KeyNodeGroup * key_node_group) {
      if (NULL == key_node_group || NULL == index_mem_info_) {
        LIB_CACHE_LOG_ERROR("IndexGroup initial key node group failed .................");

        return Err::kERR_INDEX_GROUP_INFO_OBJECT_EMPTY;
      }

      for (INT32 i = (IndexMemInfo::KEY_NODES_PER_GROUP - 1); i >= 0; --i) {
        key_node_group->key_nodes_[i].prev_node_ = IndexMemInfo::LINK_TAIL;
        key_node_group->key_nodes_[i].next_node_ = index_mem_info_->free_key_node_head_;

        index_mem_info_->free_key_node_head_ = CharPoint2Offset(reinterpret_cast<CHAR *>(&key_node_group->key_nodes_[i]));
      }

      if (0 != InitialLock(&key_node_group->mutex_)) {
        return Err::kERR_NODE_GROUP_NODE_INITIAL_LOCK_FAILED;
      }

      return 0;
    }

    IndexGroup::KeyNode * IndexGroup::GetFreeKeyNode() {
      if ((NULL == index_mem_info_)) {
        LIB_CACHE_LOG_ERROR("IndexGroup get free key node failed .................");

        return NULL;
      }

      if ((IndexMemInfo::LINK_TAIL == index_mem_info_->free_key_node_head_)) {
        LIB_CACHE_LOG_DEBUG("IndexGroup get key node has no free node .................");
        INT32 result = AllocateKeyNodeGroup();
        if (0 != result) {
          LIB_CACHE_LOG_ERROR("IndexGroup get key node has no free node and allocate key node group failed result:"
                                      << result << ".................");

          return NULL;
        }

        if (IndexMemInfo::LINK_TAIL == index_mem_info_->free_key_node_head_) {
          LIB_CACHE_LOG_ERROR("IndexGroup get key node has no free node plz check memory size .................");

          return NULL;
        }
      }
      Mutex mutex(&index_mem_info_->mutex_);
      ScopeLock<Mutex> scope(&mutex);
      KeyNode * key_node = Offset2KeyNode(index_mem_info_->free_key_node_head_);
      index_mem_info_->free_key_node_head_ = key_node->next_node_;

      return key_node;
    }

    INT64 IndexGroup::InsertKeyNodeToLink(INT64 * head, INT64 key_node) {
      if (0 >= key_node || NULL == head) {
        LIB_CACHE_LOG_ERROR("IndexGroup delete key node failed .................");

        return Err::kERR_INDEX_GROUP_INFO_OBJECT_EMPTY;
      }

      if (IndexMemInfo::LINK_TAIL == *head) {
        LIB_CACHE_LOG_DEBUG("IndexGroup insert key node key:" << Offset2KeyNode(key_node)->key_ << " into link head ....");
        *head = key_node;

        KeyNode * cur = Offset2KeyNode(key_node);
        cur->prev_node_ = IndexMemInfo::LINK_TAIL;
        cur->next_node_ = IndexMemInfo::LINK_TAIL;

        return 0;
      }

      KeyNode * p = Offset2KeyNode(*head);
      while (IndexMemInfo::LINK_TAIL != p->next_node_) {
        p = Offset2KeyNode(p->next_node_);
      }

      KeyNode * cur = Offset2KeyNode(key_node);
      p->next_node_ = key_node;
      cur->prev_node_ = CharPoint2Offset(reinterpret_cast<CHAR *>(p));
      cur->next_node_ = IndexMemInfo::LINK_TAIL;

      LIB_CACHE_LOG_DEBUG("IndexGroup insert key node prev key:" << p->key_ << " current key:" << cur->key_ << " ....");

      return 0;
    }

    INT64 IndexGroup::DelKeyNode(INT64 * head, KeyNode * key_node) {
      if (NULL == key_node) {
        LIB_CACHE_LOG_ERROR("IndexGroup delete key node failed .................");

        return Err::kERR_INDEX_GROUP_INFO_OBJECT_EMPTY;
      }

      INT64 cur_node_offset = -1;
      INT64 prev_node_offset = key_node->prev_node_;
      INT64 next_node_offset = key_node->next_node_;

      if (IndexMemInfo::LINK_TAIL != next_node_offset) {
        KeyNode * next_node = Offset2KeyNode(next_node_offset);
        cur_node_offset = next_node->prev_node_;
        next_node->prev_node_ = prev_node_offset;
      }

      if (IndexMemInfo::LINK_TAIL == prev_node_offset) {
        *head = next_node_offset;
        cur_node_offset = *head;
      } else {
        KeyNode *prev_node = Offset2KeyNode(prev_node_offset);
        cur_node_offset = prev_node->next_node_;
        prev_node->next_node_ = next_node_offset;
      }

      Mutex mutex(&index_mem_info_->mutex_);
      ScopeLock<Mutex> scope(&mutex);
      key_node->next_node_ = index_mem_info_->free_key_node_head_;
      index_mem_info_->free_key_node_head_ = cur_node_offset;

      return 0;
    }

    BOOL IndexGroup::IsKeyNodeInLink(INT64 head, const string & unique_key) {
      if (0 >= head) {
        return FALSE;
      }

      KeyNode * p = Offset2KeyNode(head);
      while (NULL != p) {
        UINT32 length = unique_key.length();
        if (length < p->key_length_) {
          length = p->key_length_;
        }
        if (0 == ::strncmp(unique_key.c_str(), p->key_, length)) {
          return TRUE;
        }
        if (IndexMemInfo::LINK_TAIL == p->next_node_) {
          return FALSE;
        }

        p = Offset2KeyNode(p->next_node_);
      }

      return FALSE;
    }

    IndexGroup::KeyNode * IndexGroup::GetKeyNode(INT64 head, const string & unique_key) {
      if (0 >= head) {
        return NULL;
      }

      KeyNode * p = Offset2KeyNode(head);
      while (NULL != p) {
        UINT32 length = unique_key.length();
        if (length < p->key_length_) {
          length = p->key_length_;
        }
        if (0 == ::strncmp(unique_key.c_str(), p->key_, length)) {
          return p;
        }
        if (IndexMemInfo::LINK_TAIL == p->next_node_) {
          return NULL;
        }

        p = Offset2KeyNode(p->next_node_);
      }

      return NULL;
    }

    INT32 IndexGroup::GetKeyNodes(INT64 head, KEYS & unique_keys) {
      if (0 >= head) {
        return 0;
      }

      KeyNode * p = Offset2KeyNode(head);
      while (NULL != p) {
        unique_keys.push_back(p->key_);

        if (IndexMemInfo::LINK_TAIL == p->next_node_) {
          break;
        }

        p = Offset2KeyNode(p->next_node_);
      }

      return 0;
    }

    INT32 IndexGroup::InitialLock(pthread_mutex_t * mutex) {
      if (NULL == mutex) {
        LIB_CACHE_LOG_ERROR("IndexGroup initial mutex failed .................");

        return Err::kERR_INDEX_GROUP_INFO_OBJECT_EMPTY;
      }
      pthread_mutexattr_t attr;
      ::pthread_mutexattr_init(&attr);
      if (0 != ::pthread_mutex_init(mutex, &attr)) {
        return -1;
      }
      ::pthread_mutexattr_destroy(&attr);

      return 0;
    }

    IndexMemInfo::IndexNode * IndexGroup::Offset2Node(INT32 group_offset_index, INT32 node_index) {
      if (0 > group_offset_index || IndexMemInfo::MAX_GROUP_COUNT <= group_offset_index) {
        LIB_CACHE_LOG_WARN("IndexGroup offset to point group index:" << group_offset_index << " is invalid");

        return NULL;
      }

      return Offset2Node(index_mem_info_->index_group_offset_[group_offset_index], node_index);
    }

    IndexMemInfo::IndexNode * IndexGroup::Offset2Node(INT64 group_offset, INT32 node_index) {
      IndexGroupMem * group = reinterpret_cast<IndexGroupMem *>(reinterpret_cast<CHAR *>(index_mem_info_) + group_offset);
      if (0 > node_index || IndexMemInfo::NODES_PER_GROUP <= node_index) {
        LIB_CACHE_LOG_WARN("IndexGroup offset to point node index:" << node_index << " is invalid");

        return NULL;
      }

      return &group->nodes_[node_index];
    }

    IndexGroup::IndexGroupMem * IndexGroup::Offset2Group(INT64 group_offset) {
      if (0 >= group_offset) {
        LIB_CACHE_LOG_WARN("IndexGroup offset to point group offset:" << group_offset << " is invalid");

        return NULL;
      }

      return reinterpret_cast<IndexGroupMem *>(reinterpret_cast<CHAR *>(index_mem_info_) + group_offset);
    }

    IndexGroup::KeyNodeGroup * IndexGroup::Offset2KeyNodeGroup(INT64 offset) {
      if (0 >= offset) {
        LIB_CACHE_LOG_WARN("IndexGroup offset to point key node group offset:" << offset << " is invalid");

        return NULL;
      }

      return reinterpret_cast<KeyNodeGroup *>(reinterpret_cast<CHAR *>(index_mem_info_) + offset);
    }

    IndexGroup::KeyNode * IndexGroup::Offset2KeyNode(INT64 offset) {
      if (0 >= offset) {
        LIB_CACHE_LOG_WARN("IndexGroup offset to point key node offset:" << offset << " is invalid");

        return NULL;
      }

      return reinterpret_cast<KeyNode *>(reinterpret_cast<CHAR *>(index_mem_info_) + offset);
    }

    INT64 IndexGroup::CharPoint2Offset(CHAR * address) {
      return reinterpret_cast<INT64>(address - reinterpret_cast<CHAR *>(index_mem_info_));
    }

    INT32 IndexGroup::GetIndexNodeGroupIndex(INT64 offset) {
      return offset >> 32;
    }

    INT32 IndexGroup::GetIndexNodeIndex(INT64 offset) {
      return (offset & 0x00000000FFFFFFFF);
    }

    INT64 IndexGroup::GetOffsetFromIndex(INT32 group, INT32 node) {
      INT64 offset = group;
      return ((offset << 32) + node);
    }

    IndexGroup::IndexNode * IndexGroup::GetInsertPosition(const string & key) {
      RBTree<IndexNode>::RBTreeNode * tree_node = index_nodes_lists_.lower_bound_first(key);
      if (NULL != tree_node) {
        return tree_node->data_;
      }

      return NULL;
    }

    INT32 IndexGroup::InsertNode2Tree(IndexGroup::IndexNode * node) {
      if (NULL == index_mem_info_ || NULL == node) {
        LIB_CACHE_LOG_ERROR("IndexGroup get insert position failed .................");

        return Err::kERR_INDEX_GROUP_INFO_OBJECT_EMPTY;
      }

      index_nodes_lists_.Insert(node->index_key_, node);

      return 0;
    }

    BOOL IndexGroup::IsExists(const string & index_key, const string & unique_key) {
      if (NULL == index_mem_info_) {
        LIB_CACHE_LOG_ERROR("IndexGroup find index key:" << index_key << " unique key:" << unique_key << " failed .");

        return FALSE;
      }

      IndexNodeRBTree::RBTreeNode * rb_node = index_nodes_lists_.Search(index_key);

      if (NULL == rb_node || !IsKeyNodeInLink(rb_node->data_->key_node_head_, unique_key)) {
        return FALSE;
      }

      LIB_CACHE_LOG_DEBUG("IndexGroup index key:" << index_key << " unique key:" << unique_key << " is exists .......");

      return TRUE;
    }

    INT32 IndexGroup::Insert(const string & index_key, const string & unique_key) {
      LIB_CACHE_LOG_DEBUG("IndexGroup insert index key:" << index_key << " unique key:" << unique_key << " ....");
      if (NULL == index_mem_info_) {
        LIB_CACHE_LOG_ERROR("IndexGroup insert index key:" << index_key << " unique key:" << unique_key << " failed .");

        return Err::kERR_INDEX_GROUP_INFO_OBJECT_EMPTY;
      }

      if (IsExists(index_key, unique_key)) {
        LIB_CACHE_LOG_DEBUG("IndexGroup insert index key:" << index_key << " unique key:" << unique_key << " is exists ....");
        return 0;
      }

      KeyNode * key_node = GetFreeKeyNode();
      if (NULL == key_node) {
        LIB_CACHE_LOG_ERROR("IndexGroup insert index key:" << index_key << " unique key:"
                                                           << unique_key << " failed has no free key node");

        return Err::kERR_INDEX_GROUP_HAS_NO_FREE_KEY_NODE;
      }

      ::memcpy(key_node->key_, unique_key.c_str(), unique_key.length());
      key_node->key_length_ = unique_key.length();

      LIB_CACHE_LOG_DEBUG("IndexGroup insert index key:" << index_key << " unique key:"
                                                         << unique_key << " get free key node success ....");

      IndexNodeRBTree::RBTreeNode * rb_node = index_nodes_lists_.lower_bound_first(index_key);
      if (NULL != rb_node && (0 == ::strncmp(rb_node->data_->index_key_, index_key.c_str(), index_key.length())) &&
              (index_key.length() >= rb_node->data_->index_key_length_)) {
        LIB_CACHE_LOG_DEBUG("IndexGroup insert index key:" << index_key << " is exists unique key:"
                                                           << unique_key << " not exists .........");

        return InsertKeyNodeToLink(&rb_node->data_->key_node_head_, CharPoint2Offset(reinterpret_cast<CHAR *>(key_node)));
      }

      INT64 index_node_offset = GetFreeIndexNodeOffset();
      INT32 index_node_group = GetIndexNodeGroupIndex(index_node_offset);
      INT32 index_node_index = GetIndexNodeIndex(index_node_offset);
      IndexNode * node = Offset2Node(index_node_group, index_node_index);
      SetNode(node, index_key);

      if (NULL == rb_node) {
        INT32 result = InsertIndexNode2LinkList(IndexMemInfo::LINK_TAIL, index_node_offset);
        if (0 != result) {
          return result;
        }
        LIB_CACHE_LOG_DEBUG("IndexGroup insert index key:" << index_key << " into link head unique key:"
                                                           << unique_key << " success .........");

        index_nodes_lists_.Insert(index_key, node);

        return InsertKeyNodeToLink(&node->key_node_head_, CharPoint2Offset(reinterpret_cast<CHAR *>(key_node)));
      }

      INT32 position_group = IndexMemInfo::LINK_TAIL;
      INT32 position_node = IndexMemInfo::LINK_TAIL;
      if (IndexMemInfo::LINK_TAIL == rb_node->data_->prev_used_group_index_ ||
              IndexMemInfo::LINK_TAIL == rb_node->data_->prev_used_node_index_) {
        position_group = index_mem_info_->used_group_link_head_;
        position_node = index_mem_info_->used_node_link_head_;
      } else {
        IndexNode * prev_node = Offset2Node(rb_node->data_->prev_used_group_index_, rb_node->data_->prev_used_node_index_);
        position_group = prev_node->next_used_group_index_;
        position_node = prev_node->next_used_node_index_;
      }

      INT32 result = InsertIndexNode2LinkList(GetOffsetFromIndex(position_group, position_node), index_node_offset);
      if (0 != result) {
        return result;
      }

      index_nodes_lists_.Insert(index_key, node);

      LIB_CACHE_LOG_DEBUG("IndexGroup insert index key:" << index_key << " into link head unique key:"
                                                         << unique_key << " prev node index key:"
                                                         << rb_node->data_->index_key_ << " success .........");

      return InsertKeyNodeToLink(&node->key_node_head_, CharPoint2Offset(reinterpret_cast<CHAR *>(key_node)));
    }

    INT32 IndexGroup::Insert(const string & index_key, const KEYS & unique_keys) {
      LIB_CACHE_LOG_DEBUG("IndexGroup insert index key:" << index_key << " unique key size:"
                                                         << unique_keys.size() << " key node allocate success ....");

      UINT32 size = unique_keys.size();
      for (UINT32 i = 0; i < size; ++i) {
        INT32 result = Insert(index_key, unique_keys[i]);
        if (0 != result) {
          return result;
        }
      }

      LIB_CACHE_LOG_DEBUG("IndexGroup insert index key:" << index_key << " into link head unique key size:"
                                                         << size << " success .........");
      return 0;
    }

    INT32 IndexGroup::Delete(const string & index_key, const string & unique_key) {
      LIB_CACHE_LOG_DEBUG("IndexGroup delete index key:" << index_key << " unique key :"
                                                         << unique_key << " ....");

      if (NULL == index_mem_info_) {
        LIB_CACHE_LOG_ERROR("IndexGroup delete index key:" << index_key << " unique key:" << unique_key << " failed .");

        return Err::kERR_INDEX_GROUP_INFO_OBJECT_EMPTY;
      }

      IndexNodeRBTree::RBTreeNode * rb_node = index_nodes_lists_.Search(index_key);

      if (NULL == rb_node || NULL == rb_node->data_) {
        LIB_CACHE_LOG_WARN("IndexGroup delete index key:" << index_key << " is not exists unique key:"
                                                          << unique_key << " failed ....");

        return FALSE;
      }

      LIB_CACHE_LOG_DEBUG("IndexGroup delete index key:" << index_key << " find in cache ....");

      KeyNode * key_node = GetKeyNode(rb_node->data_->key_node_head_, unique_key);
      if (NULL == key_node) {
        LIB_CACHE_LOG_WARN("IndexGroup delete index key:" << index_key << " is exists unique key:"
                                                          << unique_key << " is not exists ....");

        return FALSE;
      }

      LIB_CACHE_LOG_DEBUG("IndexGroup delete index key:" << index_key << " unique key :"
                                                         << unique_key << " find in cache ....");

      Mutex mutex(&rb_node->data_->mutex_);
      ScopeLock<Mutex> scope(&mutex);

      INT32 result = DelKeyNode(&rb_node->data_->key_node_head_, key_node);
      if (0 != result) {
        return result;
      }

      LIB_CACHE_LOG_DEBUG("IndexGroup delete index key:" << index_key << " unique key :"
                                                         << unique_key << " in key node link ....");

      if (IndexMemInfo::LINK_TAIL == rb_node->data_->key_node_head_) {
        result = DelIndexNodeFromLinkList(rb_node->data_);
        if (0 != result) {
          return result;
        }

        LIB_CACHE_LOG_DEBUG("IndexGroup delete from red black tree key:" << index_key << " ..........");

        index_nodes_lists_.Delete(index_key);
      }

      LIB_CACHE_LOG_DEBUG("IndexGroup delete index key:" << index_key << " unique key:"
                                                        << unique_key << " success ....");

      return 0;
    }

    INT32 IndexGroup::Delete(const string & index_key, const KEYS & unique_keys) {
      LIB_CACHE_LOG_DEBUG("IndexGroup delete index key:" << index_key << " unique key size:"
                                                         << unique_keys.size() << " key node allocate success ....");

      UINT32 size = unique_keys.size();
      for (UINT32 i = 0; i < size; ++i) {
        INT32 result = Delete(index_key, unique_keys[i]);
        if (0 != result) {
          return result;
        }
      }

      LIB_CACHE_LOG_DEBUG("IndexGroup delete index key:" << index_key << " into link head unique key size:"
                                                         << size << " success .........");

      return 0;
    }

    INT32 IndexGroup::GetUniqueKeysFromStart2End(IndexNode * start, KEYS & unique_keys) {
      if (NULL == start) {
        return 0;
      }

      LIB_CACHE_LOG_DEBUG("IndexGroup get unique keys from index key:" << start->index_key_ << " to link tail ....");

      IndexNode * p = start;
      while (NULL != p) {
        INT32 result = GetKeyNodes(p->key_node_head_, unique_keys);
        if (0 != result) {
          return result;
        }

        if (IndexMemInfo::LINK_TAIL == p->next_used_group_index_ ||
                IndexMemInfo::LINK_TAIL == p->next_used_node_index_) {
          break;
        }

        p = Offset2Node(p->next_used_group_index_, p->next_used_node_index_);
      }

      return 0;
    }

    INT32 IndexGroup::GetUniqueKeysFromStart2Node(IndexNode * start, IndexNode * end, KEYS & unique_keys) {
      if (NULL == start || NULL == end) {
        return 0;
      }

      LIB_CACHE_LOG_DEBUG("IndexGroup get unique keys from index key:" << start->index_key_
                                                                       << " index key:" << end->index_key_ << " ....");

      IndexNode * p = start;
      while (NULL != p) {
        INT32 result = GetKeyNodes(p->key_node_head_, unique_keys);
        if (0 != result) {
          return result;
        }

        if (p == end) {
          break;
        }

        if (IndexMemInfo::LINK_TAIL == p->next_used_group_index_ ||
            IndexMemInfo::LINK_TAIL == p->next_used_node_index_) {
          break;
        }

        p = Offset2Node(p->next_used_group_index_, p->next_used_node_index_);
      }

      return 0;
    }

    INT32 IndexGroup::GetUniqueKeysFromStart2Head(IndexNode * start, KEYS & unique_keys) {
      if (NULL == start) {
        return 0;
      }

      LIB_CACHE_LOG_DEBUG("IndexGroup get unique keys from index key:" << start->index_key_ << " to link head ....");

      IndexNode * p = start;
      while (NULL != p) {
        INT32 result = GetKeyNodes(p->key_node_head_, unique_keys);
        if (0 != result) {
          return result;
        }

        if (IndexMemInfo::LINK_TAIL == p->prev_used_group_index_ ||
            IndexMemInfo::LINK_TAIL == p->prev_used_node_index_) {
          break;
        }

        p = Offset2Node(p->prev_used_group_index_, p->prev_used_node_index_);
      }

      return 0;
    }

    INT32 IndexGroup::GetKeysEQ(const string & index_key, KEYS & unique_keys) {
      if (NULL == index_mem_info_) {
        LIB_CACHE_LOG_ERROR("IndexGroup get keys index key:" << index_key << " failed index mem info object is empty .");

        return Err::kERR_INDEX_GROUP_INFO_OBJECT_EMPTY;
      }

      LIB_CACHE_LOG_DEBUG("IndexGroup get unique keys by index key:" << index_key << " EQ ....");

      IndexNodeRBTree::RBTreeNode * rb_node = index_nodes_lists_.Search(index_key);

      if (NULL == rb_node || NULL == rb_node->data_) {
        LIB_CACHE_LOG_WARN("IndexGroup search index key:" << index_key << " is not exists ....");

        return Err::kERR_INDEX_GROUP_INDEX_KEY_IS_NOT_EXISTS;
      }

      if (IndexMemInfo::LINK_TAIL == rb_node->data_->key_node_head_) {
        LIB_CACHE_LOG_WARN("IndexGroup search index key:" << index_key << " is has no unique key link ....");
        return Err::kERR_INDEX_GROUP_INDEX_KEY_HAS_NO_UNIQUE_KEY;
      }

      return GetKeyNodes(rb_node->data_->key_node_head_, unique_keys);
    }

    INT32 IndexGroup::GetKeysNE(const string & index_key, KEYS & unique_keys) {
      if (NULL == index_mem_info_) {
        LIB_CACHE_LOG_ERROR("IndexGroup get keys index key:" << index_key << " failed index mem info object is empty .");

        return Err::kERR_INDEX_GROUP_INFO_OBJECT_EMPTY;
      }

      LIB_CACHE_LOG_DEBUG("IndexGroup get unique keys by index key:" << index_key << " NE ....");

      IndexNodeRBTree::RBTreeNode * rb_node = index_nodes_lists_.Search(index_key);

      if (NULL == rb_node || NULL == rb_node->data_) {
        return GetUniqueKeysFromStart2End(Offset2Node(index_mem_info_->used_group_link_head_,
                                                      index_mem_info_->used_node_link_head_),
                                          unique_keys);
      }

      LIB_CACHE_LOG_DEBUG("IndexGroup get unique keys by node key:" << rb_node->data_->index_key_ << " NE ....");

      if (IndexMemInfo::LINK_TAIL != rb_node->data_->prev_used_group_index_ &&
              IndexMemInfo::LINK_TAIL != rb_node->data_->prev_used_group_index_) {
        GetUniqueKeysFromStart2Head(Offset2Node(rb_node->data_->prev_used_group_index_,
                                                rb_node->data_->prev_used_node_index_),
                                    unique_keys);
      }

      if (IndexMemInfo::LINK_TAIL != rb_node->data_->next_used_group_index_ &&
          IndexMemInfo::LINK_TAIL != rb_node->data_->next_used_group_index_) {
        GetUniqueKeysFromStart2End(Offset2Node(rb_node->data_->next_used_group_index_,
                                                rb_node->data_->next_used_node_index_),
                                    unique_keys);
      }

      LIB_CACHE_LOG_DEBUG("IndexGroup get not eq keys success index key:" << index_key
                                                                          << " unique keys size: "
                                                                          << unique_keys.size() << " ....");

      return 0;
    }

    INT32 IndexGroup::GetKeysGT(const string & index_key, KEYS & unique_keys) {
      if (NULL == index_mem_info_) {
        LIB_CACHE_LOG_ERROR("IndexGroup get keys index key:" << index_key << " failed index mem info object is empty .");

        return Err::kERR_INDEX_GROUP_INFO_OBJECT_EMPTY;
      }

      LIB_CACHE_LOG_DEBUG("IndexGroup get unique keys by index key:" << index_key << " GT ....");

      IndexNodeRBTree::RBTreeNode * rb_node = index_nodes_lists_.upper_bound_first(index_key);

      if (NULL == rb_node || NULL == rb_node->data_) {
        LIB_CACHE_LOG_WARN("IndexGroup get keys gt index key:" << index_key << " has no unique keys .");
        return 0;
      }

      IndexNode * start = rb_node->data_;

      LIB_CACHE_LOG_DEBUG("IndexGroup get unique keys by node key:" << start->index_key_ << " GT ....");

      if (0 == ::strncmp(rb_node->data_->index_key_, index_key.c_str(), index_key.length()) &&
              rb_node->data_->index_key_length_ == index_key.length()) {
        if (IndexMemInfo::LINK_TAIL == rb_node->data_->next_used_group_index_ &&
            IndexMemInfo::LINK_TAIL == rb_node->data_->next_used_group_index_) {
          LIB_CACHE_LOG_WARN("IndexGroup get keys gt index key:" << index_key << " has no unique keys .");
          return 0;
        } else {
          start = Offset2Node(rb_node->data_->next_used_group_index_, rb_node->data_->next_used_node_index_);
        }
      }

      return GetUniqueKeysFromStart2End(start, unique_keys);
    }

    INT32 IndexGroup::GetKeysEGT(const string & index_key, KEYS & unique_keys) {
      if (NULL == index_mem_info_) {
        LIB_CACHE_LOG_ERROR("IndexGroup get keys index key:" << index_key << " failed index mem info object is empty .");

        return Err::kERR_INDEX_GROUP_INFO_OBJECT_EMPTY;
      }

      LIB_CACHE_LOG_DEBUG("IndexGroup get unique keys by index key:" << index_key << " GTE ....");

      IndexNodeRBTree::RBTreeNode * rb_node = index_nodes_lists_.upper_bound_first(index_key);

      if (NULL == rb_node || NULL == rb_node->data_) {
        LIB_CACHE_LOG_WARN("IndexGroup get keys gt index key:" << index_key << " has no unique keys .");
        return 0;
      }

      LIB_CACHE_LOG_DEBUG("IndexGroup get unique keys by node key:" << rb_node->data_->index_key_ << " GTE ....");

      return GetUniqueKeysFromStart2End(rb_node->data_, unique_keys);
    }

    INT32 IndexGroup::GetKeysLT(const string & index_key, KEYS & unique_keys) {
      if (NULL == index_mem_info_) {
        LIB_CACHE_LOG_ERROR("IndexGroup get lt keys index key:" << index_key << " failed index mem info object is empty .");

        return Err::kERR_INDEX_GROUP_INFO_OBJECT_EMPTY;
      }

      LIB_CACHE_LOG_DEBUG("IndexGroup get unique keys by index key:" << index_key << " LT ....");

      IndexNodeRBTree::RBTreeNode * rb_node = index_nodes_lists_.lower_bound_first(index_key);

      if (NULL == rb_node || NULL == rb_node->data_) {
        LIB_CACHE_LOG_WARN("IndexGroup get keys lt index key:" << index_key << " has no unique keys .");
        return 0;
      }

      IndexNode * start = rb_node->data_;

      LIB_CACHE_LOG_DEBUG("IndexGroup get unique keys by index key:" << start->index_key_ << " LT ....");

      if (0 == ::strncmp(rb_node->data_->index_key_, index_key.c_str(), index_key.length()) &&
          rb_node->data_->index_key_length_ == index_key.length()) {
        if (IndexMemInfo::LINK_TAIL == rb_node->data_->prev_used_group_index_ &&
            IndexMemInfo::LINK_TAIL == rb_node->data_->prev_used_node_index_) {
          LIB_CACHE_LOG_WARN("IndexGroup get keys lt index key:" << index_key << " has no unique keys .");
          return 0;
        } else {
          start = Offset2Node(rb_node->data_->prev_used_group_index_, rb_node->data_->prev_used_node_index_);
        }
      }

      LIB_CACHE_LOG_DEBUG("IndexGroup get unique keys by index key:" << start->index_key_ << " LT ....");

      return GetUniqueKeysFromStart2Head(start, unique_keys);
    }

    INT32 IndexGroup::GetKeysELT(const string & index_key, KEYS & unique_keys) {
      if (NULL == index_mem_info_) {
        LIB_CACHE_LOG_ERROR("IndexGroup get lt keys index key:" << index_key << " failed index mem info object is empty .");

        return Err::kERR_INDEX_GROUP_INFO_OBJECT_EMPTY;
      }

      LIB_CACHE_LOG_DEBUG("IndexGroup get unique keys by index key:" << index_key << " LTE ....");

      IndexNodeRBTree::RBTreeNode * rb_node = index_nodes_lists_.lower_bound_first(index_key);

      if (NULL == rb_node || NULL == rb_node->data_) {
        LIB_CACHE_LOG_WARN("IndexGroup get keys lt index key:" << index_key << " has no unique keys .");
        return 0;
      }

      LIB_CACHE_LOG_DEBUG("IndexGroup get unique keys by index key:" << rb_node->data_->index_key_ << " LTE ....");

      return GetUniqueKeysFromStart2Head(rb_node->data_, unique_keys);
    }

    INT32 IndexGroup::GetKeysBE(const string & min_index_key, const string & max_index_key, KEYS & unique_keys) {
      if (NULL == index_mem_info_) {
        LIB_CACHE_LOG_ERROR("IndexGroup get be keys index min key:" << min_index_key
                                                                    << " max key:"
                                                                    << max_index_key
                                                                    << " failed index mem info object is empty .");

        return Err::kERR_INDEX_GROUP_INFO_OBJECT_EMPTY;
      }

      LIB_CACHE_LOG_DEBUG("IndexGroup get unique keys by min index key:" << min_index_key
                                                                         << " max index key:"
                                                                         << max_index_key << " BE ....");

      IndexNodeRBTree::RBTreeNode * rb_node_min = index_nodes_lists_.upper_bound_first(min_index_key);
      if (NULL == rb_node_min || NULL == rb_node_min->data_) {
        LIB_CACHE_LOG_WARN("IndexGroup get keys be index min key:" << min_index_key
                                                                   << " max key:"
                                                                   << max_index_key
                                                                   <<" has no unique keys .");
        return 0;
      }

      IndexNodeRBTree::RBTreeNode * rb_node_max = index_nodes_lists_.lower_bound_first(max_index_key);
      if (NULL == rb_node_max || NULL == rb_node_max->data_) {
        LIB_CACHE_LOG_WARN("IndexGroup get keys be index min key:" << min_index_key
                                                                   << " max key:"
                                                                   << max_index_key
                                                                   <<" has no unique keys .");
        return 0;
      }

      LIB_CACHE_LOG_DEBUG("IndexGroup get unique keys by min key:" << rb_node_min->data_->index_key_
                                                                   << " max key:" << rb_node_max->data_->index_key_
                                                                   << " BE ....");

      return GetUniqueKeysFromStart2Node(rb_node_min->data_, rb_node_max->data_, unique_keys);
    }

    INT32 IndexGroup::GetKeysIN(const KEYS & index_keys, KEYS & unique_keys) {
      LIB_CACHE_LOG_DEBUG("IndexGroup get unique keys by index key size:" << index_keys.size() << " in .......");
      INT32 size = index_keys.size();
      for (INT32 i = 0; i < size; ++i) {
        INT32 result = GetKeysEQ(index_keys[i], unique_keys);
        if (0 != result) {
          LIB_CACHE_LOG_ERROR("IndexGroup get in keys index key:" << index_keys[i]
                                                                  << " result:" << result
                                                                  << " has no unique keys .");

          return result;
        }
      }

      LIB_CACHE_LOG_DEBUG("IndexGroup get in keys success unique keys size: " << unique_keys.size() << " ....");

      return 0;
    }

    INT32 IndexGroup::GetKeysNotIN(const KEYS & index_keys, KEYS & unique_keys) {
      LIB_CACHE_LOG_DEBUG("IndexGroup get unique keys by index key size:" << index_keys.size() << " not in .......");
      if (IndexMemInfo::LINK_TAIL == index_mem_info_->used_group_link_head_ ||
              IndexMemInfo::LINK_TAIL == index_mem_info_->used_node_link_head_) {
        LIB_CACHE_LOG_DEBUG("IndexGroup get not in keys index key has no unique keys .");
        return 0;
      }

      IndexNode * p = Offset2Node(index_mem_info_->used_group_link_head_, index_mem_info_->used_node_link_head_);
      while (NULL != p) {
        KEYS::const_iterator c_it = find(index_keys.cbegin(), index_keys.cend(), p->index_key_);
        if (c_it == index_keys.cend()) {
          LIB_CACHE_LOG_DEBUG("IndexGroup get not in keys index key:" << p->index_key_ << " ....");
          INT32 result = GetKeyNodes(p->key_node_head_, unique_keys);
          if (0 != result) {
            LIB_CACHE_LOG_ERROR("IndexGroup get not in keys index key:" << p->index_key_ << " failed .");
            return result;
          }
        }

        if (IndexMemInfo::LINK_TAIL == p->next_used_group_index_ ||
            IndexMemInfo::LINK_TAIL == p->next_used_node_index_) {
          break;
        }

        p = Offset2Node(p->next_used_group_index_, p->next_used_node_index_);
      }

      LIB_CACHE_LOG_DEBUG("IndexGroup get not in keys success unique keys size: " << unique_keys.size() << " ....");

      return 0;
    }

    VOID IndexGroup::Dump() {
      LIB_CACHE_LOG_DEBUG("Dump all nodes ***********************************************************************");
      DumpHead();

      INT32 group_index = index_mem_info_->used_group_link_head_;
      INT32 node_index = index_mem_info_->used_node_link_head_;
      while (IndexMemInfo::LINK_TAIL != group_index && IndexMemInfo::LINK_TAIL != node_index) {
        IndexNode * p = Offset2Node(group_index, node_index);
        DumpIndexNode(p);
        group_index = p->next_used_group_index_;
        node_index = p->next_used_node_index_;
      }

      DumpTree();
    }

    VOID IndexGroup::DumpHead() {
      LIB_CACHE_LOG_DEBUG("Index Group Head B ***********************************************************************");
      LIB_CACHE_LOG_DEBUG("flag:" << index_mem_info_->flag_);
      LIB_CACHE_LOG_DEBUG("total group count:" << index_mem_info_->total_group_count_);
      LIB_CACHE_LOG_DEBUG("used group link head:" << index_mem_info_->used_group_link_head_);
      LIB_CACHE_LOG_DEBUG("used node link head:" << index_mem_info_->used_node_link_head_);
      LIB_CACHE_LOG_DEBUG("free group link head:" << index_mem_info_->free_group_link_head_);
      LIB_CACHE_LOG_DEBUG("free node link head:" << index_mem_info_->free_node_link_head_);
      LIB_CACHE_LOG_DEBUG("key node group head head:" << index_mem_info_->key_nodes_group_head_offset_);
      LIB_CACHE_LOG_DEBUG("key node group tail head:" << index_mem_info_->key_nodes_group_tail_offset_);
      LIB_CACHE_LOG_DEBUG("free key node head:" << index_mem_info_->free_key_node_head_);
      LIB_CACHE_LOG_DEBUG("Index Group Head E ***********************************************************************");
    }

    VOID IndexGroup::DumpIndexNode(IndexNode * node) {
      if (NULL == node) {
        return;
      }

      LIB_CACHE_LOG_DEBUG("Node info B *****************************************************************************");
      LIB_CACHE_LOG_DEBUG("key:" << node->index_key_);
      LIB_CACHE_LOG_DEBUG("key length:" << node->index_key_length_);
      LIB_CACHE_LOG_DEBUG("key node head:" << node->key_node_head_);
      LIB_CACHE_LOG_DEBUG("prev used group index:" << node->prev_used_group_index_);
      LIB_CACHE_LOG_DEBUG("prev used node index:" << node->prev_used_node_index_);
      LIB_CACHE_LOG_DEBUG("next used group index:" << node->next_used_group_index_);
      LIB_CACHE_LOG_DEBUG("next used node index:" << node->next_used_node_index_);
      LIB_CACHE_LOG_DEBUG("next free group index:" << node->next_free_group_index_);
      LIB_CACHE_LOG_DEBUG("next free node index:" << node->next_free_node_index_);

      INT64 key_node_offset = node->key_node_head_;
      while (IndexMemInfo::LINK_TAIL != key_node_offset) {
        KeyNode * key_node = Offset2KeyNode(key_node_offset);
        DumpKeyNode(key_node);
        key_node_offset = key_node->next_node_;
      }

      LIB_CACHE_LOG_DEBUG("Node info E *****************************************************************************");
    }

    VOID IndexGroup::DumpKeyNode(KeyNode * node) {
      if (NULL == node) {
        return;
      }

      LIB_CACHE_LOG_DEBUG("Key Node info B *****************************************************************************");
      LIB_CACHE_LOG_DEBUG("key:" << node->key_);
      LIB_CACHE_LOG_DEBUG("key length:" << node->key_length_);
      LIB_CACHE_LOG_DEBUG("prev node:" << node->prev_node_);
      LIB_CACHE_LOG_DEBUG("next node:" << node->next_node_);
      LIB_CACHE_LOG_DEBUG("Key Node info E *****************************************************************************");
    }

    VOID IndexGroup::PrintTreeNode(IndexNodeRBTree::RBTreeNode * node, VOID * data) {
      if (NULL == node) {
        return;
      }

      LIB_CACHE_LOG_DEBUG("####################################################");
      LIB_CACHE_LOG_DEBUG("key:" << node->key_);
      LIB_CACHE_LOG_DEBUG("color:" << node->color_);
      if (NULL != node->parent_) {
        LIB_CACHE_LOG_DEBUG("parent:" << node->parent_->key_);
      }
      if (NULL != node->left_) {
        LIB_CACHE_LOG_DEBUG("left:" << node->left_->key_);
      }
      if (NULL != node->right_) {
        LIB_CACHE_LOG_DEBUG("right:" << node->right_->key_);
      }
      LIB_CACHE_LOG_DEBUG("####################################################");
    }

    VOID IndexGroup::DumpTree() {
      index_nodes_lists_.TravelMid(PrintTreeNode, NULL);
    }

    VOID IndexGroup::Destroy() {
      if (pt_malloc_) {
        delete pt_malloc_;
        pt_malloc_ = NULL;
      }
    }
  }  // namespace cache
}  // namespace lib
