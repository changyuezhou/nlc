
add_subdirectory (encoders)
add_subdirectory (proto) 
add_subdirectory (commlib)

add_executable(lts lts.c conf.c utils.c config.cc config_api.cc version.cc version_api.cc log_api.cc)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/)

target_link_libraries (lts encoders  ${LIBS} liblog.a libetc.a libmagic.a stdc++ ssl crypto dl)

# add the install targets
install (TARGETS lts DESTINATION bin)
