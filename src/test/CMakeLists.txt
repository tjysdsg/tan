# ====== tests for debug build ======
if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    # lexer_tests
    find_package(GTest)
    add_executable(lexer_tests lexer_test.cpp)
    add_dependencies(lexer_tests runtime)
    target_include_directories(lexer_tests PRIVATE ${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/include
            ${CMAKE_CURRENT_LIST_DIR})
    target_link_libraries(lexer_tests tan gtest_main ${DEP_LIBS})
    set_other_options(lexer_tests)
    gtest_discover_tests(lexer_tests WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
    enable_testing()
endif ()
