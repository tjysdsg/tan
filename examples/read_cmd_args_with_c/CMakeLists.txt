add_custom_target(
        example_read_cmd_args_with_c ALL DEPENDS tanc runtime
        SOURCES print_args.c main.tan
        COMMAND $<TARGET_FILE:tanc> -I${CMAKE_CURRENT_LIST_DIR}
        -L${PROJECT_SOURCE_DIR}/runtime -lruntime print_args.c main.tan
        WORKING_DIRECTORY ${example_dir}
)
