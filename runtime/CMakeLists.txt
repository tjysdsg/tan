aux_source_directory(${CMAKE_CURRENT_LIST_DIR} RUNTIME_CXX_SRCS)
file(GLOB_RECURSE RUNTIME_SRCS ${CMAKE_CURRENT_LIST_DIR}/*.tan)
add_custom_target(runtime ALL DEPENDS tanc SOURCES ${RUNTIME_SRCS} ${RUNTIME_CXX_SRCS})
add_custom_command(TARGET runtime
        COMMAND ${PROJECT_SOURCE_DIR}/bin/tanc -I${CMAKE_CURRENT_LIST_DIR} --obj --print-ir
        ${RUNTIME_SRCS} ${RUNTIME_CXX_SRCS}
        COMMAND ${CMAKE_CXX_COMPILER} *.o -lunwind -shared -o libruntime.so
        WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
        )
