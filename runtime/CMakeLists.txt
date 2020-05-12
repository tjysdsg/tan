aux_source_directory(${CMAKE_CURRENT_LIST_DIR} RUNTIME_CXX_SRCS)
add_library(tan_rt_in_cpp OBJECT ${RUNTIME_CXX_SRCS})
set_other_options(tan_rt_in_cpp)

file(GLOB_RECURSE RUNTIME_SRCS ${CMAKE_CURRENT_LIST_DIR}/*.tan)
add_custom_target(runtime ALL DEPENDS tanc tan_rt_in_cpp SOURCES ${RUNTIME_SRCS})
add_custom_command(TARGET runtime
        COMMAND ${PROJECT_SOURCE_DIR}/bin/tanc -I${CMAKE_CURRENT_LIST_DIR} --obj --print-ir ${RUNTIME_SRCS}
        COMMAND ${CMAKE_CXX_COMPILER} *.o $<TARGET_OBJECTS:tan_rt_in_cpp> -lunwind -shared -o runtime.so
        WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
        )
