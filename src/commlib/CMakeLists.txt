cmake_minimum_required(VERSION 2.8.0)

project(commlib)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/make)
include(common)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/../)

add_subdirectory(magic)
add_subdirectory(etc)
add_subdirectory(log)
add_subdirectory(cache)
add_subdirectory(thread)
add_subdirectory(net)
add_subdirectory(dist)
