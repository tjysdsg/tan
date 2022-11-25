aux_source_directory(${PROJECT_SOURCE_DIR}/src/ast SRCS)

add_library(ast ${SRCS})
set_target_common_compile_options(ast)
