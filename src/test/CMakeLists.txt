macro(enable_tests)
    if (${ENABLE_COVERAGE})
        message(STATUS "Enabling test coverage")
        set(COVERAGE_CXX_FLAGS "-g -O0 -Wall -fprofile-arcs -ftest-coverage")
    else ()
        set(COVERAGE_CXX_FLAGS "-g -O0 -Wall")
    endif ()
    set(TEST_DIR ${PROJECT_SOURCE_DIR}/src/test)

    # lexer tests
    add_executable(lexer_tests ${TEST_DIR}/lexer_test.cpp)
    target_include_directories(lexer_tests PRIVATE ${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/include
            ${PROJECT_SOURCE_DIR}/src/test)
    target_link_libraries(lexer_tests tan gtest ${DEP_LIBS})
    set_other_options(lexer_tests)

    # parser tests
    add_executable(parser_tests ${TEST_DIR}/parser_test.cpp)
    target_include_directories(parser_tests PRIVATE ${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/include
            ${PROJECT_SOURCE_DIR}/src/test)
    target_link_libraries(parser_tests tan gtest ${DEP_LIBS})
    set_other_options(parser_tests)

    # tanc tests
    file(GLOB TAN_TEST_SRCS ${TEST_DIR}/test_src/*.tan)
    set_property(SOURCE ${ALL_SRCS} PROPERTY COMPILE_FLAGS ${COVERAGE_CXX_FLAGS})
    add_executable(tanc_tests ${TEST_DIR}/tanc_tests.cpp ${PROJECT_SOURCE_DIR}/src/cli/cli_main.cpp)
    set_other_options(tanc_tests)
    target_link_libraries(tanc_tests tan gtest)
    target_compile_definitions(tanc_tests PRIVATE
            -DTAN_PROJECT_SOURCE_DIR=${PROJECT_SOURCE_DIR}
            )
    add_dependencies(tanc_tests runtime)

    # tests target
    add_custom_target(tests ALL DEPENDS lexer_tests tanc_tests)
    add_custom_command(TARGET tests POST_BUILD
            COMMAND ${PROJECT_SOURCE_DIR}/bin/lexer_tests
            WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
            )
    foreach (tt ${TAN_TEST_SRCS})
        add_custom_command(TARGET tests POST_BUILD
                COMMAND ${PROJECT_SOURCE_DIR}/bin/tanc_tests ${tt}
                WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
                )
    endforeach ()

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
