// Copyright (c) 2013 zhou chang yue. All rights reserved.

#ifndef COMMLIB_CACHE_INC_ERR_H_
#define COMMLIB_CACHE_INC_ERR_H_

#include <errno.h>
#include <string>
#include <cstdio>
#include "commlib/public/inc/type.h"

namespace lib {
  namespace cache {
    using std::string;

    class Err {
     public:
       // pool errors
       static const INT32 kERR_POOL_CREATE_FAILED = -100300;
       static const INT32 kERR_BUFFER_MALLOC_FAILED = -100301;
       static const INT32 kERR_BUFFER_DATA_TOO_LARGER = -100302;

       static const INT32 kERR_PTMALLOC_NO_CHUNK = -100099;
       static const INT32 kERR_PTMALLOC_CHUNK_INFO_OBJECT_EMPTY = -100100;
       static const INT32 kERR_PTMALLOC_CHUNK_HAS_NO_MEMORY = -100101;
       static const INT32 kERR_PTMALLOC_CHUNK_DELETE_FROM_LINK_HEAD_ERROR = -100102;
       static const INT32 kERR_PTMALLOC_CHUNK_SIZE_INVALID = -100103;

       static const INT32 kERR_NODE_GROUP_INFO_OBJECT_EMPTY = -100200;
       static const INT32 kERR_NODE_GROUP_NODE_KEY_EXISTS = -100201;
       static const INT32 kERR_NODE_GROUP_NODE_KEY_NOT_EXISTS = -100202;
       static const INT32 kERR_NODE_GROUP_NODE_KEY_DATA_IS_NOT_IN_CACHE = -100203;
       static const INT32 kERR_NODE_GROUP_NODE_INITIAL_LOCK_FAILED = -100204;
       static const INT32 kERR_NODE_GROUP_HAS_NO_FREE_NODE = -100205;
       static const INT32 kERR_NODE_GROUP_NODE_OFFSET_INVALID = -100206;
       static const INT32 kERR_NODE_GROUP_COUNT_IS_TOO_MANY = -100207;
       static const INT32 kERR_NODE_GROUP_ALLOCATE_BUFFER_FAILED = -100208;
       static const INT32 kERR_NODE_GROUP_INITIAL_MUTEX_FAILED = -100209;

       static const INT32 kERR_NODE_GROUP_GET_BUFFER_IS_TOO_SMALL = -100600;

       static const INT32 kERR_CACHE_ALLOCATE_NODE_GROUP_FAILED = -100300;
       static const INT32 kERR_INDEX_CACHE_ALLOCATE_INDEX_GROUP_FAILED = -100301;       

       static const INT32 kERR_INDEX_GROUP_KEY_INDEX_INVALID = -100400;
       static const INT32 kERR_INDEX_GROUP_INSERT_INDEX_NODE_OFFSET_INVALID = -100401;
       static const INT32 kERR_INDEX_GROUP_HAS_NO_FREE_INDEX_NODE = -100402;
       static const INT32 kERR_INDEX_GROUP_HAS_NO_FREE_KEY_NODE = -100403;
       static const INT32 kERR_INDEX_GROUP_INDEX_KEY_IS_NOT_EXISTS = -100404;
       static const INT32 kERR_INDEX_GROUP_INDEX_KEY_HAS_NO_UNIQUE_KEY = -100405;
       static const INT32 kERR_INDEX_GROUP_ALLOCATE_BUFFER_FAILED = -100406;   
       static const INT32 kERR_INDEX_GROUP_INFO_OBJECT_EMPTY = -100407;    
       static const INT32 kERR_INDEX_GROUP_INITIAL_MUTEX_FAILED = -100408;       
    };
  }  // namespace cache
}  // namespace lib

#endif  // COMMLIB_CACHE_INC_ERR_H_
