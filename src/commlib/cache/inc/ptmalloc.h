// Copyright (c) 2013 zhou chang yue. All rights reserved.

#ifndef COMMLIB_CACHE_INC_PTMALLOC_H_
#define COMMLIB_CACHE_INC_PTMALLOC_H_

#include <string>
#include "commlib/public/inc/type.h"
#include "commlib/cache/inc/err.h"
#include "commlib/cache/inc/chunk.h"
#include "commlib/cache/inc/allocator.h"

namespace lib {
  namespace cache {
    class PTMalloc: public Allocator {
     public:
       static const INT32 CHUNK_USED_FLAG_MASK = 0x00000001;
       static const INT32 CHUNK_UNUSED_FLAG_MASK = 0xFFFFFFFE;

     public:
       PTMalloc(ChunkInfo * chunk_info):chunk_info_(chunk_info) {}
       virtual ~PTMalloc() {}

     public:
       VOID * malloc(INT32 size);
       VOID free(VOID * address);

     public:
       VOID Dump();
       VOID DumpSeq();
       VOID MergeFragment();

     protected:
       INT64 GetChunk(INT32 size);
       INT64 MallocChunk(INT32 size);
       INT64 GetChunkFromSortLink(INT64 * link_head);
       INT64 GetChunkFromUnSortLink(INT64 * link_head, INT32 size);
       INT64 AddChunkToSortLink(INT64 * link_head, INT64 chunk_offset);
       INT64 AddChunkToUnSortLink(INT64 * link_head, INT64 chunk_offset);
       INT64 AddChunkToLink(INT64 chunk_size, INT64 chunk_offset);
       INT64 DelFromLink(INT64 * link_head, INT64 chunk_offset);
       Chunk * OffsetToChunk(INT64 offset);
       INT64 * GetLinkHead(INT32 size);
       BOOL IsChunkInLink(INT64 * link_head, INT64 chunk_offset);

     protected:
       BOOL IsChunkUsing(INT64 offset);
       BOOL IsChunkUsing(Chunk * chunk);
       VOID SetChunkUsed(INT64 offset);
       VOID SetChunkUsed(Chunk * chunk);
       VOID SetChunkUnUsed(INT64 offset);
       VOID SetChunkUnUsed(Chunk * chunk);
       VOID SetChunkSize(Chunk * chunk, INT32 size);
       INT32 GetChunkSize(Chunk * chunk);
       VOID SetChunkSize(INT64 offset, INT32 size);
       INT32 GetChunkSize(INT64 offset);

     protected:
       VOID FastMoveToSmall();
       VOID MergeNearBottom();
       VOID MergeFreeChunk();

     protected:
       VOID DumpLink(INT64 offset);
       VOID DumpChunk(INT64 offset);

     private:
       PTMalloc(const PTMalloc &);
       PTMalloc & operator=(const PTMalloc &);

     private:
       ChunkInfo * chunk_info_;
    };
  }
}

#endif  // COMMLIB_CACHE_INC_PTMALLOC_H_