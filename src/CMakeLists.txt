add_subdirectory(backtrace)
add_subdirectory(base)
add_subdirectory(ast)
add_subdirectory(lexer)
add_subdirectory(parser)
add_subdirectory(analysis)

add_subdirectory(llvm_api)
add_subdirectory(codegen)
add_subdirectory(compiler)

# ===== libtan =====
aux_source_directory(${PROJECT_SOURCE_DIR}/src/linker SRCS)
add_library(tan ${SRCS})
target_link_libraries(tan ${DEP_LIBS}
        TanBackTrace TanBase TanAST TanLexer TanParser TanAnalysis TanCodeGenerator
        TanCompiler)
set_target_common_compile_options(tan)

# ======= ENABLE sanitizers =======
# if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
#     target_compile_options(tan PUBLIC -fsanitize=address,undefined)
#     target_link_options(tan PUBLIC -fsanitize=address,undefined)
# endif ()
