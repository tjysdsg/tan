aux_source_directory(${PROJECT_SOURCE_DIR}/src/linker SRCS)
add_library(TanLinker ${SRCS})
set_target_common_compile_options(TanLinker)
target_link_libraries(TanLinker TanLLVMAPI)
