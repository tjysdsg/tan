add_subdirectory(backtrace)
add_subdirectory(base)
add_subdirectory(ast)
add_subdirectory(common)
add_subdirectory(source_file)
add_subdirectory(lexer)
add_subdirectory(parser)
add_subdirectory(analysis)

# things that depends on LLVM
add_subdirectory(llvm_api)
add_subdirectory(codegen)
add_subdirectory(driver)
add_subdirectory(linker)

# libTan
add_subdirectory(tan)

# CLI
add_subdirectory(cli)

# ======= ENABLE sanitizers =======
# if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
#     target_compile_options(tan PUBLIC -fsanitize=address,undefined)
#     target_link_options(tan PUBLIC -fsanitize=address,undefined)
# endif ()
