// Copyright (c) 2013 zhou chang yue. All rights reserved.

#ifndef COMMLIB_CACHE_INC_INDEX_CACHE_H_
#define COMMLIB_CACHE_INC_INDEX_CACHE_H_

#include <stdlib.h>
#include <string>
#include <map>
#include <vector>
#include "commlib/public/inc/type.h"
#include "commlib/cache/inc/index_group.h"
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

    class IndexCache {
     public:
       explicit IndexCache():index_group_(NULL), need_free_(FALSE), buffer_(NULL) {}
       ~IndexCache() { Destroy(); }

     public:
       INT32 Initial(INT64 size, VOID * buffer = NULL);      

     public:
       INT32 Insert(const string & index_key, const string & unique_key);
       INT32 Insert(const string & index_key, const KEYS & unique_keys);
       INT32 Delete(const string & index_key, const string & unique_key);
       INT32 Delete(const string & index_key, const KEYS & unique_keys);       

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

     public:
       VOID Dump();

     protected:
       INT32 Initial(IndexMemInfo * index_mem_info, ChunkInfo * chunk_info);
       INT32 Initial(IndexMemInfo * index_mem_info, INT64 total_size);

     protected:
       VOID Destroy();

     private:
       IndexCache(const IndexCache &);
       IndexCache & operator=(const IndexCache &);

     private:
       IndexGroup * index_group_;
       BOOL need_free_;
       VOID * buffer_;
    };
  }  // namespace cache
}  // namespace lib

#endif  // COMMLIB_CACHE_INC_INDEX_CACHE_H_
