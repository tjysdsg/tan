file(GLOB EXAMPLES ${CMAKE_CURRENT_LIST_DIR}/*)

foreach (example_dir ${EXAMPLES})
    if (IS_DIRECTORY ${example_dir})
        message(STATUS "Adding the example program from ${CMAKE_CURRENT_LIST_DIR}/${example_dir}")
        add_subdirectory(${example_dir})
    endif ()
endforeach ()
