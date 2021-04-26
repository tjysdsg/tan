# DEP_INCLUDES
# DEP_LIB_DIRS
# DEP_LIBS

set(DEP_DIR ${CMAKE_CURRENT_LIST_DIR})

set(DEP_INCLUDES
        ${DEP_DIR}/filesystem/include
        ${DEP_DIR}/libbacktrace
        PARENT_SCOPE
        )

if (MSVC)
    # TODO: libunwind for windows
    set(DEP_LIBS " " PARENT_SCOPE)
else ()
    # FIXME: libunwind
    find_library(LIBUNWIND_PATH NAMES unwind
            PATHS
            /usr/local/lib
            NO_DEFAULT_PATH
            )

    set(DEP_LIBS
            ${LIBUNWIND_PATH} lzma
            z dl rt tinfo pthread m xml2
            gcov
            backtrace
            PARENT_SCOPE
            )
endif ()

set(DEP_LIB_DIRS ${DEP_DIR}/libbacktrace/.libs PARENT_SCOPE)
