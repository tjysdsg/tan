aux_source_directory(${CMAKE_CURRENT_LIST_DIR} RUNTIME_CXX_SRCS)
file(GLOB_RECURSE RUNTIME_SRCS ${CMAKE_CURRENT_LIST_DIR}/*.tan)

if (WIN32)
    set(LIB_EXT "dll")
else ()
    set(LIB_EXT "so")
endif ()

# This tells the executable where to write test coverage output
set(RUN_TEST_COMMAND_PREFIX ${CMAKE_COMMAND} -E env LLVM_PROFILE_FILE=${PROJECT_SOURCE_DIR}/%p.profraw)

add_custom_target(
        runtime ALL
        DEPENDS ${CMAKE_CURRENT_LIST_DIR}/libruntime.${LIB_EXT} tanc TanBackTrace
)
add_custom_command(
        OUTPUT ${CMAKE_CURRENT_LIST_DIR}/libruntime.${LIB_EXT}
        DEPENDS ${RUNTIME_SRCS} ${RUNTIME_CXX_SRCS} ${CMAKE_CURRENT_LIST_DIR}/stack_trace.h tanc
        COMMAND ${RUN_TEST_COMMAND_PREFIX}
        $<TARGET_FILE:tanc> -shared -I${CMAKE_CURRENT_LIST_DIR} -L$<TARGET_FILE_DIR:TanBackTrace>
        -lTanBackTrace
        -o libruntime.${LIB_EXT}
        ${RUNTIME_SRCS} ${RUNTIME_CXX_SRCS}
        WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
)
