find_package(Doxygen)
if (DOXYGEN_FOUND)
    # set input and output files
    set(DOXYGEN_IN ${CMAKE_CURRENT_LIST_DIR}/Doxygen.in)
    set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

    # config variables in Doxygen.in
    set(DOXYGEN_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})
    set(DOXYGEN_SOURCE_DIRECTORY "${PROJECT_SOURCE_DIR}/src ${PROJECT_SOURCE_DIR}/include")
    set(DOXYGEN_INCLUDE_DIRECTORY "${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR}")

    # generate Doxyfile
    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)

    add_custom_target(doxygen
            COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Generating API documentation with Doxygen"
            )
else ()
    message(STATUS "Not generating documentation because Doxygen is not installed")
endif ()
