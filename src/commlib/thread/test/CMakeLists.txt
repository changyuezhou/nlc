cmake_minimum_required(VERSION 2.8.0)

project(threadTest)

ADD_EXECUTABLE(thread_test ./main_thread.cc)

TARGET_LINK_LIBRARIES(thread_test thread log etc magic pthread)


ADD_EXECUTABLE(process_test ./main_process.cc)

TARGET_LINK_LIBRARIES(process_test thread log etc magic pthread)