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
    set(DEP_LIBS " " PARENT_SCOPE)
else ()
    set(DEP_LIBS
            z dl rt tinfo pthread m xml2
            gcov
            backtrace
            PARENT_SCOPE
            )
endif ()

set(DEP_LIB_DIRS ${DEP_DIR}/libbacktrace/.libs PARENT_SCOPE)
