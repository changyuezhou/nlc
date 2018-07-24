
add_subdirectory (encoders)
add_subdirectory (proto) 
add_subdirectory (commlib)

add_executable(nlc nlc.c conf.c utils.c config.cc config_api.cc version.cc version_api.cc log_api.cc)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/)

target_link_libraries (nlc encoders  ${LIBS} liblog.a libetc.a libmagic.a stdc++ ssl crypto dl)

# add the install targets
install (TARGETS nlc DESTINATION bin)
