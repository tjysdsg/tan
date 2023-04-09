aux_source_directory(${PROJECT_SOURCE_DIR}/src/llvm_api SRCS)
add_library(TanLLVMAPI STATIC ${SRCS})
set_target_common_compile_options(TanLLVMAPI)
