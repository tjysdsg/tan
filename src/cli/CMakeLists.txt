aux_source_directory(${CMAKE_CURRENT_LIST_DIR} SRCS)
add_executable(tanc ${SRCS})
target_link_libraries(tanc Tan)
set_target_common_compile_options(tanc)
