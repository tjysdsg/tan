macro(enable_tests)
    # googletest
    add_subdirectory(${PROJECT_SOURCE_DIR}/dep/googletest)
    set(TEST_DIR ${PROJECT_SOURCE_DIR}/test)

    # ==========
    # Unit tests
    # ==========
    add_executable(unit_test ${TEST_DIR}/unit_test.cpp)
    target_include_directories(
            unit_test
            PRIVATE
            ${TEST_DIR}
            ${PROJECT_SOURCE_DIR}
            ${PROJECT_SOURCE_DIR}/include
    )
    target_link_libraries(
            unit_test
            PRIVATE
            TanBase
            TanCommon
            TanSourceFile
            TanLexer
            TanParser
            TanAST
            gtest
            ${DEP_LIBS}
    )
    set_target_common_compile_options(unit_test)

    # ============
    # System tests
    # ============
    # This tells the executable where to write test coverage output
    set(RUN_TEST_COMMAND_PREFIX ${CMAKE_COMMAND} -E env LLVM_PROFILE_FILE=%p.profraw)
    add_subdirectory(${TEST_DIR}/test_exec)
    add_subdirectory(${TEST_DIR}/test_lib)

    # ======================
    # Build-time Test runner
    # ======================
    add_custom_target(tests ALL DEPENDS unit_test run_tanc_exec_tests run_tanc_lib_tests)
    add_custom_command(
            TARGET tests POST_BUILD
            COMMAND ${RUN_TEST_COMMAND_PREFIX} $<TARGET_FILE:unit_test>
            WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    )

    # add extra dependencies between test run targets so that they are run sequentially
    add_dependencies(run_tanc_lib_tests run_tanc_exec_tests)

    # ========
    # Coverage
    # ========
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
