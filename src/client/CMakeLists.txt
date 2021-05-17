add_library(sik-client client_args.c gui_client.c)
target_include_directories(sik-client PUBLIC ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(sik-client sik-common)

add_executable(screen-worms-client main.c)
target_link_libraries(screen-worms-client sik-client sik-common)

