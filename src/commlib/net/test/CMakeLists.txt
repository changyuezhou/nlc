cmake_minimum_required(VERSION 2.8.0)

project(netTest)

ADD_EXECUTABLE(service ./service.cc)

TARGET_LINK_LIBRARIES(service net log magic etc pthread)

#ADD_EXECUTABLE(dgram_service ./dgram_service.cc)

#TARGET_LINK_LIBRARIES(dgram_service net)

