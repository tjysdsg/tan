aux_source_directory(${CMAKE_CURRENT_LIST_DIR} SRCS)
add_library(Tan STATIC ${SRCS})
target_link_libraries(Tan
        TanDriver TanLinker
        TanCodeGenerator TanAnalysis TanParser TanLexer
        TanAST TanSourceFile TanCommon TanBase
        TanBackTrace TanLLVMAPI
        ${DEP_LIBS}
)
set_target_common_compile_options(Tan)
