set(DEP_DIR ${CMAKE_CURRENT_LIST_DIR})

# ===== llvm =====
if (${LLVM_ROOT_DIR})
    message(STATUS "Using custom LLVM at ${LLVM_ROOT_DIR}")
endif ()
find_package(LLVM 10 REQUIRED PATHS ${LLVM_ROOT_DIR})
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})
set(LLVM_LIBS ${LLVM_AVAILABLE_LIBS})

# CLION + WSL not handling symlink correctly
set(tmp ${LLVM_INCLUDE_DIRS}/llvm)
set(LLVM_INCLUDE_DIRS "")
foreach (d ${tmp})
    get_filename_component(d ${d} REALPATH)
    get_filename_component(d ${d} DIRECTORY)
    set(LLVM_INCLUDE_DIRS ${LLVM_INCLUDE_DIRS} ${d})
endforeach ()
message(STATUS "LLVM headers location: ${LLVM_INCLUDE_DIRS}")

# ===== clang =====
if (${CLANG_ROOT_DIR})
    message(STATUS "Using custom LLVM at ${CLANG_ROOT_DIR}")
endif ()
find_package(Clang REQUIRED PATHS ${CLANG_ROOT_DIR})
if (NOT DEFINED CLANG_INCLUDE_DIRS)
    message(FATAL_ERROR "Cannot find clang")
else ()
    message(STATUS "Clang headers found in: ${CLANG_INCLUDE_DIRS}")
endif ()
set(CLANG_LIBRARIES ${CLANG_EXPORTED_TARGETS})

# ================== other deps ==================
add_subdirectory(fmt)

# build libbacktrace
if (WIN32)
    set(LIBBACKTRACE_LIBS "" CACHE INTERNAL "libbacktrace libraries")
    set(LIBBACKTRACE_INCLUDE_DIRS "" CACHE INTERNAL "libbacktrace include dirs")
    add_custom_target(libbacktrace ALL)
else ()
    set(LIBBACKTRACE_LIBS ${CMAKE_CURRENT_LIST_DIR}/libbacktrace/.libs/libbacktrace.a CACHE INTERNAL "libbacktrace libraries")
    set(LIBBACKTRACE_INCLUDE_DIRS ${CMAKE_CURRENT_LIST_DIR}/libbacktrace CACHE INTERNAL "libbacktrace include dirs")
    add_custom_target(libbacktrace ALL DEPENDS ${LIBBACKTRACE_LIBS})
    add_custom_command(
            OUTPUT ${LIBBACKTRACE_LIBS}
            COMMAND ./configure
            COMMAND make
            WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/libbacktrace
    )
endif ()
# ================================================

set(DEP_INCLUDES
        ${LLVM_INCLUDE_DIRS}
        ${CLANG_INCLUDE_DIRS}
        ${DEP_DIR}/filesystem/include
        ${DEP_DIR}/fmt/include
        ${DEP_DIR}/libbacktrace
        CACHE INTERNAL "Include dirs of dependencies")

set(DEP_LIBS ${LLVM_LIBS}
        ${CLANG_LIBRARIES}
        fmt::fmt)
if (WIN32)
    set(DEP_LIBS ${DEP_LIBS}
            Version
            CACHE INTERNAL "Dependency libs")
else ()
    set(DEP_LIBS ${DEP_LIBS}
            z dl rt tinfo pthread m xml2
            gcov
            backtrace
            CACHE INTERNAL "Dependency libs")
endif ()

set(DEP_LIB_DIRS
        ${LLVM_LIBRARY_DIR}
        ${CLANG_INSTALL_PREFIX}/lib
        PARENT_SCOPE)
