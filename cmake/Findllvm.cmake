# BASED ON https://github.com/ziglang/zig/tree/master/cmake

# Copyright (c) 2014 Andrew Kelley
# This file is MIT licensed.
# See http://opensource.org/licenses/MIT

# LLVM_FOUND
# LLVM_INCLUDE_DIRS
# LLVM_LIBRARIES
# LLVM_LIBDIRS

# ============= LLVM =============
find_program(LLVM_CONFIG_EXE
        NAMES llvm-config-12 llvm-config-12.0 llvm-config120 llvm-config12 llvm-config
        PATHS
        ${LLVM_CUSTOM_CONFIG_EXE_DIR}
        "/mingw64/bin"
        "/c/msys64/mingw64/bin"
        "c:/msys64/mingw64/bin"
        "C:/Libraries/llvm-12.0.0/bin")

if ("${LLVM_CONFIG_EXE}" STREQUAL "LLVM_CONFIG_EXE-NOTFOUND")
    message(FATAL_ERROR "unable to find llvm-config")
endif ()

if ("${LLVM_CONFIG_EXE}" STREQUAL "LLVM_CONFIG_EXE-NOTFOUND")
    message(FATAL_ERROR "unable to find llvm-config")
endif ()

# check version
execute_process(
        COMMAND ${LLVM_CONFIG_EXE} --version
        OUTPUT_VARIABLE LLVM_CONFIG_VERSION
        OUTPUT_STRIP_TRAILING_WHITESPACE)

if ("${LLVM_CONFIG_VERSION}" VERSION_LESS 12)
    message(FATAL_ERROR "expected LLVM 12.x but found ${LLVM_CONFIG_VERSION} using ${LLVM_CONFIG_EXE}")
endif ()
if ("${LLVM_CONFIG_VERSION}" VERSION_EQUAL 13)
    message(FATAL_ERROR "expected LLVM 12.x but found ${LLVM_CONFIG_VERSION} using ${LLVM_CONFIG_EXE}")
endif ()
if ("${LLVM_CONFIG_VERSION}" VERSION_GREATER 13)
    message(FATAL_ERROR "expected LLVM 12.x but found ${LLVM_CONFIG_VERSION} using ${LLVM_CONFIG_EXE}")
endif ()

# get include dirs
execute_process(
        COMMAND ${LLVM_CONFIG_EXE} --includedir
        OUTPUT_VARIABLE LLVM_INCLUDE_DIRS_SPACES
        OUTPUT_STRIP_TRAILING_WHITESPACE)
string(REPLACE " " ";" LLVM_INCLUDE_DIRS "${LLVM_INCLUDE_DIRS_SPACES}")

# get list of llvm targets
execute_process(
        COMMAND ${LLVM_CONFIG_EXE} --targets-built
        OUTPUT_VARIABLE LLVM_TARGETS_BUILT_SPACES
        OUTPUT_STRIP_TRAILING_WHITESPACE)
string(REPLACE " " ";" LLVM_TARGETS_BUILT "${LLVM_TARGETS_BUILT_SPACES}")
function(NEED_TARGET TARGET_NAME)
    list(FIND LLVM_TARGETS_BUILT "${TARGET_NAME}" _index)
    if (${_index} EQUAL -1)
        message(FATAL_ERROR "LLVM (according to ${LLVM_CONFIG_EXE}) is missing target ${TARGET_NAME}. Zig requires LLVM to be built with all default targets enabled.")
    endif ()
endfunction(NEED_TARGET)
NEED_TARGET("AArch64")
NEED_TARGET("AMDGPU")
NEED_TARGET("ARM")
NEED_TARGET("AVR")
NEED_TARGET("BPF")
NEED_TARGET("Hexagon")
NEED_TARGET("Lanai")
NEED_TARGET("Mips")
NEED_TARGET("MSP430")
NEED_TARGET("NVPTX")
NEED_TARGET("PowerPC")
NEED_TARGET("RISCV")
NEED_TARGET("Sparc")
NEED_TARGET("SystemZ")
NEED_TARGET("WebAssembly")
NEED_TARGET("X86")
NEED_TARGET("XCore")

# get libraries and lib dirs
if (ZIG_STATIC_LLVM)
    execute_process(
            COMMAND ${LLVM_CONFIG_EXE} --libfiles --link-static
            OUTPUT_VARIABLE LLVM_LIBRARIES_SPACES
            OUTPUT_STRIP_TRAILING_WHITESPACE)
    string(REPLACE " " ";" LLVM_LIBRARIES "${LLVM_LIBRARIES_SPACES}")

    execute_process(
            COMMAND ${LLVM_CONFIG_EXE} --system-libs --link-static
            OUTPUT_VARIABLE LLVM_SYSTEM_LIBS_SPACES
            OUTPUT_STRIP_TRAILING_WHITESPACE)
    string(REPLACE " " ";" LLVM_SYSTEM_LIBS "${LLVM_SYSTEM_LIBS_SPACES}")

    execute_process(
            COMMAND ${LLVM_CONFIG_EXE} --libdir --link-static
            OUTPUT_VARIABLE LLVM_LIBDIRS_SPACES
            OUTPUT_STRIP_TRAILING_WHITESPACE)
    string(REPLACE " " ";" LLVM_LIBDIRS "${LLVM_LIBDIRS_SPACES}")
endif ()
if (NOT LLVM_LIBRARIES)
    execute_process(
            COMMAND ${LLVM_CONFIG_EXE} --libs
            OUTPUT_VARIABLE LLVM_LIBRARIES_SPACES
            OUTPUT_STRIP_TRAILING_WHITESPACE)
    string(REPLACE " " ";" LLVM_LIBRARIES "${LLVM_LIBRARIES_SPACES}")

    execute_process(
            COMMAND ${LLVM_CONFIG_EXE} --system-libs
            OUTPUT_VARIABLE LLVM_SYSTEM_LIBS_SPACES
            OUTPUT_STRIP_TRAILING_WHITESPACE)
    string(REPLACE " " ";" LLVM_SYSTEM_LIBS "${LLVM_SYSTEM_LIBS_SPACES}")

    execute_process(
            COMMAND ${LLVM_CONFIG_EXE} --libdir
            OUTPUT_VARIABLE LLVM_LIBDIRS_SPACES
            OUTPUT_STRIP_TRAILING_WHITESPACE)
    string(REPLACE " " ";" LLVM_LIBDIRS "${LLVM_LIBDIRS_SPACES}")
endif ()

set(LLVM_LIBRARIES ${LLVM_LIBRARIES} ${LLVM_SYSTEM_LIBS})

if (NOT LLVM_LIBRARIES)
    find_library(LLVM_LIBRARIES NAMES LLVM LLVM-12 LLVM-12.0)
endif ()

link_directories("${CMAKE_PREFIX_PATH}/lib")
link_directories("${LLVM_LIBDIRS}")

# ============= CLANG =============
macro(FIND_AND_ADD_CLANG_LIB _libname_)
    string(TOUPPER ${_libname_} _prettylibname_)
    find_library(CLANG_${_prettylibname_}_LIB NAMES ${_libname_}
            PATHS
            ${LLVM_LIBDIRS}
            /usr/lib/llvm/12/lib
            /usr/lib/llvm-12/lib
            /usr/lib/llvm-12.0/lib
            /usr/local/llvm120/lib
            /usr/local/llvm12/lib
            /mingw64/lib
            /c/msys64/mingw64/lib
            c:\\msys64\\mingw64\\lib
            )
    if (CLANG_${_prettylibname_}_LIB)
        set(CLANG_LIBRARIES ${CLANG_LIBRARIES} ${CLANG_${_prettylibname_}_LIB})
    else ()
        message(WARNING "Cannot find Clang library ${_libname_}")
    endif ()
endmacro(FIND_AND_ADD_CLANG_LIB)

FIND_AND_ADD_CLANG_LIB(clangFrontendTool)
FIND_AND_ADD_CLANG_LIB(clangCodeGen)
FIND_AND_ADD_CLANG_LIB(clangFrontend)
FIND_AND_ADD_CLANG_LIB(clangDriver)
FIND_AND_ADD_CLANG_LIB(clangSerialization)
FIND_AND_ADD_CLANG_LIB(clangSema)
FIND_AND_ADD_CLANG_LIB(clangStaticAnalyzerFrontend)
FIND_AND_ADD_CLANG_LIB(clangStaticAnalyzerCheckers)
FIND_AND_ADD_CLANG_LIB(clangStaticAnalyzerCore)
FIND_AND_ADD_CLANG_LIB(clangAnalysis)
FIND_AND_ADD_CLANG_LIB(clangASTMatchers)
FIND_AND_ADD_CLANG_LIB(clangAST)
FIND_AND_ADD_CLANG_LIB(clangParse)
FIND_AND_ADD_CLANG_LIB(clangSema)
FIND_AND_ADD_CLANG_LIB(clangBasic)
FIND_AND_ADD_CLANG_LIB(clangEdit)
FIND_AND_ADD_CLANG_LIB(clangLex)
FIND_AND_ADD_CLANG_LIB(clangARCMigrate)
FIND_AND_ADD_CLANG_LIB(clangRewriteFrontend)
FIND_AND_ADD_CLANG_LIB(clangRewrite)
FIND_AND_ADD_CLANG_LIB(clangCrossTU)
FIND_AND_ADD_CLANG_LIB(clangIndex)
FIND_AND_ADD_CLANG_LIB(clangToolingCore)

# merge clang libs and llvm libs

set(LLVM_LIBRARIES ${LLVM_LIBRARIES} ${CLANG_LIBRARIES})

# ============= LLD =============
macro(FIND_AND_ADD_LLD_LIB _libname_)
    string(TOUPPER ${_libname_} _prettylibname_)
    find_library(LLD_${_prettylibname_}_LIB NAMES ${_libname_}
            PATHS
            ${LLVM_LIBDIRS}
            /usr/lib/llvm-12/lib
            /usr/local/llvm120/lib
            /usr/local/llvm12/lib
            /mingw64/lib
            /c/msys64/mingw64/lib
            c:/msys64/mingw64/lib)
    if (LLD_${_prettylibname_}_LIB)
        set(LLD_LIBRARIES ${LLD_LIBRARIES} ${LLD_${_prettylibname_}_LIB})
    else ()
        message(WARNING "Cannot find LLD library ${_libname_}")
    endif ()
endmacro(FIND_AND_ADD_LLD_LIB)

FIND_AND_ADD_LLD_LIB(lldDriver)
FIND_AND_ADD_LLD_LIB(lldMinGW)
FIND_AND_ADD_LLD_LIB(lldELF)
FIND_AND_ADD_LLD_LIB(lldCOFF)
FIND_AND_ADD_LLD_LIB(lldMachO)
FIND_AND_ADD_LLD_LIB(lldWasm)
FIND_AND_ADD_LLD_LIB(lldReaderWriter)
FIND_AND_ADD_LLD_LIB(lldCore)
FIND_AND_ADD_LLD_LIB(lldYAML)
FIND_AND_ADD_LLD_LIB(lldCommon)

# merge lld libs and llvm libs

set(LLVM_LIBRARIES ${LLVM_LIBRARIES} ${LLD_LIBRARIES})

# ===========================
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(llvm DEFAULT_MSG LLVM_LIBRARIES LLVM_INCLUDE_DIRS)

mark_as_advanced(LLVM_INCLUDE_DIRS LLVM_LIBRARIES LLVM_LIBDIRS)
