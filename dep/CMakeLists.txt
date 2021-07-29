set(DEP_DIR ${CMAKE_CURRENT_LIST_DIR})

# ===== llvm =====
find_package(LLVM 10 REQUIRED PATHS ${LLVM_ROOT_DIR})
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})
set(LLVM_LIBS ${LLVM_AVAILABLE_LIBS})
message(STATUS "LLVM headers location: ${LLVM_INCLUDE_DIRS}")

# ===== clang =====
find_package(Clang REQUIRED PATHS ${CLANG_ROOT_DIR})
message(STATUS "Clang headers location: ${CLANG_INCLUDE_DIRS}")
set(CLANG_LIBRARIES
        clang
        clangBasic
        clangCodeGen
        clangDriver
        clangTooling
        clangFrontend
        clangFrontendTool
        clangSerialization
        clangFrontendTool
        clangCodeGen
        clangFrontend
        clangDriver
        clangSerialization
        clangSema
        clangStaticAnalyzerFrontend
        clangStaticAnalyzerCheckers
        clangStaticAnalyzerCore
        clangAnalysis
        clangASTMatchers
        clangAST
        clangParse
        clangSema
        clangBasic
        clangEdit
        clangLex
        clangARCMigrate
        clangRewriteFrontend
        clangRewrite
        clangCrossTU
        clangIndex
        # Polly
        # PollyISL
        # PollyPPCG
        )

# ================== other deps ==================
add_subdirectory(fmt)

# build libbacktrace
set(LIBBACKTRACE_LIBS ${CMAKE_CURRENT_LIST_DIR}/libbacktrace/.libs/libbacktrace.a CACHE INTERNAL "libbacktrace libraries")
set(LIBBACKTRACE_INCLUDE_DIRS ${CMAKE_CURRENT_LIST_DIR}/libbacktrace CACHE INTERNAL "libbacktrace include dirs")
add_custom_target(libbacktrace ALL DEPENDS ${LIBBACKTRACE_LIBS})
add_custom_command(
        OUTPUT ${LIBBACKTRACE_LIBS}
        COMMAND ./configure
        COMMAND make
        WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/libbacktrace
)
# ================================================

set(DEP_INCLUDES
        ${LLVM_INCLUDE_DIRS}
        ${CLANG_INCLUDE_DIRS}
        ${DEP_DIR}/filesystem/include
        ${DEP_DIR}/libbacktrace
        PARENT_SCOPE)

set(DEP_LIBS ${LLVM_LIBS}
        ${CLANG_LIBRARIES}
        fmt::fmt
        z dl rt tinfo pthread m xml2
        gcov
        backtrace
        PARENT_SCOPE)

set(DEP_LIB_DIRS
        ${LLVM_LIBRARY_DIR}
        ${CLANG_INSTALL_PREFIX}/lib
        PARENT_SCOPE)
