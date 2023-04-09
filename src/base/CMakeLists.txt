aux_source_directory(${PROJECT_SOURCE_DIR}/src/base SRCS)
add_library(TanBase STATIC ${SRCS})
set_target_common_compile_options(TanBase)
