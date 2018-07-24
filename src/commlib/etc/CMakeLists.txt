cmake_minimum_required(VERSION 2.8.0)

project(etc)

AUX_SOURCE_DIRECTORY(./inc DIR_ETC_INC)
AUX_SOURCE_DIRECTORY(./src DIR_ETC_SRC)

ADD_LIBRARY(etc STATIC ${DIR_ETC_INC} ${DIR_ETC_SRC})

#add_subdirectory(test)
