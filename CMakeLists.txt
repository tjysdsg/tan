cmake_minimum_required(VERSION 3.0)

# version number
set(TAN_VERSION 0.0.1)
string(REPLACE "." ";" VERSION_LIST ${TAN_VERSION})
list(GET VERSION_LIST 0 PROJECT_VERSION_MAJOR)
list(GET VERSION_LIST 1 PROJECT_VERSION_MINOR)
list(GET VERSION_LIST 2 PROJECT_VERSION_PATCH)
set(CMAKE_PROJECT_TAN_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CMAKE_PROJECT_TAN_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CMAKE_PROJECT_TAN_VERSION_PATCH "${PROJECT_VERSION_PATCH}")
# see include/config.h.in
set(CMAKE_PROJECT_TAN_MAX_N_FUNCTION_CALLS 256)

project(tan LANGUAGES CXX C VERSION ${TAN_VERSION})
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

# ==================== OPTIONS ====================
option(ENABLE_PRECOMPILED_HEADERS "Enable precompiled headers to speed up build" ON)
option(ENABLE_COVERAGE "Enable coverage" OFF)
option(BUILD_EXAMPLES "Build examples" OFF)
option(ENABLE_CCACHE "Enable ccache for faster rebuilding" ON)

# ==================== GENERAL CONFIGS ====================
set(LIB_OUTPUT_DIR ${PROJECT_SOURCE_DIR}/lib CACHE STRING "Library output directory")
set(EXE_OUTPUT_DIR ${PROJECT_SOURCE_DIR}/bin CACHE STRING "Executable output directory")

# use static linking whenever possible
set(CMAKE_FIND_LIBRARY_SUFFIXES ".a" ${CMAKE_FIND_LIBRARY_SUFFIXES})

# always use full RPATH
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# default build debug
if (NOT CMAKE_BUILD_TYPE)
    message(STATUS "CMAKE_BUILD_TYPE not specified, using Debug as default")
    set(CMAKE_BUILD_TYPE Debug)
endif ()

# Use ccache to compile if possible
find_program(CCACHE_FOUND ccache)
if (CCACHE_FOUND)
    if (${ENABLE_CCACHE})
        message(STATUS "Enabling ccache for faster rebuilding...")
        set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
    endif ()
endif (CCACHE_FOUND)

# ================================ thirdparty dependencies ========================== #
# googletest
add_subdirectory(${PROJECT_SOURCE_DIR}/dep/googletest)
# others in dep/
add_subdirectory(${PROJECT_SOURCE_DIR}/dep)

message(STATUS "DEP_INCLUDES ${DEP_INCLUDES}")
message(STATUS "DEP_LIBS: ${DEP_LIBS}")
message(STATUS "DEP_LIB_DIRS: ${DEP_LIB_DIRS}")

include_directories(${DEP_INCLUDES})
link_directories(${DEP_LIB_DIRS})
# =================================================================================== #

# generate include/config.h using include/config.h.in
configure_file(${PROJECT_SOURCE_DIR}/include/config.h.in ${PROJECT_SOURCE_DIR}/include/config.h)

macro(set_other_options target_name)
    message(DEBUG "Generating target: ${target_name}")
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
    set_target_properties(${target_name} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${LIB_OUTPUT_DIR})
    set_target_properties(${target_name} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${LIB_OUTPUT_DIR})
    set_target_properties(${target_name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${EXE_OUTPUT_DIR})
endmacro()

# common source files shared by multiple targets
aux_source_directory(${PROJECT_SOURCE_DIR}/src/base COMMON_SRCS)
aux_source_directory(${PROJECT_SOURCE_DIR}/src/lexer COMMON_SRCS)
aux_source_directory(${PROJECT_SOURCE_DIR}/src/parser COMMON_SRCS)
aux_source_directory(${PROJECT_SOURCE_DIR}/src/analysis COMMON_SRCS)
aux_source_directory(${PROJECT_SOURCE_DIR}/src/compiler COMMON_SRCS)
aux_source_directory(${PROJECT_SOURCE_DIR}/src/ast COMMON_SRCS)
aux_source_directory(${PROJECT_SOURCE_DIR}/src/linker COMMON_SRCS)
aux_source_directory(${PROJECT_SOURCE_DIR}/src/codegen COMMON_SRCS)
aux_source_directory(${PROJECT_SOURCE_DIR}/src COMMON_SRCS)

# ===== target library tan_backtrace =====
aux_source_directory(${PROJECT_SOURCE_DIR}/src/backtrace BACKTRACE_SRCS)
add_library(tan_backtrace SHARED ${BACKTRACE_SRCS})
add_dependencies(tan_backtrace libbacktrace) # libbacktrace custom target defined in dep/CMakeLists.txt
target_include_directories(tan_backtrace PRIVATE ${LIBBACKTRACE_INCLUDE_DIRS})
target_link_libraries(tan_backtrace ${LIBBACKTRACE_LIBS})
set_other_options(tan_backtrace)

# ===== target library tan =====
aux_source_directory(${PROJECT_SOURCE_DIR}/src/lib LIB_SRCS)
add_library(tan ${COMMON_SRCS} ${LIB_SRCS})
target_link_libraries(tan pthread ${DEP_LIBS} tan_backtrace)
set_other_options(tan)
if (${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.16" AND ${ENABLE_PRECOMPILED_HEADERS})
    message(STATUS "precompiled headers are enabled")
    target_precompile_headers(tan PRIVATE src/llvm_include.h include/base.h include/config.h include/clang-frontend.h)
endif ()

# ===== target CLI compiler =====
aux_source_directory(${PROJECT_SOURCE_DIR}/src/cli CLI_SRCS)
add_executable(tanc ${CLI_SRCS})
target_link_libraries(tanc tan)
set_other_options(tanc)

# ===== target runtime =====
add_subdirectory(${PROJECT_SOURCE_DIR}/runtime)

# add all srcs before enabling tests (for gcov)
set(ALL_SRCS ${COMMON_SRCS} ${LIB_SRCS} ${CLI_SRCS})

# ===== tests and coverage =====
if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    add_subdirectory(test)
    enable_tests()
endif ()

# ===== docs =====
add_subdirectory(${PROJECT_SOURCE_DIR}/docs)

# ===== examples =====
if (${BUILD_EXAMPLES})
    add_subdirectory(${PROJECT_SOURCE_DIR}/examples)
endif ()
