
add_subdirectory (encoders)
add_subdirectory (proto) 

add_executable(lts lts.c conf.c utils.c config.cc config_api.cc version.cc version_api.cc log_api.cc)


#INCLUDE_DIRECTORIES(/bruce/project/ads)
INCLUDE_DIRECTORIES($(pwd)/include)

#LINK_DIRECTORIES(/bruce/project/test_lib/commlib/result)

#target_link_libraries (lts encoders ${LIBS} libmain.a stdc++)
target_link_libraries (lts encoders  ${LIBS} libcommlib.a stdc++)

# add the install targets
install (TARGETS lts DESTINATION bin)
