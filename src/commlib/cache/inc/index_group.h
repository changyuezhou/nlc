// Copyright (c) 2013 zhou chang yue. All rights reserved.

#ifndef COMMLIB_CACHE_INC_INDEX_GROUP_H_
#define COMMLIB_CACHE_INC_INDEX_GROUP_H_

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <map>
#include <vector>
#include <utility>
#include <algorithm>
#include "commlib/public/inc/type.h"
#include "commlib/cache/inc/ptmalloc.h"
#include "commlib/cache/inc/node_group.h"
#include "commlib/cache/inc/err.h"
#include "commlib/magic/src/rbtree.cc"

namespace lib {
  namespace cache {
    using std::string;
    using std::map;
    using std::vector;
    using std::find;
    using lib::magic::RBTree;

    typedef string KEY;
    typedef vector<string> KEYS;

    typedef struct _index_mem_info {
      static const INT32 MAX_GROUP_COUNT = 3; //65535;
      static const INT32 NODES_PER_GROUP = 2;
      static const INT32 KEY_NODES_PER_GROUP = 2;
      static const INT32 KEY_MAX_LENGTH = 712;
      static const INT64 LINK_TAIL = -1;
      static const INT64 LINK_HEAD = -1;
      static const INT64 INDEX_GROUP_NOT_INITIAL = 0;
      static const INT64 INDEX_GROUP_USABLE = 0xA5A5A5A5A5A5A5A5;

      typedef struct _key_node {
        CHAR key_[KEY_MAX_LENGTH];
        UINT32 key_length_;
        INT64 prev_node_;
        INT64 next_node_;
      } KeyNode;

      typedef struct _key_node_group {
        INT64 flag_;
        KeyNode key_nodes_[KEY_NODES_PER_GROUP];
        INT64 next_group_offset_;
        pthread_mutex_t mutex_;
      } KeyNodeGroup;

      typedef struct _index_node {
        CHAR  index_key_[KEY_MAX_LENGTH];
        UINT32 index_key_length_;
        INT64 key_node_head_;
        INT32 prev_used_group_index_;
        INT32 prev_used_node_index_;
        INT32 next_used_group_index_;
        INT32 next_used_node_index_;
        INT32 next_free_group_index_;
        INT32 next_free_node_index_;
        pthread_mutex_t mutex_;
      } IndexNode;

      typedef struct _index_group {
        INT64 flag_;
        IndexNode nodes_[NODES_PER_GROUP];
        pthread_mutex_t mutex_;
      } IndexGroup;

      INT64 flag_;
      pthread_mutex_t mutex_;
      INT64 index_group_offset_[MAX_GROUP_COUNT];
      INT64 total_group_count_;
      INT32 used_group_link_head_;
      INT32 used_node_link_head_;
      INT32 free_group_link_head_;
      INT32 free_node_link_head_;
      INT64 key_nodes_group_head_offset_;
      INT64 key_nodes_group_tail_offset_;
      INT64 free_key_node_head_;
    } IndexMemInfo;

    class IndexGroup {
     public:
       typedef IndexMemInfo::IndexNode IndexNode;
       typedef IndexMemInfo::IndexGroup IndexGroupMem;
       typedef IndexMemInfo::KeyNodeGroup KeyNodeGroup;
       typedef IndexMemInfo::KeyNode KeyNode;
       typedef RBTree<IndexNode> IndexNodeRBTree;

     public:
       IndexGroup():pt_malloc_(NULL), index_mem_info_(NULL) {}
       explicit IndexGroup(IndexMemInfo * index_mem_info):pt_malloc_(NULL), index_mem_info_(index_mem_info) {}
       ~IndexGroup() { Destroy(); }

     public:
       INT32 Initial(ChunkInfo * chunk_info);
       INT32 Initial(IndexMemInfo * index_mem_info, ChunkInfo * chunk_info);

     public:
       INT32 Insert(const string & index_key, const string & unique_key);
       INT32 Insert(const string & index_key, const KEYS & unique_keys);
       INT32 Delete(const string & index_key, const string & unique_key);
       INT32 Delete(const string & index_key, const KEYS & unique_keys);

     protected:
       BOOL IsExists(const string & index_key, const string & unique_key);

     public:
       INT32 GetKeysEQ(const string & index_key, KEYS & unique_keys);
       INT32 GetKeysNE(const string & index_key, KEYS & unique_keys);
       INT32 GetKeysGT(const string & index_key, KEYS & unique_keys);
       INT32 GetKeysEGT(const string & index_key, KEYS & unique_keys);
       INT32 GetKeysLT(const string & index_key, KEYS & unique_keys);
       INT32 GetKeysELT(const string & index_key, KEYS & unique_keys);
       INT32 GetKeysBE(const string & min_index_key, const string & max_index_key, KEYS & unique_keys);
       INT32 GetKeysIN(const KEYS & index_keys, KEYS & unique_keys);
       INT32 GetKeysNotIN(const KEYS & index_keys, KEYS & unique_keys);

     protected:
       INT32 GetUniqueKeysFromStart2End(IndexNode * start, KEYS & unique_keys);
       INT32 GetUniqueKeysFromStart2Head(IndexNode * start, KEYS & unique_keys);
       INT32 GetUniqueKeysFromStart2Node(IndexNode * start, IndexNode * end, KEYS & unique_keys);

     protected:
       IndexNode * Offset2Node(INT32 group_offset_index, INT32 node_index);
       IndexNode * Offset2Node(INT64 group_offset, INT32 node_index);
       IndexGroupMem * Offset2Group(INT64 group_offset);
       KeyNodeGroup * Offset2KeyNodeGroup(INT64 offset);
       KeyNode * Offset2KeyNode(INT64 offset);
       INT64 CharPoint2Offset(CHAR * address);
       INT32 GetIndexNodeGroupIndex(INT64 offset);
       INT32 GetIndexNodeIndex(INT64 offset);
       INT64 GetOffsetFromIndex(INT32 group, INT32 node);

     protected:
       INT32 AllocateIndexGroup();
       INT32 InitialIndexGroup(IndexGroupMem * index_group);
       INT64 GetFreeIndexNodeOffset();
       INT32 InsertIndexNode2LinkList(INT64 position, INT64 node);
       INT64 DelIndexNodeFromLinkList(IndexNode * node);
       INT32 ClearNode(IndexNode * node);
       VOID SetNode(IndexNode * node, const string & index_key);

       INT32 AllocateKeyNodeGroup();
       INT32 InitialKeyNodeGroup(KeyNodeGroup * key_node_group);
       KeyNode * GetFreeKeyNode();
       INT64 DelKeyNode(INT64 * head, KeyNode * key_node);
       INT64 InsertKeyNodeToLink(INT64 * head, INT64 key_node);
       KeyNode * GetKeyNode(INT64 head, const string & unique_key);
       BOOL IsKeyNodeInLink(INT64 head, const string & unique_key);
       INT32 GetKeyNodes(INT64 head, KEYS & unique_keys);

       INT32 InitialLock(pthread_mutex_t * mutex);

     protected:
       IndexNode * GetInsertPosition(const string & key);
       INT32 InsertNode2Tree(IndexNode * node);

     protected:
       INT32 InitialIndexGroupInfo();
       INT32 LoadIndexNode2Tree();

     protected:
       VOID Destroy();

     public:
       VOID Dump();
       VOID DumpHead();
       VOID DumpTree();
       VOID DumpIndexNode(IndexNode * node);
       VOID DumpKeyNode(KeyNode * node);
       static VOID PrintTreeNode(IndexNodeRBTree::RBTreeNode * node, VOID * data);

     private:
       IndexGroup(const IndexGroup &);
       IndexGroup & operator=(const IndexGroup &);

     private:
       PTMalloc * pt_malloc_;
       IndexMemInfo * index_mem_info_;
       IndexNodeRBTree  index_nodes_lists_;
    };
  }  // namespace cache
}  // namespace lib

#endif  // COMMLIB_CACHE_INC_INDEX_GROUP_H_