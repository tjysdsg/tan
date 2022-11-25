aux_source_directory(${CMAKE_CURRENT_LIST_DIR} RUNTIME_CXX_SRCS)
file(GLOB_RECURSE RUNTIME_SRCS ${CMAKE_CURRENT_LIST_DIR}/*.tan)

if (WIN32)
    set(LIB_EXT "dll")
else ()
    set(LIB_EXT "so")
endif ()

add_custom_target(
        runtime ALL
        DEPENDS ${CMAKE_CURRENT_LIST_DIR}/libruntime.${LIB_EXT} tanc TanBackTrace
)
add_custom_command(
        OUTPUT ${CMAKE_CURRENT_LIST_DIR}/libruntime.${LIB_EXT}
        DEPENDS ${RUNTIME_SRCS} ${RUNTIME_CXX_SRCS} ${CMAKE_CURRENT_LIST_DIR}/stack_trace.h tanc
        COMMAND ${PROJECT_SOURCE_DIR}/bin/tanc -shared -I${CMAKE_CURRENT_LIST_DIR} -L${LIB_OUTPUT_DIR}
        -lTanBackTrace
        -o libruntime.${LIB_EXT}
        ${RUNTIME_SRCS} ${RUNTIME_CXX_SRCS}
        WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
)
