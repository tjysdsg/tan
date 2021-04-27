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

# Define macros used in C headers
# see include/config.h.in
set(CMAKE_PROJECT_TAN_MAX_N_FUNCTION_CALLS 256)

# Override compiling flags
set(CMAKE_USER_MAKE_RULES_OVERRIDE ${CMAKE_CURRENT_SOURCE_DIR}/cmake/c_flag_overrides.cmake)
set(CMAKE_USER_MAKE_RULES_OVERRIDE_CXX ${CMAKE_CURRENT_SOURCE_DIR}/cmake/cxx_flag_overrides.cmake)

# project
project(tan LANGUAGES CXX C VERSION ${TAN_VERSION})

# ==================== OPTIONS ====================
option(ENABLE_PRECOMPILED_HEADERS "Enable precompiled headers to speed up build" ON)
option(BUILD_EXAMPLES "Build examples" OFF)
option(ENABLE_COVERAGE "Enable coverage" OFF)
option(BUILD_DOCS "Build doxygen" OFF)
option(ENABLE_CCACHE "Enable ccache for faster rebuilding" ON)

# ==================== GENERAL CONFIGS ====================
# include cmake/*.cmake
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

set(LIB_OUTPUT_DIR ${PROJECT_SOURCE_DIR}/lib CACHE STRING "Library output directory")
set(EXE_OUTPUT_DIR ${PROJECT_SOURCE_DIR}/bin CACHE STRING "Executable output directory")

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
include_directories(${DEP_INCLUDES})
link_directories(${DEP_LIB_DIRS})
set(DEP_LIBS ${LLVM_LIBRARIES} ${DEP_LIBS})

message(STATUS "Dependency library dirs are ${DEP_LIB_DIRS}")
message(STATUS "Dependency libraries are ${DEP_LIBS}")
# =================================================================================== #

# generate include/config.h using include/config.h.in
configure_file(${PROJECT_SOURCE_DIR}/include/config.h.in ${PROJECT_SOURCE_DIR}/include/config.h)

macro(set_common_options target_name)
    message(DEBUG "Generating target: ${target_name}")
    target_compile_features(${target_name} PRIVATE cxx_std_17)
    if (NOT MSVC)
        target_compile_options(${target_name}
                PRIVATE -Wall
                PRIVATE -Wextra
                PRIVATE -pedantic-errors
                PRIVATE -Wconversion
                )
    endif ()
    target_include_directories(${target_name} PRIVATE
            ${PROJECT_SOURCE_DIR}
            ${PROJECT_SOURCE_DIR}/include
            ${DEP_INCLUDES}
            )

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
    set_target_properties(${target_name} PROPERTIES COMPILE_FLAGS ${EXE_CFLAGS})
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

# ================ FLAGS ================
if (MSVC)
    set(EXE_CFLAGS "${EXE_CFLAGS}")
else ()
    set(EXE_CFLAGS "${EXE_CFLAGS} -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS -D_GNU_SOURCE -fvisibility-inlines-hidden -fno-exceptions -fno-rtti -Werror=type-limits -Wno-missing-braces")
    if (MINGW)
        set(EXE_CFLAGS "${EXE_CFLAGS} -Wno-format")
    endif ()
endif ()

set(EXE_LDFLAGS " ")
if (MSVC)
    set(EXE_LDFLAGS "${EXE_LDFLAGS} /STACK:16777216")
    if (NOT "${CMAKE_BUILD_TYPE}" STREQUAL "Release" AND NOT "${CMAKE_BUILD_TYPE}" STREQUAL "MinSizeRel")
        set(EXE_LDFLAGS "${EXE_LDFLAGS} /debug:fastlink")
    endif ()
elseif (MINGW)
    set(EXE_LDFLAGS "${EXE_LDFLAGS} -Wl,--stack,16777216")
endif ()

# ===== target library tan =====
aux_source_directory(${PROJECT_SOURCE_DIR}/src/lib LIB_SRCS)
add_library(tan ${COMMON_SRCS} ${LIB_SRCS})
if (MSVC)
    target_link_libraries(tan ${DEP_LIBS})
else ()
    target_link_libraries(tan ${DEP_LIBS} unwind)
endif ()
set_common_options(tan)
if (${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.16" AND ${ENABLE_PRECOMPILED_HEADERS})
    message(STATUS "Precompiled headers are enabled")
    target_precompile_headers(tan PRIVATE src/llvm_include.h include/base.h include/config.h)
endif ()

# ===== target CLI compiler =====
aux_source_directory(${PROJECT_SOURCE_DIR}/src/cli CLI_SRCS)
add_executable(tanc ${CLI_SRCS})
target_link_libraries(tanc tan)
set_common_options(tanc)

# ===== target runtime =====
add_subdirectory(${PROJECT_SOURCE_DIR}/runtime)

# add all srcs before enabling tests (for gcov)
set(ALL_SRCS ${COMMON_SRCS} ${LIB_SRCS} ${CLI_SRCS})

# ===== tests and coverage =====
if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    add_subdirectory(${PROJECT_SOURCE_DIR}/src/test)
    enable_tests()
endif ()

# ===== docs =====
if (${BUILD_DOCS})
    add_subdirectory(${PROJECT_SOURCE_DIR}/docs)
endif ()

# ===== examples =====
if (${BUILD_EXAMPLES})
    add_subdirectory(${PROJECT_SOURCE_DIR}/examples)
endif ()
