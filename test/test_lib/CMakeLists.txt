# Resulting in
# 1. a "test_compile_lib" executable that compiles lib1.tan and lib2.tan into test.lib
# 2. a "test_use_lib" executable that
#      1. compiles cli.tan and link it to test.lib, then generate a.out
#      2. runs a.out
# 3. "run_tanc_lib_tests" custom target that runs the above two steps during build

add_executable(test_compile_lib test_compile_lib.cpp ${PROJECT_SOURCE_DIR}/src/cli/cli_main.cpp)
set_target_common_compile_options(test_compile_lib)
target_link_libraries(test_compile_lib Tan gtest)
target_compile_definitions(test_compile_lib PRIVATE
        -DTANC_PATH=$<TARGET_FILE:tanc>
        -DTAN_PROJECT_SOURCE_DIR=${PROJECT_SOURCE_DIR}
        -DTAN_TEST_SOURCE_DIR=${CMAKE_CURRENT_LIST_DIR}
        )
add_dependencies(test_compile_lib runtime)

add_executable(test_use_lib test_use_lib.cpp ${PROJECT_SOURCE_DIR}/src/cli/cli_main.cpp)
add_dependencies(test_use_lib test_compile_lib)
set_target_common_compile_options(test_use_lib)
target_link_libraries(test_use_lib Tan gtest)
target_compile_definitions(test_use_lib PRIVATE
        -DTANC_PATH=$<TARGET_FILE:tanc>
        -DTAN_PROJECT_SOURCE_DIR=${PROJECT_SOURCE_DIR}
        -DTAN_TEST_SOURCE_DIR=${CMAKE_CURRENT_LIST_DIR}
        )
add_dependencies(test_use_lib runtime)

# target that runs this test
add_custom_target(run_tanc_lib_tests ALL DEPENDS test_compile_lib test_use_lib)
add_custom_command(TARGET run_tanc_lib_tests POST_BUILD
        COMMAND $<TARGET_FILE:test_compile_lib>
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
        )
add_custom_command(TARGET run_tanc_lib_tests POST_BUILD
        COMMAND $<TARGET_FILE:test_use_lib>
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
        )
