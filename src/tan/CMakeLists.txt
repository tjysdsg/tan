aux_source_directory(${CMAKE_CURRENT_LIST_DIR} SRCS)
add_library(Tan ${SRCS})
target_link_libraries(Tan ${DEP_LIBS}
        TanBackTrace TanBase TanAST TanLexer TanParser TanAnalysis TanCodeGenerator
        TanCompiler TanLinker)
set_target_common_compile_options(Tan)
