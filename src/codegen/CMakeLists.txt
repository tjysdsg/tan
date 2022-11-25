aux_source_directory(${PROJECT_SOURCE_DIR}/src/codegen SRCS)
add_library(TanCodeGenerator ${SRCS})
set_target_common_compile_options(TanCodeGenerator)
