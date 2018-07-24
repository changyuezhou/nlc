// Copyright (c) 2013 zhou chang yue. All rights reserved.

#ifndef COMMLIB_CACHE_INC_NODE_GROUP_H_
#define COMMLIB_CACHE_INC_NODE_GROUP_H_

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <map>
#include <vector>
#include <utility>
#include "commlib/public/inc/type.h"
#include "commlib/thread/inc/thread.h"
#include "commlib/cache/inc/ptmalloc.h"
#include "commlib/cache/inc/err.h"
#include "commlib/magic/inc/mutex.h"
#include "commlib/magic/inc/atomic.h"
#include "commlib/magic/inc/scopeLock.h"

namespace lib {
  namespace cache {
    using std::string;
    using std::map;
    using std::vector;
    using lib::thread::Thread;
    using lib::magic::Mutex;
    using lib::magic::ScopeLock;

    typedef struct _node_mem_info {
      static const INT32 MAX_GROUP_COUNT = 65535; //65535;
      static const INT32 HASH_BUCKET_LINK_LENGTH = 4;
      static const INT32 NODES_PER_GROUP = 256;
      static const INT32 KEY_MAX_LENGTH = 256;
      static const INT64 LINK_TAIL = -1;
      static const INT64 NODE_GROUP_NOT_INITIAL = 0;
      static const INT64 NODE_GROUP_USABLE = 0xA5A5A5A5A5A5A5A5;

      typedef struct _node {
        CHAR  key_[KEY_MAX_LENGTH];
        INT64 key_length_;
        INT64 flag_;
        INT64 data_offset_;
        INT64 data_size_;
        INT64 data_capacity_;
        INT64 last_access_timestamp_;
        INT32 prev_used_group_index_;
        INT32 prev_used_node_index_;
        INT32 next_used_group_index_;
        INT32 next_used_node_index_;
        INT32 next_free_group_index_;
        INT32 next_free_node_index_;
        pthread_mutex_t mutex_;
      } Node;

      typedef struct _node_group {
        INT64 flag_;
        Node  node_list_[NODES_PER_GROUP];
        INT64 next_group_offset_;  //  0 tail
        pthread_mutex_t mutex_;
      } NodeGroup;

      INT64 flag_;
      pthread_mutex_t mutex_;
      INT64 node_group_offset_[MAX_GROUP_COUNT];
      INT64 total_group_count_;
      INT32 used_group_link_head_;
      INT32 used_node_link_head_;
      INT32 free_group_link_head_;
      INT32 free_node_link_head_;
      atomic_t total_keys_;
    } NodeMemInfo;

    typedef struct _hash_node {
      NodeMemInfo::Node * node_;
      _hash_node * hash_next_;
      _hash_node * lru_prev_;
      _hash_node * lru_next_;

      _hash_node() {
        node_ = NULL;
        hash_next_ = NULL;
        lru_prev_ = NULL;
        lru_next_ = NULL;
      }
    } HashNode;

    class NodeGroup: public Thread {
     public:
       enum NODE_FLAG {
         FREE  = 1,
         CLEAN = 2,
         DIRTY = 3,
         KEY_NOT_EXISTS = 4,
         IN_CACHE = 5
       };

     public:
       typedef NodeMemInfo::NodeGroup NodeGroupMem;
       typedef map<string, INT32> KeyList;

     public:
       NodeGroup(NodeMemInfo * node_mem_info): pt_malloc_(NULL),
                                               node_mem_info_(node_mem_info),
                                               keys_hash_(((2<<16)-1)),
                                               hash_bucket_(NULL),
                                               lru_node_head_(NULL) {}
       ~NodeGroup() { Destroy(); }

     public:
       INT32 LoadNode(ChunkInfo * chunk_info);

     public:
       INT32 Set(const string & key, const CHAR * data, INT32 size);
       INT32 Set(const string & key);
       INT32 Get(const string & key, CHAR * data, INT32 * max_size);
       INT32 Del(const string & key);
       INT32 Touch(const string & key);

     public:
       UINT64 LastAccessTimestamp();
       UINT32 TotalKeys() { return node_mem_info_->total_keys_.counter; }
       INT32 Status(const string & key);

     public:
       virtual INT32 Working(VOID * parameter);

     protected:
       INT32 FreeExpiredNode(INT32 size);

     protected:
       NodeMemInfo::Node * Offset2Node(INT32 group_offset_index, INT32 node_index);
       NodeMemInfo::Node * Offset2Node(INT64 group_offset, INT32 node_index);
       NodeGroupMem * Offset2Group(INT64 group_offset);
       CHAR * Offset2DataPoint(INT64 offset);
       INT64 CharPoint2Offset(CHAR * address);

     protected:
       INT32 AddNode2Hash(NodeMemInfo::Node * node);
       INT32 AddNode2Hash(HashNode * hash_node);
       INT32 AddNode2LRU(HashNode * hash_node);
       INT32 DelNodeFromLRU(HashNode * hash_node);
       INT32 DelNodeFromBucket(HashNode * hash_node);
       INT32 UpdateNode2LRU(HashNode * hash_node);
       HashNode * GetHashNodeFromHash(const string & key);
       BOOL IsInHashBucket(const string & key);

     protected:
       NodeMemInfo::Node * GetFreeNode();
       INT32 RecycleNode(NodeMemInfo::Node * node);
       INT32 ClearNode(NodeMemInfo::Node * node);
       INT32 AllocateNodeGroup();
       INT32 InitialNodeGroup(NodeGroupMem * node_group);
       INT32 InitialLock(pthread_mutex_t * mutex);

     protected:
       INT32 InitialNodeGroupInfo();
       INT32 LoadNodeFromUsedList();

     protected:
       VOID Destroy();

     protected:
       UINT64 DefHash(const string & id);

     public:
       VOID Dump();

     protected:
       VOID DumpHashBucket();
       VOID DumpLRULink();
       VOID DumpHead();
       VOID DumpNode(const NodeMemInfo::Node * node);

     private:
       NodeGroup(const NodeGroup &);
       NodeGroup & operator=(const NodeGroup &);

     private:
       PTMalloc * pt_malloc_;
       NodeMemInfo * node_mem_info_;
       INT32 keys_hash_;
       HashNode ** hash_bucket_;
       HashNode * lru_node_head_;
       KeyList key_list_;
       Mutex mutex_;
    };
  }  // namespace cache
}  // namespace lib

#endif  // COMMLIB_CACHE_INC_NODE_GROUP_H_