aux_source_directory(${PROJECT_SOURCE_DIR}/src/llvm_api SRCS)
add_library(TanLLVMAPI ${SRCS})
set_target_common_compile_options(TanLLVMAPI)

if (${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.16" AND ${ENABLE_PRECOMPILED_HEADERS})
    message(STATUS "Precompiled include/llvm_api.h")
    target_precompile_headers(TanLLVMAPI PUBLIC ${PROJECT_SOURCE_DIR}/include/llvm_api.h)
endif ()

