cmake_minimum_required (VERSION 2.8)
project (lts)
# The version number.
set (lts_VERSION_MAJOR 0)
set (lts_VERSION_MINOR 1)

execute_process(
  COMMAND git describe --abbrev=12 --dirty --always
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE _GIT_VERSION
  OUTPUT_STRIP_TRAILING_WHITESPACE
 )

IF (NOT DEFINED _GIT_VERSION)
    SET(_GIT_VERSION "version-not-defined")
ENDIF()
ADD_DEFINITIONS(-DVERSION=\"${_GIT_VERSION}\")

string(TIMESTAMP _BUILD_TIME "%Y-%m-%d %H:%M:%S UTC" UTC)
ADD_DEFINITIONS(-DBUILD_TIME=\"${_BUILD_TIME}\")


execute_process(
  COMMAND git describe --abbrev=12 --dirty --always --tag
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE _GIT_TAG
  OUTPUT_STRIP_TRAILING_WHITESPACE
 )
IF (NOT DEFINED _GIT_TAG)
    SET(_GIT_TAG "tag-not-defined")
ENDIF()
ADD_DEFINITIONS(-DTAG=\"${_GIT_TAG}\")

# Enable static build
SET(KAFKA_LINK_STATIC TRUE)
#set_target_properties(icarus PROPERTIES LINK_SEARCH_START_STATIC 1)
#set_target_properties(icarus PROPERTIES LINK_SEARCH_END_STATIC 1)
SET(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
set(CMAKE_EXE_LINKER_FLAGS "-static-libstdc++ -pthread $ENV{LDFLAGS}")

# Load local cmake modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

# Set compiler flags
set (CMAKE_C_FLAGS "-m64 -std=gnu99 -Wall -Wextra -pedantic -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64")
set (CMAKE_C_FLAGS_DEBUG "-g")
set (CMAKE_C_FLAGS_RELEASE "-O2")
set (CMAKE_CXX_FLAGS "-std=c++11 -m64 -Wall -Wextra -pedantic -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64")
set (CMAKE_CXX_FLAGS_DEBUG "-g")
set (CMAKE_CXX_FLAGS_RELEASE "-O2")


# Set external dependencies


find_library(LIBC libc.a)
find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIRS})
set(LIBS ${LIBS} ${PROTOBUF_LIBRARY})
find_package(Jansson REQUIRED)
include_directories(${JANSSON_INCLUDE_DIRS})
set(LIBS ${LIBS} ${JANSSON_LIBRARIES})
find_package(KafkaC REQUIRED)
include_directories(${KAFKA_INCLUDE_DIRS})
set(LIBS ${LIBS} ${KAFKA_LIBRARIES})
find_package(ZLIB REQUIRED)
set(LIBS ${LIBS} ${ZLIB_LIBRARIES})


# configure a header file to pass some of the CMake settings
# to the source code
configure_file (
  "${PROJECT_SOURCE_DIR}/src/config.h.in"
  "${PROJECT_BINARY_DIR}/config.h"
  )

# add the binary tree to the search path for include files
# so that we will find config.h
include_directories("${PROJECT_BINARY_DIR}")

add_subdirectory (src)

# build a CPack driven installer package
include (InstallRequiredSystemLibraries)
set (CPACK_RESOURCE_FILE_LICENSE
     "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set (CPACK_PACKAGE_VERSION_MAJOR "${lts_VERSION_MAJOR}")
set (CPACK_PACKAGE_VERSION_MINOR "${lts_VERSION_MINOR}")
include (CPack)
