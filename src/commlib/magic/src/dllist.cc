// Copyright (c) 2012 zhou chang yue. All rights reserved.

#ifndef COMMLIB_MAGIC_INC_DLLIST_CC_
#define COMMLIB_MAGIC_INC_DLLIST_CC_

#include <string>
#include "commlib/magic/inc/dllist.h"

namespace lib {
  namespace magic {
    template<class T>
    VOID DLList<T>::Insert(DLListNode * node) {
      if (NULL == node) {
        return;
      }

      if (NULL == head_) {
        head_ = node;
        head_->next_ = head_;
        head_->prev_ = head_;

        return;
      }

      node->next_ = head_;
      node->prev_ = head_->prev_;
      head_->prev_->next_ = node;
      head_->prev_ = node;
    }

    template<class T>
    VOID DLList<T>::Insert(T * data) {
      DLListNode * node = new DLListNode();

      if (NULL == node) {
        return;
      }

      node->data_ = data;

      Insert(node);
    }

    template<class T>
    VOID DLList<T>::Delete(DLListNode * node) {
      if (NULL == node || NULL == head_ || !IsInList(node)) {
        return;
      }

      if (node == head_) {
        if (head_->next_ == head_) {
          head_ = NULL;
        } else {
          head_->prev_->next_ = head_->next_;
          head_->next_->prev_ = head_->prev_;
          head_ = head_->next;
        }

        delete node;

        return;
      }

      node->prev_->next_ = node->next_;
      node->next_->prev_ = node->prev_;
    }

    template<class T>
    VOID DLList<T>::Delete(T * data) {
      Delete(Search(data));
    }

    template<class T>
    typename DLList<T>::DLListNode * DLList<T>::Search(T * data) {
      if (NULL == head_ || NULL == data) {
        return NULL;
      }

      DLListNode * p = head_;
      while (NULL != p) {
        if (data == p->data_) {
          return p;
        }
        p = p->next_;
      }

      return NULL;
    }

    template<class T>
    BOOL DLList<T>::IsInList(DLListNode * node) {
      if (NULL == head_ || NULL == node) {
        return FALSE;
      }

      if (node == head_) {
        return TRUE;
      }

      DLListNode * p = head_->next_;
      while (NULL != p && p != head_) {
        if (p == node) {
          return TRUE;
        }
        p = p->next_;
      }

      return FALSE;
    }

    template<class T>
    VOID DLList<T>::Destroy() {
      while (head_) {
        _DLListNode * p = head_;
        head_ = head_->next_;
        if (p->data_) {
          delete p->data_;
        }
        delete p;
      }
    }
  }
}

#endif  // COMMLIB_MAGIC_INC_DLLIST_CC_