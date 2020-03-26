if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    # target fuzzing
    set(CLI_SRCS ${PROJECT_SOURCE_DIR}/src/cli/tanc.cpp)
    add_executable(tanc_fuzzing ${CLI_SRCS})
    # set compiler to clang for fuzzing
    set(CMAKE_CXX_COMPILER "/usr/bin/afl-clang++")
    set(CMAKE_C_COMPILER "/usr/bin/afl-clang")
    target_link_libraries(tanc_fuzzing tan pthread gflags::gflags ${DEP_LIBS})
    set_other_options(tanc_fuzzing)
endif ()
