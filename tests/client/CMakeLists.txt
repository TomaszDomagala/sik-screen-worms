file(GLOB TEST_SRCS *.c)

foreach (test_file IN ITEMS ${TEST_SRCS})
    get_filename_component(test_name ${test_file} NAME)
    set(test_target ${test_name}_target)
    add_executable(${test_target} ${test_file})
    target_link_libraries(${test_target} sik-client unity)
    add_test(${test_name} ${test_target})
endforeach ()
