// Copyright (c) 2013 zhou chang yue. All rights reserved.

#include <string.h>
#include "commlib/cache/inc/index_cache.h"
#include "commlib/cache/inc/log.h"
#include "commlib/cache/inc/err.h"

namespace lib {
  namespace cache {
    INT32 IndexCache::Initial(INT64 size, VOID * buffer) {
      if (NULL == buffer) {
        buffer = reinterpret_cast<CHAR *>(::malloc(size));
        if (NULL == buffer) {
          LIB_CACHE_LOG_ERROR("index cache memory malloc failed size:" << size);

          return Err::kERR_INDEX_GROUP_ALLOCATE_BUFFER_FAILED;
        }

        memset(buffer, 0x00, size);
        
        need_free_ = TRUE;    
      }

      buffer_ = buffer;

      return Initial(reinterpret_cast<IndexMemInfo *>(buffer), size);
    }

    INT32 IndexCache::Initial(IndexMemInfo * index_mem_info, INT64 total_size) {
      ChunkInfo * chunk_info = reinterpret_cast<ChunkInfo *>(reinterpret_cast<CHAR *>(index_mem_info) + sizeof(IndexMemInfo));
      if (NodeMemInfo::NODE_GROUP_USABLE != chunk_info->flag_) {
        pthread_mutexattr_t attr;
        ::pthread_mutexattr_init(&attr);
        if (0 != ::pthread_mutex_init(&chunk_info->mutex_, &attr)) {
          return Err::kERR_INDEX_GROUP_INITIAL_MUTEX_FAILED;
        }
        ::pthread_mutexattr_destroy(&attr);
        chunk_info->bottom_ = sizeof(IndexMemInfo) + sizeof(ChunkInfo);
        chunk_info->chunk_start_ = chunk_info->bottom_;
        chunk_info->top_ = total_size;
        chunk_info->size_ = chunk_info->top_ - chunk_info->bottom_;

        chunk_info->flag_ = NodeMemInfo::NODE_GROUP_USABLE;
      }

      return Initial(index_mem_info, chunk_info);
    }

    INT32 IndexCache::Initial(IndexMemInfo * index_mem_info, ChunkInfo * chunk_info) {
      if (NULL == index_mem_info || NULL == chunk_info) {
        LIB_CACHE_LOG_ERROR("index_mem_info or chunk_info is empty .................");
        return Err::kERR_INDEX_GROUP_INFO_OBJECT_EMPTY;
      }

      index_group_ = new IndexGroup(index_mem_info);
      if (NULL == index_group_) {
        LIB_CACHE_LOG_ERROR("index cache allocate node group failed .................");
        return Err::kERR_INDEX_CACHE_ALLOCATE_INDEX_GROUP_FAILED;
      }

      return index_group_->Initial(chunk_info);
    }  

    INT32 IndexCache::Insert(const string & index_key, const string & unique_key) {
      if (NULL == index_group_) {
        LIB_CACHE_LOG_ERROR("index cache node group is empty .................");
        return Err::kERR_INDEX_GROUP_INFO_OBJECT_EMPTY;
      }

      return index_group_->Insert(index_key, unique_key);      
    }

    INT32 IndexCache::Insert(const string & index_key, const KEYS & unique_keys) {
      if (NULL == index_group_) {
        LIB_CACHE_LOG_ERROR("index cache node group is empty .................");
        return Err::kERR_INDEX_GROUP_INFO_OBJECT_EMPTY;
      }

      return index_group_->Insert(index_key, unique_keys);  
    }

    INT32 IndexCache::Delete(const string & index_key, const string & unique_key) {
      if (NULL == index_group_) {
        LIB_CACHE_LOG_ERROR("index cache node group is empty .................");
        return Err::kERR_INDEX_GROUP_INFO_OBJECT_EMPTY;
      }

      return index_group_->Delete(index_key, unique_key);  
    }

    INT32 IndexCache::Delete(const string & index_key, const KEYS & unique_keys) {
      if (NULL == index_group_) {
        LIB_CACHE_LOG_ERROR("index cache node group is empty .................");
        return Err::kERR_INDEX_GROUP_INFO_OBJECT_EMPTY;
      }

      return index_group_->Delete(index_key, unique_keys);  
    }

    INT32 IndexCache::GetKeysEQ(const string & index_key, KEYS & unique_keys) {
      if (NULL == index_group_) {
        LIB_CACHE_LOG_ERROR("index cache node group is empty .................");
        return Err::kERR_INDEX_GROUP_INFO_OBJECT_EMPTY;
      }

      return index_group_->GetKeysEQ(index_key, unique_keys);
    }

    INT32 IndexCache::GetKeysNE(const string & index_key, KEYS & unique_keys) {
      if (NULL == index_group_) {
        LIB_CACHE_LOG_ERROR("index cache node group is empty .................");
        return Err::kERR_INDEX_GROUP_INFO_OBJECT_EMPTY;
      }

      return index_group_->GetKeysNE(index_key, unique_keys);
    }

    INT32 IndexCache::GetKeysGT(const string & index_key, KEYS & unique_keys) {
      if (NULL == index_group_) {
        LIB_CACHE_LOG_ERROR("index cache node group is empty .................");
        return Err::kERR_INDEX_GROUP_INFO_OBJECT_EMPTY;
      }

      return index_group_->GetKeysGT(index_key, unique_keys);
    }

    INT32 IndexCache::GetKeysEGT(const string & index_key, KEYS & unique_keys) {
      if (NULL == index_group_) {
        LIB_CACHE_LOG_ERROR("index cache node group is empty .................");
        return Err::kERR_INDEX_GROUP_INFO_OBJECT_EMPTY;
      }

      return index_group_->GetKeysEGT(index_key, unique_keys);
    }

    INT32 IndexCache::GetKeysLT(const string & index_key, KEYS & unique_keys) {
      if (NULL == index_group_) {
        LIB_CACHE_LOG_ERROR("index cache node group is empty .................");
        return Err::kERR_INDEX_GROUP_INFO_OBJECT_EMPTY;
      }

      return index_group_->GetKeysLT(index_key, unique_keys);
    }

    INT32 IndexCache::GetKeysELT(const string & index_key, KEYS & unique_keys) {
      if (NULL == index_group_) {
        LIB_CACHE_LOG_ERROR("index cache node group is empty .................");
        return Err::kERR_INDEX_GROUP_INFO_OBJECT_EMPTY;
      }

      return index_group_->GetKeysELT(index_key, unique_keys);
    }

    INT32 IndexCache::GetKeysBE(const string & min_index_key, const string & max_index_key, KEYS & unique_keys) {
      if (NULL == index_group_) {
        LIB_CACHE_LOG_ERROR("index cache node group is empty .................");
        return Err::kERR_INDEX_GROUP_INFO_OBJECT_EMPTY;
      }

      return index_group_->GetKeysBE(min_index_key, max_index_key, unique_keys);
    }

    INT32 IndexCache::GetKeysIN(const KEYS & index_keys, KEYS & unique_keys) {
      if (NULL == index_group_) {
        LIB_CACHE_LOG_ERROR("index cache node group is empty .................");
        return Err::kERR_INDEX_GROUP_INFO_OBJECT_EMPTY;
      }

      return index_group_->GetKeysIN(index_keys, unique_keys);
    }

    INT32 IndexCache::GetKeysNotIN(const KEYS & index_keys, KEYS & unique_keys) {
      if (NULL == index_group_) {
        LIB_CACHE_LOG_ERROR("index cache node group is empty .................");
        return Err::kERR_INDEX_GROUP_INFO_OBJECT_EMPTY;
      }

      return index_group_->GetKeysNotIN(index_keys, unique_keys);
    }

    VOID IndexCache::Dump() {
      if (index_group_) {
        index_group_->Dump();
      }      
    }

    VOID IndexCache::Destroy() {
      if (index_group_) {
        delete index_group_;
        index_group_ = NULL;
      }
      if (need_free_ && (buffer_ != NULL)) {
        ::free(buffer_);
        buffer_ = NULL;
      }        
    }
  }  // namespace cache
}  // namespace lib    