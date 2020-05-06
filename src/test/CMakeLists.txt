# ====== tests for debug build ======
if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    set(CMAKE_CXX_FLAGS
            "-g -O0 -Wall -fprofile-arcs -ftest-coverage"
            PARENT_SCOPE)
    set(ENV{GCOV_PREFIX} ${PROJECT_SOURCE_DIR}/coverage)

    # lexer tests
    add_executable(lexer_tests lexer_test.cpp)
    target_include_directories(lexer_tests PRIVATE ${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/include
            ${CMAKE_CURRENT_LIST_DIR})
    target_link_libraries(lexer_tests tan gtest_main ${DEP_LIBS})
    set_other_options(lexer_tests)

    # tanc tests
    set(TANC_ALL_TESTS "lexer_tests")
    file(GLOB TAN_TEST_SRCS ${CMAKE_CURRENT_LIST_DIR}/test_src/*.tan)
    foreach (tts ${TAN_TEST_SRCS})
        get_filename_component(test_name ${tts} NAME_WLE)
        set(test_target_name "test_tanc_${test_name}")
        add_executable(${test_target_name}
                ${CMAKE_CURRENT_LIST_DIR}/tanc_tests.cpp
                ${PROJECT_SOURCE_DIR}/src/cli/cli_main.cpp)
        set_other_options(${test_target_name})
        target_link_libraries(${test_target_name} tan gtest_main gflags::gflags ${DEP_LIBS})
        target_compile_definitions(${test_target_name} PRIVATE -DTAN_SOURCE=${tts})
        set(TANC_ALL_TESTS ${TANC_ALL_TESTS} ${test_target_name})
    endforeach ()

    # tests target
    add_custom_target(tests DEPENDS ${TANC_ALL_TESTS})
    foreach (tt ${TANC_ALL_TESTS})
        add_custom_command(TARGET tests POST_BUILD
                COMMAND ${PROJECT_SOURCE_DIR}/bin/${tt}
                WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
                )
    endforeach ()

    add_custom_target(coverage
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
endif ()
