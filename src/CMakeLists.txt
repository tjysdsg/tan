add_subdirectory(ast)

# common source files shared by multiple targets
aux_source_directory(${PROJECT_SOURCE_DIR}/src/base COMMON_SRCS)
aux_source_directory(${PROJECT_SOURCE_DIR}/src/lexer COMMON_SRCS)
aux_source_directory(${PROJECT_SOURCE_DIR}/src/parser COMMON_SRCS)
aux_source_directory(${PROJECT_SOURCE_DIR}/src/analysis COMMON_SRCS)
aux_source_directory(${PROJECT_SOURCE_DIR}/src/compiler COMMON_SRCS)
aux_source_directory(${PROJECT_SOURCE_DIR}/src/linker COMMON_SRCS)
aux_source_directory(${PROJECT_SOURCE_DIR}/src/codegen COMMON_SRCS)
aux_source_directory(${PROJECT_SOURCE_DIR}/src COMMON_SRCS)

# ===== libtan =====
aux_source_directory(${PROJECT_SOURCE_DIR}/src/lib LIB_SRCS)
add_library(tan ${COMMON_SRCS} ${LIB_SRCS})
target_link_libraries(tan ${DEP_LIBS} tan_backtrace ast)
set_target_common_compile_options(tan)

# ======= ENABLE sanitizers =======
if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    target_compile_options(tan PUBLIC -fsanitize=address,undefined)
    target_link_options(tan PUBLIC -fsanitize=address,undefined)
endif ()

# ======= precompile headers =======
if (${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.16" AND ${ENABLE_PRECOMPILED_HEADERS})
    message(STATUS "precompiled headers are enabled")
    target_precompile_headers(tan PRIVATE
            ${PROJECT_SOURCE_DIR}/src/llvm_include.h
            ${PROJECT_SOURCE_DIR}/include/base.h
            ${PROJECT_SOURCE_DIR}/include/config.h
            ${PROJECT_SOURCE_DIR}/include/clang-frontend.h)
endif ()

