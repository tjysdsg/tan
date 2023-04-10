# Resulting in a "tanc_exec_tests" executable and a "run_tanc_exec_tests" custom target
# ./tanc_exec_tests array.tan
#   compiles array.tan into a.out and run it

# run_tanc_exec_tests custom target compiles and runs all *.tan files in this directory

# target that compiles a tan test source, run the resulting executable, and check the return status
add_executable(tanc_exec_tests tanc_exec_tests.cpp ${PROJECT_SOURCE_DIR}/src/cli/cli_main.cpp)
set_target_common_compile_options(tanc_exec_tests)
target_link_libraries(tanc_exec_tests Tan gtest)
target_compile_definitions(tanc_exec_tests PRIVATE
        -DTANC_PATH=$<TARGET_FILE:tanc>
        -DTAN_PROJECT_SOURCE_DIR=${PROJECT_SOURCE_DIR}
        -DTAN_TEST_SOURCE_DIR=${CMAKE_CURRENT_LIST_DIR}
        )
add_dependencies(tanc_exec_tests runtime)

# target that runs this test on all .tan files
add_custom_target(run_tanc_exec_tests ALL DEPENDS tanc_exec_tests)

# ==================
# add positive tests
set(TEST_SRCS
        ${CMAKE_CURRENT_LIST_DIR}/arithmetic.tan
        ${CMAKE_CURRENT_LIST_DIR}/array.tan
        ${CMAKE_CURRENT_LIST_DIR}/assertion.tan
        ${CMAKE_CURRENT_LIST_DIR}/boolean.tan
        ${CMAKE_CURRENT_LIST_DIR}/function.tan
        ${CMAKE_CURRENT_LIST_DIR}/if_else.tan
        ${CMAKE_CURRENT_LIST_DIR}/intrinsics.tan
        ${CMAKE_CURRENT_LIST_DIR}/io.tan
        ${CMAKE_CURRENT_LIST_DIR}/loop.tan
        ${CMAKE_CURRENT_LIST_DIR}/pointers.tan
        ${CMAKE_CURRENT_LIST_DIR}/string.tan
        ${CMAKE_CURRENT_LIST_DIR}/struct.tan
        ${CMAKE_CURRENT_LIST_DIR}/type_system.tan
        ${CMAKE_CURRENT_LIST_DIR}/variables.tan
        ${CMAKE_CURRENT_LIST_DIR}/bits.tan
        ${CMAKE_CURRENT_LIST_DIR}/import.tan
        ${CMAKE_CURRENT_LIST_DIR}/symbol_resolution.tan
        )
foreach (test_src ${TEST_SRCS})
    add_custom_command(TARGET run_tanc_exec_tests POST_BUILD
            COMMAND LLVM_PROFILE_FILE=%p.profraw $<TARGET_FILE:tanc_exec_tests> ${test_src} 0
            WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
            )
endforeach ()

# add negative compilation tests
set(TEST_SRCS
        ${CMAKE_CURRENT_LIST_DIR}/fail_recursive_type_reference.tan
        )
foreach (test_src ${TEST_SRCS})
    add_custom_command(TARGET run_tanc_exec_tests POST_BUILD
            COMMAND LLVM_PROFILE_FILE=%p.profraw $<TARGET_FILE:tanc_exec_tests> ${test_src} 1
            WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
            )
endforeach ()
