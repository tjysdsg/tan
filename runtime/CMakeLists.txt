aux_source_directory(${CMAKE_CURRENT_LIST_DIR} RUNTIME_CXX_SRCS)
file(GLOB_RECURSE RUNTIME_SRCS ${CMAKE_CURRENT_LIST_DIR}/*.tan)
add_custom_target(
        runtime ALL
        DEPENDS ${CMAKE_CURRENT_LIST_DIR}/libruntime.a tanc
)
add_custom_command(
        OUTPUT ${CMAKE_CURRENT_LIST_DIR}/libruntime.a
        DEPENDS ${RUNTIME_SRCS} ${RUNTIME_CXX_SRCS} ${CMAKE_CURRENT_LIST_DIR}/stack_trace.h tanc
        COMMAND ${PROJECT_SOURCE_DIR}/bin/tanc -static -I${CMAKE_CURRENT_LIST_DIR} -ltan_backtrace
        -L${LIB_OUTPUT_DIR} -o libruntime.a
        ${RUNTIME_SRCS} ${RUNTIME_CXX_SRCS}
        WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
)
