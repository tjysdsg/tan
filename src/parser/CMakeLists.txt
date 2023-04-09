aux_source_directory(${PROJECT_SOURCE_DIR}/src/parser SRCS)
add_library(TanParser STATIC ${SRCS})
set_target_common_compile_options(TanParser)
