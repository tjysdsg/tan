aux_source_directory(${PROJECT_SOURCE_DIR}/src/common SRCS)
add_library(TanCommon STATIC ${SRCS})
set_target_common_compile_options(TanCommon)
