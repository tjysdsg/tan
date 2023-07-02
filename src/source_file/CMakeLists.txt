aux_source_directory(${PROJECT_SOURCE_DIR}/src/source_file SRCS)
add_library(TanSourceFile STATIC ${SRCS})
set_target_common_compile_options(TanSourceFile)
