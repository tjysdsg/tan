cmake_minimum_required(VERSION 3.19)

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
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

# ==================== OPTIONS ====================
option(ENABLE_COVERAGE "Enable coverage" OFF)
option(BUILD_EXAMPLES "Build examples" OFF)
option(ENABLE_CCACHE "Enable ccache for faster rebuilding" ON)
option(LLVM_ROOT_DIR "Custom LLVM location" "")
option(CLANG_ROOT_DIR "Custom CLANG location" "")
option(LLD_ROOT_DIR "Custom LLD location" "")
option(USE_LLD "Use lld to link" OFF)

# ==================== GENERAL CONFIGS ====================
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

# use lld to link libraries
if (NOT MSVC AND USE_LLD)
    message(STATUS "Using LLD to link binaries. Make sure ld.lld is in PATH.")
    add_link_options("-fuse-ld=lld")
endif ()

# ================================ thirdparty dependencies ========================== #
# others in dep/
add_subdirectory(${PROJECT_SOURCE_DIR}/dep)

message(STATUS "DEP_INCLUDES ${DEP_INCLUDES}")
message(STATUS "DEP_LIBS: ${DEP_LIBS}")
message(STATUS "DEP_LIB_DIRS: ${DEP_LIB_DIRS}")
# =================================================================================== #

# generate include/config.h using include/config.h.in
configure_file(${PROJECT_SOURCE_DIR}/include/config.h.in ${PROJECT_SOURCE_DIR}/include/config.h)

macro(set_target_common_compile_options target_name)
    message(STATUS ${target_name})
    get_property(prev_targets GLOBAL PROPERTY ALL_TAN_TARGETS)
    set_property(GLOBAL PROPERTY ALL_TAN_TARGETS ${prev_targets} ${target_name})

    if (MSVC)
        target_compile_options(${target_name} PRIVATE /w)
    else ()
        target_compile_options(${target_name}
                PRIVATE -Wall
                PRIVATE -Wextra
                PRIVATE -pedantic-errors
                PRIVATE -Wconversion)
    endif ()
    target_include_directories(${target_name} PRIVATE ${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/include)

    target_include_directories(${target_name} SYSTEM PRIVATE ${DEP_INCLUDES})
    target_link_directories(${target_name} PRIVATE ${DEP_LIB_DIRS})

    if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
        # add `DEBUG` and `_DEBUG` macro
        target_compile_definitions(${target_name} PRIVATE -DDEBUG)
        target_compile_definitions(${target_name} PRIVATE -D_DEBUG)

        # =========== COVERAGE =========== #
        if (${ENABLE_COVERAGE})
            message(STATUS "Enabling test coverage")
            target_compile_options(${target_name}
                    PUBLIC -O0
                    PUBLIC -fprofile-instr-generate
                    PUBLIC -fcoverage-mapping)
            target_link_options(${target_name}
                    PUBLIC -fprofile-instr-generate
                    PUBLIC -fcoverage-mapping)
        endif ()
    else ()
        target_compile_options(${target_name} PRIVATE -O3)
    endif ()
endmacro()

# Library and CLI targets
add_subdirectory(src)

# ===== target runtime =====
add_subdirectory(${PROJECT_SOURCE_DIR}/runtime)

# ===== tests =====
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


get_property(all_targets GLOBAL PROPERTY ALL_TAN_TARGETS)
message(STATUS "All targets: ${all_targets}")