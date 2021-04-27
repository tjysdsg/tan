# DEP_INCLUDES
# DEP_LIB_DIRS
# DEP_LIBS

set(DEP_DIR ${CMAKE_CURRENT_LIST_DIR})

find_package(llvm)
find_package(clang)
find_package(lld)

set(DEP_INCLUDES
        ${LLVM_INCLUDE_DIRS}
        ${CLANG_INCLUDE_DIRS}
        ${LLD_INCLUDE_DIRS}
        ${DEP_DIR}/filesystem/include
        ${DEP_DIR}/libbacktrace
        PARENT_SCOPE)

# FIXME: libunwind
find_library(LIBUNWIND_PATH NAMES unwind
        PATHS
        /usr/local/lib
        NO_DEFAULT_PATH
        )
set(DEP_LIBS
        ${LLVM_LIBRARIES}
        ${CLANG_LIBRARIES}
        ${LLD_LIBRARIES}
        ${LIBUNWIND_PATH} lzma
        z dl rt tinfo pthread m xml2
        gcov
        backtrace
        PARENT_SCOPE
        )

set(DEP_LIB_DIRS
        ${LLVM_LIBDIRS}
        ${CLANG_LIBDIRS}
        ${LLD_LIBDIRS}
        ${DEP_DIR}/libbacktrace/.libs
        PARENT_SCOPE)
