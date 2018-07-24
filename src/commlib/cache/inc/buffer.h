// Copyright (c) 2013 zhou chang yue. All rights reserved.

#ifndef COMMLIB_CACHE_INC_BUFFER_H_
#define COMMLIB_CACHE_INC_BUFFER_H_

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <map>
#include <utility>
#include "commlib/public/inc/type.h"
#include "commlib/cache/inc/err.h"

namespace lib {
  namespace cache {
    using std::pair;
    using std::make_pair;

    class Buffer {
     public:
       typedef pair<INT32, const CHAR *> Result;

     public:
       Buffer():data_(NULL), capacity_(0), size_(0), skip_(0) {}
       Buffer(CHAR * data, INT32 capacity):data_(data), capacity_(capacity), size_(0), skip_(0) {}
       ~Buffer() {}

     private:
       const Buffer & operator=(const Buffer &);
       Buffer(const Buffer &);

     public:
       Result GetData() { return make_pair(size_, data_ + skip_); }
       VOID Skip(INT32 size) {
         skip_ = size;
         size_ -= size;
       }
       VOID Reset() { skip_ = size_ = 0; }

     public:
       INT32 GetCapacity() { return capacity_; }
       INT32 GetSize() { return size_; }
       CHAR * GetString() { return data_?(data_ + skip_):NULL; }
       VOID Free() { if (data_) ::free(data_); }

     public:
       INT32 Append(const CHAR * data, INT32 size) {
         if ((size + size_) > capacity_) {
           INT32 result = recycle(size + size_);
           if (0 != result) {
             return result;
           }
         }

         ::memcpy(data_ + size_, data, size);
         size_ += size;

         return 0;
       }

       INT32 Fetch(CHAR * data, INT32 * max_size) {
         if (*max_size < size_) {
           return Err::kERR_BUFFER_DATA_TOO_LARGER;
         }

         ::memcpy(data, data_ + skip_, size_);
         size_ = skip_ = 0;
         *max_size = size_;

         return 0;
       }

     private:
       INT32 recycle(INT32 size) {
         INT32 capacity = ((size / 1024) + 1) * 1024;

         CHAR * data = reinterpret_cast<CHAR *>(::malloc(capacity));
         if (NULL == data) {
           return Err::kERR_BUFFER_MALLOC_FAILED;
         }

         if (0 < size_) {
           ::memcpy(data, data_, size_);
           ::free(data_);
         }

         data_ = data;
         capacity_ = capacity;

         return 0;
       }

     private:
       CHAR * data_;
       INT32 capacity_;
       INT32 size_;
       INT32 skip_;
    };
  }  // namespace cache
}  // namespace lib

#endif  // COMMLIB_CACHE_INC_BUFFER_H_
