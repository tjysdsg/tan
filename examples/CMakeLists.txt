file(GLOB EXAMPLES ${CMAKE_CURRENT_LIST_DIR}/*)

foreach (example_dir ${EXAMPLES})
    if (IS_DIRECTORY ${example_dir})
        get_filename_component(example_name ${example_dir} NAME)

        set(example_c_target_name example_c_${example_name})
        # target for compiling C/CXX sources
        aux_source_directory(${example_dir} c_srcs)
        add_library(${example_c_target_name} OBJECT ${c_srcs})
        set_other_options(${example_c_target_name})

        # example tan sources
        file(GLOB_RECURSE tan_srcs ${CMAKE_CURRENT_LIST_DIR}/*.tan)
        string(REPLACE ";" " " tan_srcs_str "${tan_srcs}")
        add_custom_target(example_${example_name} ALL DEPENDS tanc runtime ${example_c_target_name}
                SOURCES ${tan_srcs}
                COMMAND ${PROJECT_SOURCE_DIR}/bin/tanc -I${CMAKE_CURRENT_LIST_DIR}
                --print-ir -l${PROJECT_SOURCE_DIR}/runtime/runtime.so --obj ${tan_srcs_str}
                COMMAND ${CMAKE_CXX_COMPILER} *.o $<TARGET_OBJECTS:${example_c_target_name}> -o ${example_name}
                WORKING_DIRECTORY ${example_dir}
                )
    endif ()
endforeach ()
