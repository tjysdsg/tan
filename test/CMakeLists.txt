macro(enable_tests)
    # googletest
    add_subdirectory(${PROJECT_SOURCE_DIR}/dep/googletest)
    set(TEST_DIR ${PROJECT_SOURCE_DIR}/test)

    # lexer tests
    add_executable(lexer_tests ${TEST_DIR}/lexer_test.cpp)
    target_include_directories(lexer_tests PRIVATE ${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/include
            ${PROJECT_SOURCE_DIR}/test)
    target_link_libraries(lexer_tests TanSourceFile TanBase TanCommon TanLexer gtest ${DEP_LIBS})
    set_target_common_compile_options(lexer_tests)

    # parser tests
    add_executable(parser_tests ${TEST_DIR}/parser_test.cpp)
    target_include_directories(parser_tests PRIVATE ${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/include
            ${PROJECT_SOURCE_DIR}/test)
    target_link_libraries(parser_tests TanParser TanSourceFile TanBase TanAST TanCommon TanLexer gtest ${DEP_LIBS})
    set_target_common_compile_options(parser_tests)

    # This tells the executable where to write test coverage output
    set(RUN_TEST_COMMAND_PREFIX ${CMAKE_COMMAND} -E env LLVM_PROFILE_FILE=%p.profraw)

    # tanc exec tests
    add_subdirectory(${PROJECT_SOURCE_DIR}/test/test_exec)
    # tanc library tests
    add_subdirectory(${PROJECT_SOURCE_DIR}/test/test_lib)

    # tests target that runs all defined tests
    add_custom_target(tests ALL DEPENDS lexer_tests parser_tests run_tanc_exec_tests run_tanc_lib_tests)
    add_custom_command(TARGET tests POST_BUILD
            COMMAND ${RUN_TEST_COMMAND_PREFIX} $<TARGET_FILE:lexer_tests>
            WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
            )
    add_custom_command(TARGET tests POST_BUILD
            COMMAND ${RUN_TEST_COMMAND_PREFIX} $<TARGET_FILE:parser_tests>
            WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
            )

    # add extra dependencies between test run targets so that they are run sequentially
    add_dependencies(run_tanc_lib_tests run_tanc_exec_tests)

    # generate coverage
    if (DEFINED ENABLE_COVERAGE)
        add_custom_target(coverage ALL
                DEPENDS tests
                WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})

        # get_property(all_targets GLOBAL PROPERTY ALL_TAN_TARGETS)
        set(all_targets tanc)
        foreach (target ${all_targets})
            set(LLVM_COV_ARGS ${LLVM_COV_ARGS} --object $<TARGET_FILE:${target}>)
        endforeach ()

        add_custom_command(TARGET coverage POST_BUILD
                COMMAND llvm-profdata-15 merge -sparse *.profraw -o tan.profdata
                COMMAND llvm-cov-15 export ${LLVM_COV_ARGS} --instr-profile=tan.profdata --format=lcov > coverage.txt
                WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
                )
    endif ()
endmacro()
