aux_source_directory(${PROJECT_SOURCE_DIR}/src/linker SRCS)
add_library(TanLinker STATIC ${SRCS})
set_target_common_compile_options(TanLinker)
