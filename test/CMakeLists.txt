macro(enable_tests)
    # googletest
    add_subdirectory(${PROJECT_SOURCE_DIR}/dep/googletest)

    if (${ENABLE_COVERAGE})
        message(STATUS "Enabling test coverage")
        set(COVERAGE_CXX_FLAGS "-g -O0 -Wall -fprofile-arcs -ftest-coverage")
    else ()
        set(COVERAGE_CXX_FLAGS "-g -O0 -Wall")
    endif ()
    set_property(SOURCE ${ALL_SRCS} PROPERTY COMPILE_FLAGS ${COVERAGE_CXX_FLAGS})
    set(TEST_DIR ${PROJECT_SOURCE_DIR}/test)

    # lexer tests
    add_executable(lexer_tests ${TEST_DIR}/lexer_test.cpp)
    target_include_directories(lexer_tests PRIVATE ${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/include
            ${PROJECT_SOURCE_DIR}/test)
    target_link_libraries(lexer_tests Tan gtest ${DEP_LIBS})
    set_target_common_compile_options(lexer_tests)

    # parser tests
    add_executable(parser_tests ${TEST_DIR}/parser_test.cpp)
    target_include_directories(parser_tests PRIVATE ${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/include
            ${PROJECT_SOURCE_DIR}/test)
    target_link_libraries(parser_tests Tan gtest ${DEP_LIBS})
    set_target_common_compile_options(parser_tests)

    # tanc exec tests
    add_subdirectory(${PROJECT_SOURCE_DIR}/test/test_exec)
    # tanc library tests
    add_subdirectory(${PROJECT_SOURCE_DIR}/test/test_lib)

    # tests target that runs all defined tests
    add_custom_target(tests ALL DEPENDS lexer_tests run_tanc_exec_tests run_tanc_lib_tests)
    add_custom_command(TARGET tests POST_BUILD
            COMMAND ${PROJECT_SOURCE_DIR}/bin/lexer_tests
            WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
            )

    # generate coverage
    if (DEFINED ENABLE_COVERAGE)
        add_custom_target(coverage ALL
                COMMAND mkdir -p ${PROJECT_SOURCE_DIR}/coverage
                DEPENDS tests
                WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
                )
        add_custom_command(TARGET coverage POST_BUILD
                COMMAND gcovr --xml
                -r ${PROJECT_SOURCE_DIR}
                --object-directory=${CMAKE_BINARY_DIR}
                -o ${PROJECT_SOURCE_DIR}/coverage/report.xml
                COMMAND gcovr --html --html-details
                -r ${PROJECT_SOURCE_DIR}
                --object-directory=${CMAKE_BINARY_DIR}
                -o ${PROJECT_SOURCE_DIR}/coverage/report.html
                WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                )
    endif ()
endmacro()
