// Copyright (c) 2013 zhou chang yue. All rights reserved.

#include <string.h>
#include "commlib/cache/inc/ptmalloc.h"
#include "commlib/cache/inc/err.h"
#include "commlib/cache/inc/log.h"
#include "commlib/magic/inc/mutex.h"
#include "commlib/magic/inc/scopeLock.h"

namespace lib {
  namespace cache {
    using lib::magic::Mutex;
    using lib::magic::ScopeLock;

    VOID * PTMalloc::malloc(INT32 size) {
      Mutex mutex(&chunk_info_->mutex_);
      ScopeLock<Mutex> scope(&mutex);

      INT32 chunk_size = sizeof(Chunk) + size;
      if (ChunkInfo::LARGE_BINS_MAX_BYTE >= chunk_size) {
        chunk_size = (chunk_size % ChunkInfo::SORT_BINS_STEP) ? ((chunk_size / ChunkInfo::SORT_BINS_STEP) + 1) * ChunkInfo::SORT_BINS_STEP
                                                  : chunk_size;
      } else if (ChunkInfo::MID_LARGE_BINS_MAX_BYTE >= chunk_size) {
        chunk_size -= ChunkInfo::MID_LARGE_BINS_MIN_BYTE;
        chunk_size = (chunk_size % ChunkInfo::MID_LARGE_BINS_STEP) ?
               ((chunk_size / ChunkInfo::MID_LARGE_BINS_STEP) + 1) * ChunkInfo::MID_LARGE_BINS_STEP : chunk_size;
        chunk_size += ChunkInfo::MID_LARGE_BINS_MIN_BYTE;
      } else {
        chunk_size = (chunk_size % ChunkInfo::SORT_BINS_STEP) ? ((chunk_size / ChunkInfo::SORT_BINS_STEP) + 1) * ChunkInfo::SORT_BINS_STEP
                                                  : chunk_size;
      }
      LIB_CACHE_LOG_DEBUG("ptmalloc malloc chunk size:" << size << " chunk size:" << chunk_size << " ...................................");
      INT64 chunk_offset = GetChunk(chunk_size);
      if (Err::kERR_PTMALLOC_NO_CHUNK == chunk_offset) {
        LIB_CACHE_LOG_DEBUG("ptmalloc malloc has no idle chunk ...................................");
        chunk_offset = MallocChunk(chunk_size);
        if (Err::kERR_PTMALLOC_CHUNK_HAS_NO_MEMORY == chunk_offset ||
                Err::kERR_PTMALLOC_CHUNK_INFO_OBJECT_EMPTY == chunk_offset) {
          LIB_CACHE_LOG_ERROR("ptmalloc malloc failed result:" << chunk_offset);
          return NULL;
        }
      }

      Chunk * chunk = OffsetToChunk(chunk_offset);
      chunk->next_ = 0;
      chunk->prev_ = 0;

      LIB_CACHE_LOG_DEBUG("ptmalloc malloc success size:" << size << " chunk size:" << chunk_size
                                                          << " offset:" << chunk_offset
                                                          << " bottom:" << chunk_info_->bottom_
                                                          << " unused memory:"
                                                          << (chunk_info_->top_ - chunk_info_->bottom_)
                                                          << " LARGE BIN MAX:" << ChunkInfo::LARGE_BINS_MAX_BYTE
                                                          << " MIDDLE LARGE MIN:" << ChunkInfo::MID_LARGE_BINS_MIN_BYTE
                                                          << " MIDDLE LARGE MAX:" << ChunkInfo::MID_LARGE_BINS_MAX_BYTE);
      DumpChunk(chunk_offset);

      SetChunkUsed(chunk);
      return reinterpret_cast<VOID *>(reinterpret_cast<CHAR *>(chunk_info_) + chunk_offset + 2*sizeof(INT32));
    }

    INT64 PTMalloc::AddChunkToSortLink(INT64 * link_head, INT64 chunk_offset) {
      if (NULL == link_head || 0 >= chunk_offset) {
        LIB_CACHE_LOG_ERROR("ptmalloc get link head failed link object is empty");
        return Err::kERR_PTMALLOC_CHUNK_INFO_OBJECT_EMPTY;
      }

      Chunk * chunk = OffsetToChunk(chunk_offset);

      if (0 < *link_head) {
        Chunk * head = OffsetToChunk(*link_head);
        head->prev_ = chunk_offset;
        chunk->next_ = *link_head;
      }

      *link_head = chunk_offset;

      return 0;
    }

    INT64 PTMalloc::AddChunkToUnSortLink(INT64 * link_head, INT64 chunk_offset) {
      if (NULL == link_head || 0 >= chunk_offset) {
        LIB_CACHE_LOG_ERROR("ptmalloc get link head failed link object is empty");
        return Err::kERR_PTMALLOC_CHUNK_INFO_OBJECT_EMPTY;
      }

      Chunk * chunk = OffsetToChunk(chunk_offset);
      INT32 chunk_size = GetChunkSize(chunk);

      INT64 offset = *link_head;
      if (0 >= offset) {
        *link_head = chunk_offset;

        LIB_CACHE_LOG_DEBUG("ptmalloc free to unsorted bin head:" << *link_head);

        DumpLink(*link_head);

        return 0;
      }

      Chunk * p = NULL;
      while (0 < offset) {
        p = OffsetToChunk(offset);
        if (GetChunkSize(p) > chunk_size) {
          break;
        }
        offset = p->next_;
      }

      INT64 insert_offset = reinterpret_cast<INT64>(reinterpret_cast<CHAR *>(p) - reinterpret_cast<CHAR *>(chunk_info_));

      if (0 == offset) {
        LIB_CACHE_LOG_DEBUG("ptmalloc free to unsorted bin tail current size:" << chunk_size
                                                                               << " current  offset:" << chunk_offset
                                                                               << " tail size:" << GetChunkSize(p)
                                                                               << " tail offset:" << insert_offset);

        p->next_ = chunk_offset;
        chunk->prev_ = insert_offset;

        DumpLink(*link_head);

        return 0;
      }

      LIB_CACHE_LOG_DEBUG("ptmalloc free to unsorted bin middle current size:" << chunk_size
                                                                               << " current  offset:" << chunk_offset);
      DumpChunk(chunk_offset);
      DumpChunk(insert_offset);

      chunk->next_ = insert_offset;
      if (0 == p->prev_) {
        *link_head = chunk_offset;
      } else {
        Chunk * p_prev = OffsetToChunk(p->prev_);
        p_prev->next_ = chunk_offset;
      }
      chunk->prev_ = p->prev_;
      p->prev_ = chunk_offset;

      return 0;
    }

    INT64 PTMalloc::AddChunkToLink(INT64 chunk_size, INT64 chunk_offset) {
      if (chunk_size < ChunkInfo::SMALL_BINS_MIN_BYTE) {
        LIB_CACHE_LOG_ERROR("free memory failed chunk size: " << chunk_size << " is invalid");

        return Err::kERR_PTMALLOC_CHUNK_SIZE_INVALID;
      }

      if (chunk_size <= ChunkInfo::LARGE_BINS_MAX_BYTE) {
        INT32 index = (chunk_size - ChunkInfo::SMALL_BINS_MIN_BYTE)/ChunkInfo::SORT_BINS_STEP - 1;
        if (0 != AddChunkToSortLink(&chunk_info_->sort_bins_[index], chunk_offset)) {
          LIB_CACHE_LOG_ERROR("ptmalloc add to sort bin failed size:" << chunk_size
                                                                       << " offset:" << chunk_offset
                                                                       << " index:" << index);
        } else {
          LIB_CACHE_LOG_DEBUG("ptmalloc add to sort bin success size:" << chunk_size
                                                                        << " offset:" << chunk_offset
                                                                        << " index:" << index);
        }

        return 0;
      }

      if (ChunkInfo::MID_LARGE_BINS_MAX_BYTE >= chunk_size && ChunkInfo::MID_LARGE_BINS_MIN_BYTE < chunk_size) {
        INT32 index = ((chunk_size - ChunkInfo::MID_LARGE_BINS_MIN_BYTE)/ChunkInfo::MID_LARGE_BINS_STEP) - 1;
        if (0 != AddChunkToSortLink(&chunk_info_->mid_large_bins_[index], chunk_offset)) {
          LIB_CACHE_LOG_ERROR("ptmalloc add to mid large bin failed size:" << chunk_size
                                                                            << " offset:" << chunk_offset
                                                                            << " index:" << index);
        } else {
          LIB_CACHE_LOG_DEBUG("ptmalloc add to mid large bin success size:" << chunk_size
                                                                             << " offset:" << chunk_offset
                                                                             << " index:" << index);
        }

        return 0;
      }

      if (ChunkInfo::MID_LARGE_BINS_MAX_BYTE < chunk_size) {
        if (0 != AddChunkToUnSortLink(&chunk_info_->unsorted_bins_, chunk_offset) ) {
          LIB_CACHE_LOG_ERROR("ptmalloc add to unsorted bin failed size:" << chunk_size << " offset:" << chunk_offset);
        } else {
          LIB_CACHE_LOG_DEBUG("ptmalloc add to unsorted bin success size:" << chunk_size << " offset:" << chunk_offset);
        }

        return 0;
      }

      return Err::kERR_PTMALLOC_CHUNK_SIZE_INVALID;
    }

    VOID PTMalloc::free(VOID * address) {
      if (NULL == address) {
        return;
      }
      Mutex mutex(&chunk_info_->mutex_);
      ScopeLock<Mutex> scope(&mutex);

      Chunk * chunk = reinterpret_cast<Chunk *>((reinterpret_cast<CHAR *>(address)) - 2*sizeof(INT32));
      chunk->next_ = 0;
      chunk->prev_ = 0;
      INT32 chunk_size = GetChunkSize(chunk);
      if (chunk_size < ChunkInfo::SMALL_BINS_MIN_BYTE) {
        LIB_CACHE_LOG_ERROR("free memory failed chunk size: " << chunk_size << " is invalid");

        return;
      }
      INT32 chunk_offset = reinterpret_cast<INT64>(chunk) - reinterpret_cast<INT64>(chunk_info_);
      LIB_CACHE_LOG_DEBUG("Free chunk********************************************************************************");
      DumpChunk(chunk_offset);
      if (chunk_size <= ChunkInfo::FAST_BINS_MAX_BYTE) {
        INT32 index = ((chunk_size - ChunkInfo::FAST_BINS_MIN_BYTE)/ChunkInfo::SORT_BINS_STEP - 1);
        if (0 != AddChunkToSortLink(&chunk_info_->fast_bins_[index], chunk_offset)) {
          LIB_CACHE_LOG_ERROR("ptmalloc free to fast bin failed size:" << chunk_size
                                                                        << " offset:" << chunk_offset
                                                                        << " index:" << index);
        } else {
          LIB_CACHE_LOG_DEBUG("ptmalloc free to fast bin success size:" << chunk_size
                                                                        << " offset:" << chunk_offset
                                                                        << " index:" << index);
        }

        return;
      }

      if (chunk_size <= ChunkInfo::LARGE_BINS_MAX_BYTE) {
        INT32 index = (chunk_size - ChunkInfo::SMALL_BINS_MIN_BYTE)/ChunkInfo::SORT_BINS_STEP - 1;
        if (0 != AddChunkToSortLink(&chunk_info_->sort_bins_[index], chunk_offset)) {
          LIB_CACHE_LOG_ERROR("ptmalloc free to sort bin failed size:" << chunk_size
                                                                       << " offset:" << chunk_offset
                                                                       << " index:" << index);
        } else {
          SetChunkUnUsed(chunk);
          LIB_CACHE_LOG_DEBUG("ptmalloc free to sort bin success size:" << chunk_size
                                                                        << " offset:" << chunk_offset
                                                                        << " index:" << index);
        }

        return;
      }

      if (ChunkInfo::MID_LARGE_BINS_MAX_BYTE >= chunk_size && ChunkInfo::MID_LARGE_BINS_MIN_BYTE < chunk_size) {
        INT32 index = ((chunk_size - ChunkInfo::MID_LARGE_BINS_MIN_BYTE)/ChunkInfo::MID_LARGE_BINS_STEP) - 1;
        if (0 != AddChunkToSortLink(&chunk_info_->mid_large_bins_[index], chunk_offset)) {
          LIB_CACHE_LOG_ERROR("ptmalloc free to mid large bin failed size:" << chunk_size
                                                                        << " offset:" << chunk_offset
                                                                        << " index:" << index);
        } else {
          SetChunkUnUsed(chunk);
          LIB_CACHE_LOG_DEBUG("ptmalloc free to mid large bin success size:" << chunk_size
                                                                         << " offset:" << chunk_offset
                                                                         << " index:" << index);
        }

        return;
      }

      if (ChunkInfo::MID_LARGE_BINS_MAX_BYTE < chunk_size) {
        if (0 != AddChunkToUnSortLink(&chunk_info_->unsorted_bins_, chunk_offset) ) {
          LIB_CACHE_LOG_ERROR("ptmalloc free to unsorted bin failed size:" << chunk_size << " offset:" << chunk_offset);
        } else {
          SetChunkUnUsed(chunk);
          LIB_CACHE_LOG_DEBUG("ptmalloc free to unsorted bin success size:" << chunk_size << " offset:" << chunk_offset);
        }

        DumpLink(chunk_info_->unsorted_bins_);

        return;
      }
    }

    INT64 PTMalloc::GetChunkFromSortLink(INT64 * link_head) {
      if (NULL == link_head) {
        LIB_CACHE_LOG_ERROR("ptmalloc get link head failed link object is empty");
        return Err::kERR_PTMALLOC_NO_CHUNK;
      }
      INT32 offset = *link_head;
      Chunk * chunk = OffsetToChunk(offset);
      if (0 < chunk->next_) {
        Chunk *next_chunk = OffsetToChunk(chunk->next_);
        next_chunk->prev_ = 0;
      }
      *link_head = chunk->next_;

      return offset;
    }

    INT64 PTMalloc::GetChunkFromUnSortLink(INT64 * link_head, INT32 size) {
      if (NULL == link_head) {
        LIB_CACHE_LOG_ERROR("ptmalloc get link head failed link object is empty");
        return Err::kERR_PTMALLOC_NO_CHUNK;
      }

      INT64 offset = *link_head;
      while (0 < offset) {
        Chunk * chunk = OffsetToChunk(offset);
        if (GetChunkSize(chunk) >= size) {
          if (0 == chunk->prev_) {
            *link_head = chunk->next_;
          } else {
            Chunk * prev_chunk = OffsetToChunk(chunk->prev_);
            prev_chunk->next_ = chunk->next_;
          }

          if (0 < chunk->next_) {
            Chunk * next_chunk = OffsetToChunk(chunk->next_);
            next_chunk->prev_ = chunk->prev_;
          }

          DumpLink(*link_head);

          return offset;
        }

        if (0 == chunk->next_) {
          return Err::kERR_PTMALLOC_NO_CHUNK;
        }

        offset = chunk->next_;
      }

      return Err::kERR_PTMALLOC_NO_CHUNK;
    }

    INT64 PTMalloc::GetChunk(INT32 size) {
      if (NULL == chunk_info_) {
        LIB_CACHE_LOG_ERROR("ptmalloc get bin index failed size:" << size << " chunk info object is empty");
        return Err::kERR_PTMALLOC_CHUNK_INFO_OBJECT_EMPTY;
      }

      if (ChunkInfo::FAST_BINS_MAX_BYTE >= size) {
        INT32 index = ((size - ChunkInfo::FAST_BINS_MIN_BYTE)/ChunkInfo::SORT_BINS_STEP) - 1;
        if (0 < chunk_info_->fast_bins_[index]) {
          LIB_CACHE_LOG_DEBUG("ptmalloc get chunk from fast bin index:" << index << " size:" << size
                                                                            << " offset:" << chunk_info_->fast_bins_[index]);
          return GetChunkFromSortLink(&chunk_info_->fast_bins_[index]);
        }
      }

      if (ChunkInfo::LARGE_BINS_MAX_BYTE >= size) {
        INT32 index = ((size - ChunkInfo::SMALL_BINS_MIN_BYTE) / ChunkInfo::SORT_BINS_STEP) - 1;

        if (0 < chunk_info_->sort_bins_[index]) {
          LIB_CACHE_LOG_DEBUG("ptmalloc get chunk from sort bin index:" << index << " size:" << size
                                                                            << " offset:" << chunk_info_->sort_bins_[index]);
          return GetChunkFromSortLink(&chunk_info_->sort_bins_[index]);
        }

        return Err::kERR_PTMALLOC_NO_CHUNK;
      }

      if (ChunkInfo::MID_LARGE_BINS_MIN_BYTE < size && ChunkInfo::MID_LARGE_BINS_MAX_BYTE >= size) {
        INT32 index = ((size - ChunkInfo::MID_LARGE_BINS_MIN_BYTE) / ChunkInfo::MID_LARGE_BINS_STEP) - 1;
        if (0 < chunk_info_->mid_large_bins_[index]) {
          LIB_CACHE_LOG_DEBUG("ptmalloc get chunk from mid large bin index:" << index << " size:" << size
                                                                            << " offset:" << chunk_info_->mid_large_bins_[index]);
          return GetChunkFromSortLink(&chunk_info_->mid_large_bins_[index]);
        }

        return Err::kERR_PTMALLOC_NO_CHUNK;
      }

      if (ChunkInfo::UNSORT_LARGE_BINS_MIN_BYTE < size) {
        if (0 >= chunk_info_->unsorted_bins_) {
          return Err::kERR_PTMALLOC_NO_CHUNK;
        }

        LIB_CACHE_LOG_DEBUG("ptmalloc get chunk from supper large bin ******************************************");
        DumpLink(chunk_info_->unsorted_bins_);

        return GetChunkFromUnSortLink(&chunk_info_->unsorted_bins_, size);
      }

      return Err::kERR_PTMALLOC_NO_CHUNK;
    }

    Chunk * PTMalloc::OffsetToChunk(INT64 offset) {
      return reinterpret_cast<Chunk *>(reinterpret_cast<CHAR *>(chunk_info_) + offset);
    }

    BOOL PTMalloc::IsChunkUsing(INT64 offset) {
      if (0 >= offset) {
        LIB_CACHE_LOG_ERROR("ptmalloc is chunk using chunk info object is empty");
        return TRUE;
      }

      return IsChunkUsing(OffsetToChunk(offset));
    }

    BOOL PTMalloc::IsChunkUsing(Chunk * chunk) {
      if (NULL == chunk) {
        LIB_CACHE_LOG_ERROR("ptmalloc is chunk using chunk info object is empty");
        return TRUE;
      }

      return (1 == (chunk->size_ & CHUNK_USED_FLAG_MASK));
    }

    VOID PTMalloc::SetChunkUsed(INT64 offset) {
      if (0 >= offset) {
        LIB_CACHE_LOG_ERROR("ptmalloc set chunk used chunk info object is empty");
        return ;
      }

      SetChunkUsed(OffsetToChunk(offset));
    }

    VOID PTMalloc::SetChunkUsed(Chunk * chunk) {
      if (NULL == chunk) {
        LIB_CACHE_LOG_ERROR("ptmalloc set chunk used chunk info object is empty");
        return ;
      }

      chunk->size_ |= CHUNK_USED_FLAG_MASK;
    }

    VOID PTMalloc::SetChunkUnUsed(INT64 offset) {
      if (0 >= offset) {
        LIB_CACHE_LOG_ERROR("ptmalloc set chunk unused chunk info object is empty");
        return ;
      }

      SetChunkUnUsed(OffsetToChunk(offset));
    }

    VOID PTMalloc::SetChunkUnUsed(Chunk * chunk) {
      if (NULL == chunk) {
        LIB_CACHE_LOG_ERROR("ptmalloc set chunk unused chunk info object is empty");
        return ;
      }

      chunk->size_ &= CHUNK_UNUSED_FLAG_MASK;
    }

    VOID PTMalloc::SetChunkSize(Chunk * chunk, INT32 size) {
      if (NULL == chunk) {
        LIB_CACHE_LOG_ERROR("ptmalloc set chunk size chunk info object is empty");
        return ;
      }

      chunk->size_ = size << 1;
      chunk->size_ &= CHUNK_UNUSED_FLAG_MASK;
    }

    INT32 PTMalloc::GetChunkSize(Chunk * chunk) {
      if (NULL == chunk) {
        LIB_CACHE_LOG_ERROR("ptmalloc gert chunk size chunk info object is empty");
        return Err::kERR_PTMALLOC_CHUNK_INFO_OBJECT_EMPTY;
      }

      return chunk->size_ >> 1;
    }

    VOID PTMalloc::SetChunkSize(INT64 offset, INT32 size) {
      if (0 >= offset) {
        LIB_CACHE_LOG_ERROR("ptmalloc set chunk size chunk info object is empty");
        return;
      }

      SetChunkSize(OffsetToChunk(offset), size);
    }

    INT32 PTMalloc::GetChunkSize(INT64 offset) {
      if (0 >= offset) {
        LIB_CACHE_LOG_ERROR("ptmalloc get chunk size chunk info object is empty");
        return Err::kERR_PTMALLOC_CHUNK_INFO_OBJECT_EMPTY;
      }

      return GetChunkSize(OffsetToChunk(offset));
    }

    INT64 PTMalloc::MallocChunk(INT32 size) {
      if (NULL == chunk_info_) {
        LIB_CACHE_LOG_ERROR("ptmalloc get bin index failed size:" << size << " chunk info object is empty");
        return Err::kERR_PTMALLOC_CHUNK_INFO_OBJECT_EMPTY;
      }

      if (size > (chunk_info_->top_ - chunk_info_->bottom_)) {
        LIB_CACHE_LOG_ERROR("ptmalloc malloc size: " << size << " has no enough memory bottom:" << chunk_info_->bottom_
                                                     << " top:" << chunk_info_->top_ << " total size:"
                                                     << chunk_info_->size_);
        return Err::kERR_PTMALLOC_CHUNK_HAS_NO_MEMORY;
      }

      INT64 offset = chunk_info_->bottom_;
      chunk_info_->bottom_ += size;
      Chunk * chunk = OffsetToChunk(offset);
      SetChunkSize(chunk, size);
      SetChunkUsed(chunk);
      chunk->prev_ = 0;
      chunk->next_ = 0;
      INT32 chunk_head_size = sizeof(Chunk);
      if ((chunk_info_->top_ - chunk_info_->bottom_) > chunk_head_size) {
        Chunk *next_chunk = OffsetToChunk(chunk_info_->bottom_);
        next_chunk->prev_size_ = size;
      }

      return offset;
    }

    BOOL PTMalloc::IsChunkInLink(INT64 * link_head, INT64 chunk_offset) {
      if (0 >= *link_head) {
        return FALSE;
      }

      INT64 offset = *link_head;
      while (0 < offset) {
        if (offset == chunk_offset) {
          return TRUE;
        }

        offset = OffsetToChunk(offset)->next_;
      }

      return FALSE;
    }

    INT64 PTMalloc::DelFromLink(INT64 * link_head, INT64 chunk_offset) {
      if (NULL == link_head || 0 >= chunk_offset) {
        LIB_CACHE_LOG_ERROR("ptmalloc link head object is empty");
        return Err::kERR_PTMALLOC_CHUNK_INFO_OBJECT_EMPTY;
      }

      Chunk * chunk = OffsetToChunk(chunk_offset);
      if (0 >= chunk->prev_ && 0 >= chunk->prev_ && *link_head != chunk_offset) {
        LIB_CACHE_LOG_WARN("ptmalloc delete from link chunk offset:" << chunk_offset << " is not in link list");
        return 0;
      }

      if (0 >= chunk->prev_) {
        if (*link_head == chunk_offset) {
          *link_head = chunk->next_;
        } else {
          LIB_CACHE_LOG_ERROR("ptmalloc delete from link head:" << *link_head << " is not equal chunk_offset:"
                                                                << chunk_offset);
          DumpChunk(*link_head);
          DumpChunk(chunk_offset);

          return Err::kERR_PTMALLOC_CHUNK_DELETE_FROM_LINK_HEAD_ERROR;
        }
      } else {
        Chunk * chunk_prev = OffsetToChunk(chunk->prev_);
        chunk_prev->next_ = chunk->next_;
      }

      if (0 < chunk->next_) {
        Chunk * chunk_next = OffsetToChunk(chunk->next_);
        chunk_next->prev_ = chunk->prev_;
      }

      chunk->prev_ = 0;
      chunk->next_ = 0;

      return 0;
    }

    INT64 * PTMalloc::GetLinkHead(INT32 size) {
      if (ChunkInfo::LARGE_BINS_MAX_BYTE >= size) {
        INT32 index = ((size - ChunkInfo::SMALL_BINS_MIN_BYTE) / ChunkInfo::SORT_BINS_STEP) - 1;

        if (0 < chunk_info_->sort_bins_[index]) {
          return &chunk_info_->sort_bins_[index];
        }

        return NULL;
      }

      if (ChunkInfo::MID_LARGE_BINS_MIN_BYTE < size && ChunkInfo::MID_LARGE_BINS_MAX_BYTE >= size) {
        INT32 index = ((size - ChunkInfo::MID_LARGE_BINS_MIN_BYTE) / ChunkInfo::MID_LARGE_BINS_STEP) - 1;
        if (0 < chunk_info_->mid_large_bins_[index]) {
          return &chunk_info_->mid_large_bins_[index];
        }

        return NULL;
      }

      if (ChunkInfo::UNSORT_LARGE_BINS_MIN_BYTE < size) {
        if (0 >= chunk_info_->unsorted_bins_) {
          return NULL;
        }

        return &chunk_info_->unsorted_bins_;
      }

      return NULL;
    }

    VOID PTMalloc::FastMoveToSmall() {
      LIB_CACHE_LOG_DEBUG("ptmalloc move fast to small **************************************************************");
      for (INT32 i = 0; i < ChunkInfo::FAST_BINS_COUNT; ++i) {
        while (0 < chunk_info_->fast_bins_[i]) {
          INT64 offset = chunk_info_->fast_bins_[i];
          DelFromLink(&chunk_info_->fast_bins_[i], offset);
          SetChunkUnUsed(offset);
          AddChunkToSortLink(&chunk_info_->sort_bins_[i], offset);
        }
      }
    }

    VOID PTMalloc::MergeNearBottom() {
      LIB_CACHE_LOG_DEBUG("ptmalloc merge near bottom ***************************************************************");
      INT64 cursor = chunk_info_->bottom_;
      while (cursor > chunk_info_->chunk_start_) {
        cursor -= OffsetToChunk(cursor)->prev_size_;

        if (IsChunkUsing(cursor)) {
          return;
        }

        INT64 * link_head = GetLinkHead(GetChunkSize(cursor));
        if (0 != DelFromLink(link_head, cursor)) {
          LIB_CACHE_LOG_ERROR("ptmalloc merge near bottom chunk delete from link failed head:" << *link_head
                                                                                               << " cursor:" << cursor);

          return;
        }

        chunk_info_->bottom_ = cursor;
      }
    }

    VOID PTMalloc::MergeFreeChunk() {
      INT64 cursor = chunk_info_->bottom_;
      LIB_CACHE_LOG_DEBUG("ptmalloc merge free chunk cursor:" << cursor << " start:" << chunk_info_->chunk_start_);
      while (cursor > chunk_info_->chunk_start_) {
        cursor -= OffsetToChunk(cursor)->prev_size_;

        if (IsChunkUsing(cursor)) {
          continue;
        }

        INT64 next_offset = cursor + GetChunkSize(cursor);

        if (IsChunkUsing(next_offset)) {
          continue;
        }

        INT64 * cursor_link_head = GetLinkHead(GetChunkSize(cursor));
        INT64 * next_link_head = GetLinkHead(GetChunkSize(next_offset));

        if (!IsChunkInLink(cursor_link_head, cursor)) {
          continue;
        }

        if (!IsChunkInLink(next_link_head, next_offset)) {
          continue;
        }

        if (0 != DelFromLink(cursor_link_head, cursor)) {
          LIB_CACHE_LOG_WARN("ptmalloc merge free chunk delete from link failed head:" << *cursor_link_head
                                                                                        << " cursor:" << cursor);

          continue;
        }

        if (0 != DelFromLink(next_link_head, next_offset)) {
          LIB_CACHE_LOG_WARN("ptmalloc merge free chunk delete from link failed head:" << *next_link_head
                                                                                       << " next offset:" << next_offset);

          continue;
        }

        INT64 new_chunk_size = GetChunkSize(cursor) + GetChunkSize(next_offset);
        SetChunkSize(cursor, new_chunk_size);
        OffsetToChunk(cursor)->prev_ = 0;
        OffsetToChunk(cursor)->next_ = 0;
        INT64 next_next_offset = next_offset + GetChunkSize(next_offset);
        OffsetToChunk(next_next_offset)->prev_size_ = new_chunk_size;

        if (0 != AddChunkToLink(new_chunk_size, cursor)) {
          LIB_CACHE_LOG_ERROR("ptmalloc merge chunk add to link failed offset:" << cursor
                                                                                << " chunk size:" << new_chunk_size);
        }
      }
    }

    VOID PTMalloc::MergeFragment() {
      Mutex mutex(&chunk_info_->mutex_);
      ScopeLock<Mutex> scope(&mutex);

      FastMoveToSmall();

      MergeNearBottom();

      MergeFreeChunk();
    }

    VOID PTMalloc::Dump() {
      LIB_CACHE_LOG_DEBUG("Head info B *****************************************************************************");
      LIB_CACHE_LOG_DEBUG("flag:" << chunk_info_->flag_);
      LIB_CACHE_LOG_DEBUG("size:" << chunk_info_->size_);
      LIB_CACHE_LOG_DEBUG("unsorted_bins:" << chunk_info_->unsorted_bins_);
      LIB_CACHE_LOG_DEBUG("top:" << chunk_info_->top_);
      LIB_CACHE_LOG_DEBUG("bottom:" << chunk_info_->bottom_);
      LIB_CACHE_LOG_DEBUG("unused memory:" << (chunk_info_->top_ - chunk_info_->bottom_));
      LIB_CACHE_LOG_DEBUG("Head info E *****************************************************************************");

      for (INT32 i = 0; i < ChunkInfo::SORT_BINS_COUNT; ++i) {
        if (0 < chunk_info_->sort_bins_[i]) {
          LIB_CACHE_LOG_DEBUG("sort bins index:" << i << "****************************************************");
          DumpLink(chunk_info_->sort_bins_[i]);
        }
      }

      for (INT32 i = 0; i < ChunkInfo::FAST_BINS_COUNT; ++i) {
        if (0 < chunk_info_->fast_bins_[i]) {
          LIB_CACHE_LOG_DEBUG("fast bins index:" << i << "****************************************************");
          DumpLink(chunk_info_->fast_bins_[i]);
        }
      }

      for (INT32 i = 0; i < ChunkInfo::MID_LARGE_BINS_COUNT; ++i) {
        if (0 < chunk_info_->mid_large_bins_[i]) {
          LIB_CACHE_LOG_DEBUG("middle large bins index:" << i << "****************************************************");
          DumpLink(chunk_info_->mid_large_bins_[i]);
        }
      }

      if (0 < chunk_info_->unsorted_bins_) {
        LIB_CACHE_LOG_DEBUG("supper large bins ***********************************************************************");
        DumpLink(chunk_info_->unsorted_bins_);
      }
    }

    VOID PTMalloc::DumpLink(INT64 offset) {
      INT32 i = 0;
      while (0 < offset) {
        LIB_CACHE_LOG_DEBUG("*** index:" << i++ << " ****************************************************");
        Chunk * chunk = OffsetToChunk(offset);
        DumpChunk(offset);
        offset = chunk->next_;
      }
    }

    VOID PTMalloc::DumpSeq() {
      LIB_CACHE_LOG_DEBUG("chunk info B *****************************************************************************");
      INT64 offset = chunk_info_->chunk_start_;
      while (offset != chunk_info_->bottom_) {
        DumpChunk(offset);
        Chunk * chunk = OffsetToChunk(offset);
        offset += GetChunkSize(chunk);
      }
      LIB_CACHE_LOG_DEBUG("chunk info E *****************************************************************************");
    }

    VOID PTMalloc::DumpChunk(INT64 offset) {
      Chunk * chunk = OffsetToChunk(offset);
      LIB_CACHE_LOG_DEBUG("chunk info B *****************************************************************************");
      LIB_CACHE_LOG_DEBUG("offset:" << offset);
      LIB_CACHE_LOG_DEBUG("prev_size:" << chunk->prev_size_);
      LIB_CACHE_LOG_DEBUG("size:" << GetChunkSize(chunk));
      LIB_CACHE_LOG_DEBUG("used:" << IsChunkUsing(chunk));
      LIB_CACHE_LOG_DEBUG("prev:" << chunk->prev_);
      LIB_CACHE_LOG_DEBUG("next:" << chunk->next_);
      LIB_CACHE_LOG_DEBUG("chunk info E *****************************************************************************");
    }
  }  // namespace cache
}  // namespace lib