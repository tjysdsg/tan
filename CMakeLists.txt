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

project(tan LANGUAGES CXX VERSION ${TAN_VERSION})

# generate include/config.h using include/config.h.in
configure_file(${PROJECT_SOURCE_DIR}/include/config.h.in ${PROJECT_SOURCE_DIR}/include/config.h)

# C++17
set(CMAKE_CXX_STANDARD 17)

# default build debug
# use -DCMAKE_BUILD_TYPE=Release to build Release version
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif ()

# compiler flags
set(CMAKE_CXX_FLAGS "-Wall -Wextra -pedantic-errors -Wconversion -Wsign-conversion")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O2")

# ================================ thirdparty dependencies ========================== #
# ===== Google test ======
find_package(GTest REQUIRED)
# ===== LLVM =====
find_package(LLVM REQUIRED CONFIG)
include_directories(${LLVM_INCLUDE_DIRS})
# resolve symlink, because CLION with WSL doesn't seem to recognize this
get_filename_component(LLVM_INCLUDE_DIRS "${LLVM_INCLUDE_DIRS}/llvm/" REALPATH)
get_filename_component(LLVM_INCLUDE_DIRS ${LLVM_INCLUDE_DIRS} DIRECTORY)

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

# ===== gflags =====
find_package(gflags REQUIRED)
# ================================----------------------- ========================== #

# common source files shared by multiple targets
aux_source_directory(${PROJECT_SOURCE_DIR}/src/base COMMON_SRCS)
aux_source_directory(${PROJECT_SOURCE_DIR}/src/reader COMMON_SRCS)
aux_source_directory(${PROJECT_SOURCE_DIR}/src/lexer COMMON_SRCS)
aux_source_directory(${PROJECT_SOURCE_DIR}/src/parser COMMON_SRCS)
aux_source_directory(${PROJECT_SOURCE_DIR}/src/compiler COMMON_SRCS)
aux_source_directory(${PROJECT_SOURCE_DIR}/src/ast COMMON_SRCS)
aux_source_directory(${PROJECT_SOURCE_DIR}/src/tanc COMMON_SRCS)

# ====== tests for debug build ======
if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    # target test
    aux_source_directory(${PROJECT_SOURCE_DIR}/src/test TEST_SRCS)
    add_executable(tan_tests ${TEST_SRCS} ${COMMON_SRCS})
    target_include_directories(tan_tests PUBLIC ${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR}/src/test)
    target_link_libraries(tan_tests gtest LLVM-9)

    add_test(NAME cmake_reader_test COMMAND tan_tests WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/src/test)
    enable_testing()
endif (${CMAKE_BUILD_TYPE} STREQUAL "Debug")

# ===== target libtanc =====
aux_source_directory(${PROJECT_SOURCE_DIR}/src/lib LIB_SRCS)
add_library(tan ${COMMON_SRCS} ${LIB_SRCS})
target_include_directories(tan PUBLIC ${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(tan pthread LLVM-9)
set_target_properties(tan PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)

# ===== target CLI compiler =====
set(CLI_SRCS ${PROJECT_SOURCE_DIR}/src/cli/tanc.cpp)
add_executable(tanc ${CLI_SRCS})
target_include_directories(tanc PUBLIC ${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(tanc pthread gflags tan)
set_target_properties(tanc PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

# ===== target CLI JIT =====
set(JIT_SRCS ${PROJECT_SOURCE_DIR}/src/cli/tan-jit.cpp)
add_executable(tan-jit ${JIT_SRCS})
target_include_directories(tan-jit PUBLIC ${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(tan-jit pthread gflags tan)
set_target_properties(tan-jit PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

