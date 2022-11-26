aux_source_directory(${PROJECT_SOURCE_DIR}/src/compiler SRCS)
add_library(TanCompiler ${SRCS})
set_target_common_compile_options(TanCompiler)
target_link_libraries(TanCompiler TanLLVMAPI)
