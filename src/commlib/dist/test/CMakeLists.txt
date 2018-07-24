cmake_minimum_required(VERSION 2.8.0)

project(electionTest)

ADD_EXECUTABLE(election_service ./election_service_test.cc)

TARGET_LINK_LIBRARIES(election_service dist_lib log magic etc thread pthread zookeeper_mt)

ADD_EXECUTABLE(election_cli ./election_cli_test.cc)

TARGET_LINK_LIBRARIES(election_cli dist_lib log magic etc thread pthread zookeeper_mt)

