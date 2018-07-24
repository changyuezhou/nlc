cmake_minimum_required(VERSION 2.8.0)

project(etcTest)

add_executable(etcTest
  ./test.cc
  DEPENDS etc gtest gtest_main
  LINK_LIBS etc gtest gtest_main pthread
)

