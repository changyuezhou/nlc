// Copyright (c) 2013 zhou chang yue. All rights reserved.

#include <unistd.h>
#include <signal.h>
#include <string>
#include "commlib/public/inc/type.h"
#include "commlib/magic/inc/daemonize.h"
#include "commlib/magic/inc/singletonHolder.h"
#include "commlib/magic/inc/str.h"
#include "commlib/magic/src/rbtree.cc"
#include "commlib/log/inc/handleManager.h"
#include "commlib/cache/inc/index_group.h"
#include "commlib/magic/inc/md5.h"

using std::string;
using lib::magic::SingletonHolder;
using lib::log::HandleManager;
using lib::magic::RBTree;
using lib::magic::String;
using lib::cache::IndexMemInfo;


typedef IndexMemInfo::IndexNode IndexNode;
typedef RBTree<IndexNode> RBTreeTest;
typedef RBTree<IndexNode>::RBTreeNode RBTreeNode;

VOID Print(RBTreeNode * node, VOID * data) {
  if (NULL == node) {
    return;
  }

  printf("####################################################\n");
  printf("key:%s\n", node->key_);
  printf("color:%d\n", node->color_);
  if (NULL != node->parent_) {
    printf("parent:%s\n", node->parent_->key_);
  }
  if (NULL != node->left_) {
    printf("left:%s\n", node->left_->key_);
  }
  if (NULL != node->right_) {
    printf("right:%s\n", node->right_->key_);
  }
  printf("####################################################\n");
}

INT32 main(INT32 argc, CHAR ** argv) {
  RBTreeTest rb_tree;
  if (rb_tree.InitialTree()) {
    printf("initial rb tree failed  ...............................\n");
    return -1;
  }

  rb_tree.Insert("test", NULL);
  rb_tree.Insert("test1", NULL);
  rb_tree.Insert("test2", NULL);
  rb_tree.Insert("aaa", NULL);
  rb_tree.Insert("zzz", NULL);
  rb_tree.TravelMid(Print, NULL);
/*
  printf("************************************************************\n");
  rb_tree.Delete("aaa");
  rb_tree.Delete("zzz");
  rb_tree.Delete("test1");

  rb_tree.TravelMid(Print, NULL);
*/
  return 0;
}
