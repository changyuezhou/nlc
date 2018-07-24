
add_subdirectory (encoders)
add_subdirectory (proto) 

add_executable(nlc nlc.c conf.c utils.c)

target_link_libraries (nlc encoders ${LIBS} ssl crypto dl)

# add the install targets
install (TARGETS nlc DESTINATION bin)
