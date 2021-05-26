cmake_minimum_required(VERSION 3.10)
project(sik_screen_worms C)

set(CMAKE_C_STANDARD 11)
#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
enable_testing()

add_subdirectory(src)
add_subdirectory(tests)

set_target_properties(
        screen-worms-client screen-worms-server
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)
