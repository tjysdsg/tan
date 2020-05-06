macro(enable_tests)
    set(COVERAGE_CXX_FLAGS "-g -O0 -Wall -fprofile-arcs -ftest-coverage")
    set(TEST_DIR ${PROJECT_SOURCE_DIR}/src/test)

    # lexer tests
    add_executable(lexer_tests ${TEST_DIR}/lexer_test.cpp)
    target_include_directories(lexer_tests PRIVATE ${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/include
            ${PROJECT_SOURCE_DIR}/src/test)
    target_link_libraries(lexer_tests tan gtest_main ${DEP_LIBS})
    set_other_options(lexer_tests)

    # tanc tests
    set(TANC_ALL_TESTS "lexer_tests")
    file(GLOB TAN_TEST_SRCS ${TEST_DIR}/test_src/*.tan)
    set_property(SOURCE ${ALL_SRCS} PROPERTY COMPILE_FLAGS ${COVERAGE_CXX_FLAGS})
    foreach (tts ${TAN_TEST_SRCS})
        get_filename_component(test_name ${tts} NAME_WLE)
        set(test_target_name "test_tanc_${test_name}")
        add_executable(${test_target_name}
                ${TEST_DIR}/tanc_tests.cpp
                ${PROJECT_SOURCE_DIR}/src/cli/cli_main.cpp)
        set_other_options(${test_target_name})
        target_link_libraries(${test_target_name} tan gtest_main gflags::gflags ${DEP_LIBS})
        target_compile_definitions(${test_target_name} PRIVATE -DTAN_SOURCE=${tts})
        set(TANC_ALL_TESTS ${TANC_ALL_TESTS} ${test_target_name})
    endforeach ()

    # tests target
    add_custom_target(tests ALL DEPENDS ${TANC_ALL_TESTS})
    foreach (tt ${TANC_ALL_TESTS})
        add_custom_command(TARGET tests POST_BUILD
                COMMAND ${PROJECT_SOURCE_DIR}/bin/${tt}
                WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
                )
    endforeach ()

    add_custom_target(coverage ALL
            COMMAND mkdir -p ${PROJECT_SOURCE_DIR}/coverage
            DEPENDS tests
            WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
            )

    add_custom_command(TARGET coverage POST_BUILD
            COMMAND gcovr --html --html-details
            -r ${PROJECT_SOURCE_DIR}
            --object-directory=${CMAKE_BINARY_DIR}
            -o ${PROJECT_SOURCE_DIR}/coverage/index.html
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            )
endmacro()
