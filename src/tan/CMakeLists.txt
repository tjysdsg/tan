aux_source_directory(${CMAKE_CURRENT_LIST_DIR} SRCS)
add_library(Tan STATIC ${SRCS})
target_link_libraries(Tan ${DEP_LIBS}
        TanBackTrace TanBase TanAST TanCommon TanLexer TanParser TanAnalysis TanCodeGenerator
        TanCompiler TanLinker TanSourceFile)
set_target_common_compile_options(Tan)
