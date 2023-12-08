aux_source_directory(${PROJECT_SOURCE_DIR}/src/driver SRCS)
add_library(TanDriver STATIC ${SRCS})
set_target_common_compile_options(TanDriver)
