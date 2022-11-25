aux_source_directory(${PROJECT_SOURCE_DIR}/src/lexer SRCS)
add_library(TanLexer ${SRCS})
set_target_common_compile_options(TanLexer)
