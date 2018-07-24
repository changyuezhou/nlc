// Copyright (c) 2013 zhou chang yue. All rights reserved.

#ifndef COMMLIB_CACHE_INC_CACHE_H_
#define COMMLIB_CACHE_INC_CACHE_H_

#include <stdlib.h>
#include <string>
#include <map>
#include <vector>
#include "commlib/public/inc/type.h"
#include "commlib/cache/inc/node_group.h"
#include "commlib/cache/inc/log.h"
#include "commlib/cache/inc/err.h"

namespace lib {
  namespace cache {
    using std::string;
    using std::map;
    using std::vector;

    typedef string KEY;
    typedef vector<string> KEYS;
    typedef vector<string> COLUMNS;

    class Cache {
     public:
       enum CACHE_FLAG {
         DIRTY = 3,
         KEY_NOT_EXISTS = 4,
         IN_CACHE = 5
       };

     public:
       explicit Cache():node_group_(NULL), need_free_(FALSE), buffer_(NULL) {}
       ~Cache() { Destroy(); }

     public:
       INT32 Initial(INT64 size, VOID * buffer = NULL);

     protected:
       INT32 Initial(NodeMemInfo * node_mem_info, ChunkInfo * chunk_info);
       INT32 Initial(NodeMemInfo * node_mem_info, INT64 total_size);

     public:
       INT32 Update(const KEYS & keys);
       INT32 Update(const KEY & key);
       INT32 Delete(const KEYS & keys);
       INT32 Delete(const KEY & key);

     public:
       INT32 Get(const KEY & key, CHAR * data, INT32 * max_size);
       INT32 Set(const KEY & key, const CHAR * data, INT32 size);
       UINT64 LastAccessTimestamp();
       UINT32 KeysCount() { return node_group_->TotalKeys(); }
       INT32 Status(const KEY & key);

     public:
       VOID Dump();

     protected:
       VOID Destroy();

     private:
       Cache(const Cache &);
       Cache & operator=(const Cache &);

     private:
       NodeGroup * node_group_;
       BOOL need_free_;
       VOID * buffer_;
    };
  }  // namespace cache
}  // namespace lib

#endif  // COMMLIB_CACHE_INC_CACHE_H_
