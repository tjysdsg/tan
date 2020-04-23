cmake_minimum_required(VERSION 3.0)

# set version number
set(TAN_VERSION 0.0.1)
string(REPLACE "." ";" VERSION_LIST ${TAN_VERSION})
list(GET VERSION_LIST 0 PROJECT_VERSION_MAJOR)
list(GET VERSION_LIST 1 PROJECT_VERSION_MINOR)
list(GET VERSION_LIST 2 PROJECT_VERSION_PATCH)
set(CMAKE_PROJECT_TAN_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CMAKE_PROJECT_TAN_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CMAKE_PROJECT_TAN_VERSION_PATCH "${PROJECT_VERSION_PATCH}")

set(CMAKE_PROJECT_TAN_MAX_N_FUNCTION_CALLS 256)

project(tan LANGUAGES CXX C VERSION ${TAN_VERSION})

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

# generate include/config.h using include/config.h.in
configure_file(${PROJECT_SOURCE_DIR}/include/config.h.in ${PROJECT_SOURCE_DIR}/include/config.h)

# default build debug
# use -DCMAKE_BUILD_TYPE=Release to build Release version
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif ()

macro(set_other_options target_name)
    message(${target_name})
    target_compile_features(${target_name} PRIVATE cxx_std_17)
    target_compile_options(${target_name}
            PRIVATE -Wall
            PRIVATE -Wextra
            PRIVATE -pedantic-errors
            PRIVATE -Wconversion
            )
    target_include_directories(${target_name} PRIVATE ${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/include)

    if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
        # add `DEBUG` and `_DEBUG` macro
        target_compile_definitions(${target_name} PRIVATE -DDEBUG)
        target_compile_definitions(${target_name} PRIVATE -D_DEBUG)
    else ()
        target_compile_options(${target_name} PRIVATE -O3)
    endif ()
    set_target_properties(${target_name} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
    set_target_properties(${target_name} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
    set_target_properties(${target_name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
endmacro()

# Use ccache to compile if possible
find_program(CCACHE_FOUND ccache)
if (CCACHE_FOUND)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
endif (CCACHE_FOUND)

# ================================ thirdparty dependencies ========================== #
# ===== Google test ======
add_subdirectory(${PROJECT_SOURCE_DIR}/dep/googletest)

# ===== gflags =====
add_subdirectory(${PROJECT_SOURCE_DIR}/dep/gflags)
include_directories(${GFLAGS_INCLUDE_DIR})

# ===== others ======
add_subdirectory(${PROJECT_SOURCE_DIR}/dep)
include_directories(${DEP_INCLUDES})
link_directories(${DEP_LIB_DIRS})
# ================================ thirdparty dependencies end ========================== #

# common source files shared by multiple targets
aux_source_directory(${PROJECT_SOURCE_DIR}/src/base COMMON_SRCS)
aux_source_directory(${PROJECT_SOURCE_DIR}/src/lexer COMMON_SRCS)
aux_source_directory(${PROJECT_SOURCE_DIR}/src/parser COMMON_SRCS)
aux_source_directory(${PROJECT_SOURCE_DIR}/src/compiler COMMON_SRCS)
aux_source_directory(${PROJECT_SOURCE_DIR}/src/ast COMMON_SRCS)
aux_source_directory(${PROJECT_SOURCE_DIR}/src/linker COMMON_SRCS)

# ====== tests for debug build ======
if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    # target test
    aux_source_directory(${PROJECT_SOURCE_DIR}/src/test TEST_SRCS)
    add_executable(tan_tests ${TEST_SRCS})
    target_include_directories(tan_tests PRIVATE ${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/include
            ${PROJECT_SOURCE_DIR}/src/test)
    target_link_libraries(tan_tests tan gtest_main ${DEP_LIBS})
    set_other_options(tan_tests)

    add_test(NAME cmake_reader_test COMMAND tan_tests WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/src/test)
    enable_testing()
endif ()

# ===== target libtan =====
aux_source_directory(${PROJECT_SOURCE_DIR}/src/lib LIB_SRCS)
add_library(tan ${COMMON_SRCS} ${LIB_SRCS})
target_link_libraries(tan pthread ${DEP_LIBS} stdc++fs)
set_other_options(tan)

# ===== target CLI compiler =====
set(CLI_SRCS ${PROJECT_SOURCE_DIR}/src/cli/tanc.cpp)
add_executable(tanc ${CLI_SRCS})
target_link_libraries(tanc tan pthread gflags::gflags ${DEP_LIBS})
set_other_options(tanc)

add_subdirectory(${PROJECT_SOURCE_DIR}/runtime)
