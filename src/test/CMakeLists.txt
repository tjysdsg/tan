# ====== tests for debug build ======
if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    find_package(GTest)
    aux_source_directory(${CMAKE_CURRENT_LIST_DIR} TEST_SRCS)
    add_executable(tan_tests ${TEST_SRCS} ${PROJECT_SOURCE_DIR}/src/cli/cli_main.cpp)
    add_dependencies(tan_tests runtime)
    target_include_directories(tan_tests PRIVATE ${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/include
            ${CMAKE_CURRENT_LIST_DIR})
    target_link_libraries(tan_tests tan gtest_main gflags::gflags ${DEP_LIBS})
    set_other_options(tan_tests)
    gtest_discover_tests(tan_tests WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
    enable_testing()
endif ()

