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
##
project(tan LANGUAGES CXX VERSION ${TAN_VERSION})
# generate include/config.h
configure_file(${PROJECT_SOURCE_DIR}/include/config.h.in ${PROJECT_SOURCE_DIR}/include/config.h)

# C++11
set(CMAKE_CXX_STANDARD 11)

# default build debug
# use -DCMAKE_BUILD_TYPE=Release to build Release version
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif ()

# compiler flags
set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

# ================================ thirdparty dependencies ========================== #
# this project uses Google Test framework
find_package(GTest REQUIRED)
# C++ boost library
find_package(Boost 1.50 COMPONENTS filesystem program_options REQUIRED)
include_directories(${Boost_INCLUDE_DIR})
# LLVM
find_package(LLVM REQUIRED CONFIG)
include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})
# Find the libraries that correspond to the LLVM components
# that we wish to use
llvm_map_components_to_libnames(llvm_libs support core irreader)
# ================================----------------------- ========================== #

# common source files shared by multiple targets
aux_source_directory(${PROJECT_SOURCE_DIR}/src/base COMMON_SRCS)
aux_source_directory(${PROJECT_SOURCE_DIR}/src/reader COMMON_SRCS)
aux_source_directory(${PROJECT_SOURCE_DIR}/src/lexer COMMON_SRCS)
aux_source_directory(${PROJECT_SOURCE_DIR}/src/parser COMMON_SRCS)

# target test
aux_source_directory(${PROJECT_SOURCE_DIR}/src/test TEST_SRCS)
add_executable(reader_test ${TEST_SRCS} ${COMMON_SRCS})
target_include_directories(reader_test PUBLIC ${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR}/src/test)
target_link_libraries(reader_test gtest ${Boost_LIBRARIES} ${llvm_libs})

add_test(NAME cmake_reader_test COMMAND reader_test WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/src/test)

# target tanc (cli)
aux_source_directory(${PROJECT_SOURCE_DIR}/src/cli CLI_SRCS)
add_executable(tanc ${COMMON_SRCS} ${CLI_SRCS})
target_include_directories(tanc PUBLIC ${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(tanc pthread ${Boost_LIBRARIES} ${llvm_libs})
set_target_properties(tanc
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin"
)

enable_testing()
