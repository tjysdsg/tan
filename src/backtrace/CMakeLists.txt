aux_source_directory(${PROJECT_SOURCE_DIR}/src/backtrace SRCS)

if (WIN32)
    add_library(TanBackTrace ${SRCS})
else ()
    add_library(TanBackTrace SHARED ${SRCS})
endif ()

add_dependencies(TanBackTrace libbacktrace) # libbacktrace custom target defined in dep/CMakeLists.txt
target_include_directories(TanBackTrace PRIVATE ${LIBBACKTRACE_INCLUDE_DIRS})
target_link_libraries(TanBackTrace ${LIBBACKTRACE_LIBS})

set_target_common_compile_options(TanBackTrace)
