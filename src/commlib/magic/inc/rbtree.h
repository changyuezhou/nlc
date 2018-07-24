// Copyright (c) 2013 zhou chang yue. All rights reserved.

#ifndef COMMLIB_MAGIC_INC_RBTREE_H_
#define COMMLIB_MAGIC_INC_RBTREE_H_

#include <string.h>
#include <string>
#include "commlib/public/inc/type.h"
#include "commlib/magic/inc/log.h"

namespace lib {
  namespace magic {
    using std::string;

    template <class T>
    class RBTree {
     public:
       typedef struct _RBTreeNode {
         CHAR key_[1024];
         INT32 color_;
         _RBTreeNode * parent_;
         _RBTreeNode * left_;
         _RBTreeNode * right_;
         T * data_;

         _RBTreeNode () {
           memset(key_, 0x00, sizeof(key_));
           color_ = 0;
           parent_ = NULL;
           left_ = NULL;
           right_ = NULL;
           data_ = NULL;
         }
       } RBTreeNode;

      typedef VOID (*_operating)(RBTreeNode * node, VOID * data);

     public:
       static const INT32 kCOLOR_BLACK = 1;
       static const INT32 kCOLOR_RED = 2;
       static const INT32 kERROR_ROOT_IS_NOT_BLACK = 1;
       static const INT32 kERROR_COLOR_INVALID = 2;
       static const INT32 kERROR_HEIGHT_OF_BLACK = 3;
       static const INT32 kERROR_RED_PARENT_IS_RED = 4;
       static const INT32 kERROR_VOLATILE_BINARY_SEARCH_PROPERTY = 5;

     public:
       explicit RBTree(BOOL need_free_data = FALSE):need_free_data_(need_free_data), root_(NULL), sentinel_(NULL), size_(0) {}
       ~RBTree() { Destroy(); }

     public:
       inline VOID BlackNode(RBTreeNode * node) {
         if (node) {
           node->color_ = kCOLOR_BLACK;
         }
       }

       inline VOID RedNode(RBTreeNode * node) {
         if (node) {
           node->color_ = kCOLOR_RED;
         }
       }

       inline BOOL IsRedNode(RBTreeNode * node) {
         if (node) {
           return (node->color_ == kCOLOR_RED);
         }

         return FALSE;
       }

       inline BOOL IsBlackNode(RBTreeNode * node) {
         if (node) {
           return (node->color_ == kCOLOR_BLACK);
         }

         return FALSE;
       }

       inline VOID ClearNode(RBTreeNode * node) {
         if (node) {
           node->key = 0;
           node->color_ = 0;
           node->parent_ = NULL;
           node->left_ = NULL;
           node->right_ = NULL;
           node->data_ = NULL;
         }
       }

       inline VOID IsTreeEmpty() {
         return (NULL == root_);
       }

       inline INT32 size() {
         return size_;
       }

       inline RBTreeNode * MinNode() {
         if (NULL == root_) {
           return NULL;
         }

         RBTreeNode * node = root_;
         while (NULL != node->left_) {
           node = node->left_;
         }

         return node;
       }

       inline RBTreeNode * MaxNode() {
         if (NULL == root_) {
           return NULL;
         }

         RBTreeNode * node = root_;
         while (NULL != node->right_) {
           node = node->right_;
         }

         return node;
       }

       inline RBTreeNode * Root() {
         return root_;
       }

     public:
       INT32 InitialTree();
       VOID Insert(RBTreeNode * node);
       VOID Delete(RBTreeNode * node);
       VOID Insert(const string & key, T * data);
       VOID Delete(const string & key);
       RBTreeNode * Search(const string & key);
       RBTreeNode * upper_bound_first(const string & key);
       RBTreeNode * lower_bound_first(const string & key);
       RBTreeNode * SubTreeMin(RBTreeNode * root, RBTreeNode * sentinel);
       RBTreeNode * SubTreeMax(RBTreeNode * root, RBTreeNode * sentinel);
       VOID TravelMid(_operating operating, VOID * data);
       INT32 TreeSize() { return size_; }

     protected:
       VOID RotateL(RBTreeNode * node);
       VOID RotateR(RBTreeNode * node);

     protected:
       VOID InsertFixup(RBTreeNode * node);
       VOID DeleteFixup(RBTreeNode * node);

       VOID Transplant(RBTreeNode * parent, RBTreeNode * child);
       VOID FixupWhereUncleIsRed(RBTreeNode * * node, RBTreeNode * uncle);

       VOID TravelMid(RBTreeNode * node, _operating operating, VOID * data);
       VOID TravelMidFree(RBTreeNode * node);

     protected:
       RBTreeNode * GetInsertPosition(RBTreeNode * node);
       RBTreeNode * GetInsertPosition(const string & key);
       RBTreeNode * GetUncleNode(RBTreeNode * node);

     protected:
       BOOL IsLeftChild(RBTreeNode * node);
       BOOL IsRightChild(RBTreeNode * node);
       BOOL IsLeaf(RBTreeNode * node);

     public:
       static VOID FreeNode(RBTreeNode * node, VOID * data);

     public:
       BOOL IsRBTree();

     public:
       VOID FreeTree();

       VOID Destroy() {
         FreeTree();
       }

     private:
       BOOL need_free_data_;
       RBTreeNode * root_;
       RBTreeNode * sentinel_;
       INT32 size_;
    };
  }  // namespace magic
}  // namespace lib

#endif  // COMMLIB_MAGIC_INC_RBTREE_H_