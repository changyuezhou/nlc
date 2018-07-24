// Copyright (c) 2013 zhou chang yue. All rights reserved.

#ifndef COMMLIB_MAGIC_INC_DLLIST_H_
#define COMMLIB_MAGIC_INC_DLLIST_H_

#include <string.h>
#include <string>
#include "commlib/public/inc/type.h"
#include "commlib/magic/inc/log.h"

namespace lib {
  namespace magic {
    template<class T>
    class DLList {
     public:
       typedef struct _DLListNode {
         _DLListNode *next_;
         _DLListNode *prev_;
         T * data_;

         _DLListNode() {
           next_ = NULL;
           prev_ = NULL;
           data_ = NULL;
         }
       } DLListNode;

     public:
       DLList(): head_(NULL) {}
       ~DLList() { Destroy(); }

     public:
       VOID Insert(DLListNode * node);
       VOID Insert(T * data);
       VOID Delete(DLListNode * node);
       VOID Delete(T * data);
       DLListNode * Search(T * data);

     protected:
       BOOL IsInList(DLListNode * node);

     public:
       VOID Destroy();

     private:
       _DLListNode * head_;
    };
  }
}

#endif  // COMMLIB_MAGIC_INC_DLLIST_H_