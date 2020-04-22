set(RUNTIME_SRCS
        ${CMAKE_CURRENT_LIST_DIR}/print.tan
        ${CMAKE_CURRENT_LIST_DIR}/stdlib.tan
        ${CMAKE_CURRENT_LIST_DIR}/debug.tan)
add_custom_target(runtime ALL DEPENDS tanc SOURCES ${RUNTIME_SRCS})
add_custom_command(TARGET runtime
        COMMAND ${PROJECT_SOURCE_DIR}/bin/tanc -o ${CMAKE_CURRENT_LIST_DIR}/runtime.so --type=dlib ${RUNTIME_SRCS}
        WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})
