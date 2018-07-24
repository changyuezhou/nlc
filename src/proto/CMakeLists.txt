INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/../)

file(GLOB ProtoFiles "${CMAKE_CURRENT_SOURCE_DIR}/*.proto")
PROTOBUF_GENERATE_CPP(ProtoSources ProtoHeaders ${ProtoFiles})
add_library(logspec STATIC ${ProtoSources} ${ProtoHeaders})
target_link_libraries(logspec ${PROTOBUF_LIBRARY})
target_include_directories(logspec PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
