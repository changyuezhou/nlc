cmake_minimum_required(VERSION 2.8.0)

project(ptmallocTest)

ADD_EXECUTABLE(ptmallocTest ./ptmalloc_test.cc)

TARGET_LINK_LIBRARIES(ptmallocTest cache_lib log magic etc pthread)

ADD_EXECUTABLE(node_group_test ./node_group_test.cc)

TARGET_LINK_LIBRARIES(node_group_test cache_lib thread log magic etc pthread)

ADD_EXECUTABLE(index_group_test ./index_group_test.cc)

TARGET_LINK_LIBRARIES(index_group_test cache_lib thread log magic etc pthread)

ADD_EXECUTABLE(rbtree_test ./rbtree_test.cc)

TARGET_LINK_LIBRARIES(rbtree_test cache_lib log magic etc pthread)