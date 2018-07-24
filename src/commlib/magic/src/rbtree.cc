// Copyright (c) 2012 zhou chang yue. All rights reserved.

#ifndef COMMLIB_MAGIC_INC_RBTREE_CC_
#define COMMLIB_MAGIC_INC_RBTREE_CC_

#include <string>
#include "commlib/magic/inc/rbtree.h"

namespace lib {
  namespace magic {
    template<class T>
    INT32 RBTree<T>::InitialTree() {
      sentinel_ = new RBTreeNode();
      if (NULL == sentinel_) {
        return -1;
      }

      BlackNode(sentinel_);
      root_ = sentinel_;

      return 0;
    }

    template<class T>
    VOID RBTree<T>::Insert(RBTreeNode * node) {
      if (NULL == node) {
        return;
      }

      RBTreeNode * pos = GetInsertPosition(node);
      if (NULL == pos) {
        root_ = node;
      } else {
        INT32 length = ::strlen(pos->key_);
        INT32 length_node = ::strlen(node->key_);
        if (length < length_node) {
          length = length_node;
        }
        node->parent_ = pos;
        if (0 < ::strncmp(pos->key_, node->key_, length)) {
          pos->left_ = node;
        } else {
          pos->right_ = node;
        }
      }

      node->left_ = sentinel_;
      node->right_ = sentinel_;

      node->color_ = kCOLOR_RED;

      InsertFixup(node);
      size_++;
    }

    template<class T>
    VOID RBTree<T>::Delete(RBTreeNode * node) {
      if (NULL == node || NULL == root_ || root_ == sentinel_) {
        LIB_MAGIC_LOG_WARN("Tree node did not need delete  ............................................");

        return;
      }

      RBTreeNode * need_del = node;
      INT32 color = need_del->color_;
      RBTreeNode * child_node = NULL;
      if (sentinel_ == node->left_ || NULL == node->left_) {
        child_node = node->right_;
        Transplant(need_del, child_node);
      } else if (sentinel_ == node->right_ || NULL == node->right_) {
        child_node = node->left_;
        Transplant(need_del, child_node);
      } else {
        need_del = SubTreeMin(node->right_, sentinel_);
        if (NULL == need_del) {
          LIB_MAGIC_LOG_WARN("Tree node did not need delete  ............................................");
          return;
        }

        color = need_del->color_;
        child_node = need_del->right_;

        if (need_del->parent_ == node) {
          child_node->parent_ = need_del;
        } else {
          Transplant(need_del, need_del->right_);
          need_del->right_ = node->right_;
          need_del->right_->parent_ = need_del;
        }

        Transplant(node, need_del);

        need_del->left_ = node->left_;
        need_del->left_->parent_ = need_del;
        need_del->color_ = node->color_;
      }

      if (kCOLOR_BLACK == color) {
        DeleteFixup(child_node);
      }

      if (need_free_data_) {
        delete node->data_;
      }
      delete node;

      size_--;
      if (0 >= size_) {
        root_ = sentinel_;
      }
    }

    template<class T>
    VOID RBTree<T>::Insert(const string & key, T * data) {
      RBTreeNode  * node = new RBTreeNode();
      if (NULL == node) {
        return;
      }

      node->data_ = data;
      memcpy(node->key_, key.c_str(), key.length());
      node->color_ = kCOLOR_RED;
      node->right_ = NULL;
      node->left_ = NULL;
      node->parent_ = NULL;

      Insert(node);
    }

    template<class T>
    VOID RBTree<T>::Delete(const string & key) {
      RBTreeNode * node = Search(key);
      if (node) {
        Delete(node);
      }
    }

    template<typename T>
    typename RBTree<T>::RBTreeNode * RBTree<T>::Search(const string & key) {
      if (NULL == root_ || sentinel_ == root_ || 0 >= key.length()) {
        return NULL;
      }

      RBTreeNode * cur = root_;
      while (sentinel_ != cur && NULL != cur) {
        INT32 length = ::strlen(cur->key_);
        if (length < key.length()) {
          length = key.length();
        }

        if (0 < ::strncmp(key.c_str(), cur->key_, length)) {
          cur = cur->right_;
        } else if (0 > ::strncmp(key.c_str(), cur->key_, length)) {
          cur = cur->left_;
        } else if (0 == ::strncmp(key.c_str(), cur->key_, length)) {
          return cur;
        }
      }

      return NULL;
    }

    template<class T>
    typename RBTree<T>::RBTreeNode * RBTree<T>::upper_bound_first(const string & key) {
      if (NULL == root_ || sentinel_ == root_) {
        return NULL;
      }

      RBTreeNode * cur = root_;
      while (NULL != cur && sentinel_ != cur) {
        INT32 length = ::strlen(cur->key_);
        if (length < key.length()) {
          length = key.length();
        }

        INT32 length_left = key.length();
        if (NULL != cur->left_) {
          INT32 len = ::strlen(cur->left_->key_);
          if (length_left < len) {
            length_left = len;
          }
        }

        if (0 > ::strncmp(key.c_str(), cur->key_, length)) {
          if (NULL == cur->left_ || sentinel_ == cur->left_) {
            return cur;
          } else if (0 < ::strncmp(key.c_str(), cur->left_->key_, length_left)) {
            RBTreeNode * max = SubTreeMax(cur->left_, sentinel_);
            if (NULL == max) {
              return cur;
            }

            length = ::strlen(max->key_);
            if (length < key.length()) {
              length = key.length();
            }
            if (0 < ::strncmp(key.c_str(), max->key_, length)) {
              return cur;
            }

            cur = cur->left_;
          } else {
            cur = cur->left_;
          }
        } else if (0 == ::strncmp(key.c_str(), cur->key_, length)) {
          return cur;
        } else {
          if (NULL == cur->right_ || sentinel_ == cur->right_) {
            return NULL;
          } else {
            cur = cur->right_;
          }
        }
      }

      return NULL;
    }

    template<class T>
    typename RBTree<T>::RBTreeNode * RBTree<T>::lower_bound_first(const string & key) {
      if (NULL == root_ || sentinel_ == root_) {
        return NULL;
      }

      RBTreeNode * cur = root_;
      while (NULL != cur && sentinel_ != cur) {
        INT32 length = ::strlen(cur->key_);
        if (length < key.length()) {
          length = key.length();
        }

        INT32 length_right = key.length();
        if (NULL != cur->right_) {
          INT32 len = ::strlen(cur->right_->key_);
          if (length_right < len) {
            length_right = len;
          }
        }

        if (0 < ::strncmp(key.c_str(), cur->key_, length)) {  // key > cur
          if (NULL == cur->right_ || sentinel_ == cur->right_) {
            return cur;
          } else if (0 > ::strncmp(key.c_str(), cur->right_->key_, length_right)) {  // key < cur->right
            RBTreeNode * min = SubTreeMin(cur->right_, sentinel_);
            if (NULL == min) {
              return cur;
            }

            length = ::strlen(min->key_);
            if (length < key.length()) {
              length = key.length();
            }
            if (0 > ::strncmp(key.c_str(), min->key_, length)) {  // key < min
              return cur;
            }

            cur = cur->right_;
          } else {
            cur = cur->right_;
          }
        } else if (0 == ::strncmp(key.c_str(), cur->key_, length)) {
          return cur;
        } else {  // key < cur
          if (NULL == cur->left_ || sentinel_ == cur->left_) {
            return NULL;
          } else {
            cur = cur->left_;
          }
        }
      }

      return NULL;
    }

    template<class T>
    typename RBTree<T>::RBTreeNode * RBTree<T>::SubTreeMin(RBTreeNode * root, RBTreeNode * sentinel) {
      if (NULL == root) {
        return NULL;
      }

      RBTreeNode * cur = root;
      while (NULL != cur->left_ && sentinel != cur->left_) {
        cur = cur->left_;
      }

      return cur;
    }

    template<class T>
    typename RBTree<T>::RBTreeNode * RBTree<T>::SubTreeMax(RBTreeNode * root, RBTreeNode * sentinel) {
      if (NULL == root) {
        return NULL;
      }

      RBTreeNode * cur = root;
      while (NULL != cur->right_ && sentinel != cur->right_) {
        cur = cur->right_;
      }

      return cur;
    }

    template<class T>
    VOID RBTree<T>::TravelMid(_operating operating, VOID * data) {
      TravelMid(root_, operating, data);
    }

    template<class T>
    VOID RBTree<T>::TravelMid(RBTreeNode * node, _operating operating, VOID * data) {
      if (NULL == node || sentinel_ == node) {
        return;
      }

      if (NULL != node->left_ && sentinel_ != node->left_) {
        TravelMid(node->left_, operating, data);
      }

      operating(node, data);

      if (NULL != node->right_ && sentinel_ != node->right_) {
        TravelMid(node->right_, operating, data);
      }
    }

    template<class T>
    VOID RBTree<T>::FreeNode(RBTreeNode * node, VOID * data) {
      if (NULL == node || NULL == data) {
        return;
      }

      BOOL need_free_data = *(reinterpret_cast<BOOL *>(data));

      if (NULL != node->data_ && (TRUE == need_free_data)) {
        delete node->data_;
      }

      delete node;
    }

    template<class T>
    VOID RBTree<T>::FreeTree() {
      while (NULL != root_ && sentinel_ != root_) {
        Delete(root_);
      }

      if (sentinel_) {
        delete sentinel_;
        sentinel_ = NULL;
      }
    }

    template<class T>
    typename RBTree<T>::RBTreeNode * RBTree<T>::GetInsertPosition(RBTreeNode * node) {
      if (NULL == root_ || NULL == node || root_ == sentinel_) {
        return NULL;
      }

      RBTreeNode * p = root_;
      while (NULL != p && sentinel_ != p) {
        INT32 length = ::strlen(node->key_);
        INT32 length_p = ::strlen(p->key_);
        if (length < length_p) {
          length = length_p;
        }
        if (0 < ::strncmp(node->key_, p->key_, length)) {
          if (NULL == p->right_ || sentinel_ == p->right_) {
            return p;
          }
          p = p->right_;
        } else {
          if (NULL == p->left_ || sentinel_ == p->left_) {
            return p;
          }
          p = p->left_;
        }
      }

      return p;
    }

    template<class T>
    typename RBTree<T>::RBTreeNode * RBTree<T>::GetInsertPosition(const string & key) {
      if (NULL == root_) {
        return NULL;
      }

      RBTreeNode * p = root_;
      while (NULL != p && sentinel_ != p) {
        INT32 length = ::strlen(p->key_);
        if (length < key.length()) {
          length = key.length();
        }

        if (0 < ::strncmp(key.c_str(), p->key, length)) {
          if (NULL == p->right_ || sentinel_ == p->right_) {
            return p;
          }
          p = p->right_;
        } else {
          if (NULL == p->left_ || sentinel_ == p->left_) {
            return p;
          }
          p = p->left_;
        }
      }

      return p;
    }

    template<class T>
    BOOL RBTree<T>::IsLeftChild(RBTreeNode * node) {
      if (NULL == node || NULL == node->parent_) {
        return FALSE;
      }

      if (node == node->parent_->left_) {
        return TRUE;
      }

      return FALSE;
    }

    template<class T>
    BOOL RBTree<T>::IsRightChild(RBTreeNode * node) {
      if (NULL == node || NULL == node->parent_) {
        return FALSE;
      }

      if (node == node->parent_->right_) {
        return TRUE;
      }

      return FALSE;
    }

    template<class T>
    BOOL RBTree<T>::IsLeaf(RBTreeNode * node) {
      if (NULL == node) {
        return FALSE;
      }

      if (sentinel_ != node->left_ || sentinel_ != node->right_) {
        return FALSE;
      }

      return TRUE;
    }

    template<class T>
    typename RBTree<T>::RBTreeNode * RBTree<T>::GetUncleNode(RBTreeNode * node) {
      if (node->parent_ == node->parent_->parent_->left_) {
        return node->parent_->parent_->right_;
      }

      return node->parent_->parent_->left_;
    }

    template<class T>
    VOID RBTree<T>::Transplant(RBTreeNode * parent, RBTreeNode * child) {
      if (NULL == parent || NULL == child) {
        return;
      }

      if (NULL == parent->parent_) {
        root_ = child;
      } else if (IsLeftChild(parent)) {
        parent->parent_->left_ = child;
      } else {
        parent->parent_->right_ = child;
      }

      child->parent_ = parent->parent_;
    }

    template<class T>
    VOID RBTree<T>::FixupWhereUncleIsRed(RBTreeNode * * node, RBTreeNode * uncle) {
      (*node)->parent_->color_ = kCOLOR_BLACK;
      uncle->color_ = kCOLOR_BLACK;
      (*node)->parent_->parent_->color_ = kCOLOR_RED;
      (*node) = (*node)->parent_->parent_;
    }

    template<class T>
    VOID RBTree<T>::InsertFixup(RBTreeNode * node) {
      RBTreeNode * cur = node;
      while (IsRedNode(cur->parent_)) {
        RBTreeNode * p = cur->parent_;
        RBTreeNode * uncle = GetUncleNode(cur);
        if (IsLeftChild(p)) {
          if (IsRedNode(uncle)) {
            FixupWhereUncleIsRed(&cur, uncle);
          } else {
            if (IsRightChild(cur)) {
              cur = p;
              RotateL(cur);
              p = cur->parent_;
            }

            p->color_ = kCOLOR_BLACK;
            p->parent_->color_ = kCOLOR_RED;
            RotateR(p->parent_);
          }
        } else {
          if (IsRedNode(uncle)) {
            FixupWhereUncleIsRed(&cur, uncle);
          } else {
            if (IsLeftChild(cur)) {
              cur = p;
              RotateR(cur);
              p = cur->parent_;
            }

            p->color_ = kCOLOR_BLACK;
            p->parent_->color_ = kCOLOR_RED;
            RotateL(p->parent_);
          }
        }
      }

      BlackNode(root_);
    }

    template<class T>
    VOID RBTree<T>::DeleteFixup(RBTreeNode * node) {
      while (node != root_ && IsBlackNode(node)) {
        if (IsLeftChild(node)) {
          RBTreeNode * brother = node->parent_->right_;
          if (IsRedNode(brother)) {
            BlackNode(brother);
            RedNode(node->parent_);
            RotateL(node->parent_);
            brother = node->parent_->right_;
          }

          if (IsBlackNode(brother->left_) && IsBlackNode(brother->right_)) {
            RedNode(brother);
            node = node->parent_;
          } else {
            if (IsBlackNode(brother->right_)) {
              BlackNode(brother->left_);
              RedNode(brother);
              RotateR(brother);
              brother = node->parent_->right_;
            }

            brother->color_ = node->parent_->color_;
            BlackNode(node->parent_);
            BlackNode(brother->right_);
            RotateL(node->parent_);
            node = root_;
          }
        } else {
          RBTreeNode * brother = node->parent_->left_;
          if (IsRedNode(brother)) {
            BlackNode(brother);
            RedNode(node->parent_);
            RotateR(node->parent_);
            brother = node->parent_->left_;
          }

          if (IsBlackNode(brother->left_) && IsBlackNode(brother->right_)) {
            RedNode(brother);
            node = node->parent_;
          } else {
            if (IsBlackNode(brother->left_)) {
              BlackNode(brother->right_);
              RedNode(brother);
              RotateL(brother);
              brother = node->parent_->left_;
            }

            brother->color_ = node->parent_->color_;
            BlackNode(node->parent_);
            BlackNode(brother->left_);
            RotateR(node->parent_);
            node = root_;
          }
        }
      }

      BlackNode(node);
    }

    template<class T>
    VOID RBTree<T>::RotateL(RBTreeNode * node) {
      if (NULL == node || NULL == node->right_) {
        return;
      }

      RBTreeNode * y = node->right_;
      node->right_ = y->left_;
      if (NULL != y->left_) {
        y->left_->parent_ = node;
      }

      y->parent_ = node->parent_;

      if (NULL == node->parent_) {
        root_ = y;
      } else {
        if (IsLeftChild(node)) {
          node->parent_->left_ = y;
        } else {
          node->parent_->right_ = y;
        }
      }

      y->left_ = node;
      node->parent_ = y;
    }

    template<class T>
    VOID RBTree<T>::RotateR(RBTreeNode * node) {
      if (NULL == node || NULL == node->left_) {
        return;
      }

      RBTreeNode * y = node->left_;
      node->left_ = y->right_;
      if (NULL != y->right_) {
        y->right_->parent_ = node;
      }

      y->parent_ = node->parent_;

      if (NULL == node->parent_) {
        root_ = y;
      } else {
        if (IsLeftChild(node)) {
          node->parent_->left_ = y;
        } else {
          node->parent_->right_ = y;
        }
      }

      y->right_ = node;
      node->parent_ = y;
    }
  }  // namespace magic
}  // namespace lib

#endif  // COMMLIB_MAGIC_INC_RBTREE_CC_