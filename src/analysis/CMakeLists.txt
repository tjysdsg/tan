aux_source_directory(${PROJECT_SOURCE_DIR}/src/analysis SRCS)
add_library(TanAnalysis STATIC ${SRCS})
set_target_common_compile_options(TanAnalysis)
