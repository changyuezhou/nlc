INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/../)

add_library(encoders STATIC quote.c quote.h url_escape.c url_escape.h identity.c protobuf.cpp pb2tsv.cpp http_query.cpp
identity.h encoders.h encoders.c protobuf.h pb2tsv.h http_query.h bin2ascii.h ${ProtoHeaders})
target_link_libraries (encoders protobuf logspec)
