file(GLOB EXAMPLES ${CMAKE_CURRENT_LIST_DIR}/*)

foreach (example_dir ${EXAMPLES})
    if (IS_DIRECTORY ${example_dir})
        get_filename_component(example_name ${example_dir} NAME)
        aux_source_directory(${example_dir} c_srcs)
        file(GLOB_RECURSE tan_srcs ${CMAKE_CURRENT_LIST_DIR}/*.tan)
        string(REPLACE ";" " " tan_srcs_str "${tan_srcs} ${c_srcs}")
        add_custom_target(example_${example_name} ALL DEPENDS tanc runtime
                SOURCES ${tan_srcs}
                COMMAND $<TARGET_FILE:tanc> -I${CMAKE_CURRENT_LIST_DIR}
                -L${PROJECT_SOURCE_DIR}/runtime -lruntime ${tan_srcs_str}
                WORKING_DIRECTORY ${example_dir}
                )
    endif ()
endforeach ()
