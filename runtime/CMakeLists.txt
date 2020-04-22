set(RUNTIME_SRCS
        print.tan
        stdlib.tan
        debug.tan)
add_custom_target(runtime ALL DEPENDS tanc SOURCES ${RUNTIME_SRCS})
add_custom_command(TARGET runtime
        COMMAND ${PROJECT_SOURCE_DIR}/bin/tanc -o runtime.so --type=dlib ${RUNTIME_SRCS}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
