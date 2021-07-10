aux_source_directory(${CMAKE_CURRENT_LIST_DIR} RUNTIME_CXX_SRCS)
file(GLOB_RECURSE RUNTIME_SRCS ${CMAKE_CURRENT_LIST_DIR}/*.tan)
add_custom_target(
        runtime ALL
        DEPENDS ${CMAKE_CURRENT_LIST_DIR}/libruntime.so tanc
)
add_custom_command(
        OUTPUT ${CMAKE_CURRENT_LIST_DIR}/libruntime.so
        DEPENDS ${RUNTIME_SRCS} ${RUNTIME_CXX_SRCS} ${CMAKE_CURRENT_LIST_DIR}/stack_trace.h tanc
        COMMAND ${PROJECT_SOURCE_DIR}/bin/tanc -I${CMAKE_CURRENT_LIST_DIR} --shared -lunwind -o libruntime.so
        ${RUNTIME_SRCS} ${RUNTIME_CXX_SRCS}
        WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
)
