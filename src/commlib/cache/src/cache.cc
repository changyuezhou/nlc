// Copyright (c) 2013 zhou chang yue. All rights reserved.

#include <string.h>
#include "commlib/cache/inc/cache.h"
#include "commlib/cache/inc/log.h"
#include "commlib/cache/inc/err.h"

namespace lib {
  namespace cache {
    INT32 Cache::Initial(INT64 size, VOID * buffer) {
      if (NULL == buffer) {
        buffer = reinterpret_cast<CHAR *>(::malloc(size));
        if (NULL == buffer) {
          LIB_CACHE_LOG_ERROR("cache memory malloc failed size:" << size);

          return Err::kERR_NODE_GROUP_ALLOCATE_BUFFER_FAILED;
        }

        memset(buffer, 0x00, size);
        
        need_free_ = TRUE;    
      }

      buffer_ = buffer;

      return Initial(reinterpret_cast<NodeMemInfo *>(buffer), size);
    }

    INT32 Cache::Initial(NodeMemInfo * node_mem_info, ChunkInfo * chunk_info) {
      if (NULL == node_mem_info || NULL == chunk_info) {
        LIB_CACHE_LOG_ERROR("node_mem_info or chunk_info is empty .................");
        return Err::kERR_NODE_GROUP_INFO_OBJECT_EMPTY;
      }

      node_group_ = new NodeGroup(node_mem_info);
      if (NULL == node_group_) {
        LIB_CACHE_LOG_ERROR("cache allocate node group failed .................");
        return Err::kERR_CACHE_ALLOCATE_NODE_GROUP_FAILED;
      }

      return node_group_->LoadNode(chunk_info);
    }

    INT32 Cache::Initial(NodeMemInfo * node_mem_info, INT64 total_size) {
      ChunkInfo * chunk_info = reinterpret_cast<ChunkInfo *>(reinterpret_cast<CHAR *>(node_mem_info) + sizeof(NodeMemInfo));
      if (NodeMemInfo::NODE_GROUP_USABLE != chunk_info->flag_) {
        pthread_mutexattr_t attr;
        ::pthread_mutexattr_init(&attr);
        if (0 != ::pthread_mutex_init(&chunk_info->mutex_, &attr)) {
          return Err::kERR_NODE_GROUP_INITIAL_MUTEX_FAILED;
        }
        ::pthread_mutexattr_destroy(&attr);
        chunk_info->bottom_ = sizeof(NodeMemInfo) + sizeof(ChunkInfo);
        chunk_info->chunk_start_ = chunk_info->bottom_;
        chunk_info->top_ = total_size;
        chunk_info->size_ = chunk_info->top_ - chunk_info->bottom_;

        chunk_info->flag_ = NodeMemInfo::NODE_GROUP_USABLE;
      }

      return Initial(node_mem_info, chunk_info);
    }

    INT32 Cache::Update(const KEY & key) {
      if (NULL == node_group_) {
        LIB_CACHE_LOG_ERROR("cache node group is empty .................");
        return Err::kERR_NODE_GROUP_INFO_OBJECT_EMPTY;
      }

      return node_group_->Touch(key);
    }

    INT32 Cache::Delete(const KEYS & keys) {
      for (KEYS::const_iterator c_it = keys.cbegin(); c_it != keys.cend(); ++c_it) {
        INT32 result = Delete(*c_it);
        if (0 != result) {
          LIB_CACHE_LOG_WARN("cache node group delete key:" << *c_it << " failed .................");
        }
      }

      return 0;
    }

    INT32 Cache::Update(const KEYS & keys) {
      for (KEYS::const_iterator c_it = keys.cbegin(); c_it != keys.cend(); ++c_it) {
        INT32 result = Update(*c_it);
        if (0 != result) {
          LIB_CACHE_LOG_WARN("cache node group update key:" << *c_it << " failed .................");
        }
      }

      return 0;
    }

    INT32 Cache::Delete(const KEY & key) {
      if (NULL == node_group_) {
        LIB_CACHE_LOG_ERROR("cache node group is empty .................");
        return Err::kERR_NODE_GROUP_INFO_OBJECT_EMPTY;
      }

      return node_group_->Del(key);
    }

    INT32 Cache::Get(const KEY & key, CHAR * data, INT32 * max_size) {
      if (NULL == node_group_) {
        LIB_CACHE_LOG_ERROR("cache node group is empty .................");
        return Err::kERR_NODE_GROUP_INFO_OBJECT_EMPTY;
      }

      return node_group_->Get(key, data, max_size);
    }

    INT32 Cache::Status(const KEY & key) {
      if (NULL == node_group_) {
        LIB_CACHE_LOG_ERROR("cache node group is empty .................");
        return Err::kERR_NODE_GROUP_INFO_OBJECT_EMPTY;
      }

      return node_group_->Status(key);
    }

    INT32 Cache::Set(const KEY & key, const CHAR * data, INT32 size) {
      if (NULL == node_group_) {
        LIB_CACHE_LOG_ERROR("cache node group is empty .................");
        return Err::kERR_NODE_GROUP_INFO_OBJECT_EMPTY;
      }

      if (0 >= size) {
        return node_group_->Set(key);
      }

      return node_group_->Set(key, data, size);
    }

    UINT64 Cache::LastAccessTimestamp() {
      if (NULL == node_group_) {
        return 0;
      }

      return node_group_->LastAccessTimestamp();
    }

    VOID Cache::Dump() {
      if (node_group_) {
        node_group_->Dump();
      }
    }

    VOID Cache::Destroy() {
      if (node_group_) {
        delete node_group_;
        node_group_ = NULL;
      }
      if (need_free_ && (buffer_ != NULL)) {
        ::free(buffer_);
        buffer_ = NULL;
      }
    }
  }  // namespace cache
}  // namespace lib
