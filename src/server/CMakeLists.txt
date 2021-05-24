add_library(sik-server server_args.c clients.c game.c game_board.c rand.c config.h events_storage.c)
target_include_directories(sik-server PUBLIC ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(sik-server sik-common)
target_link_libraries(sik-server m)

add_executable(screen-worms-server main.c)
target_link_libraries(screen-worms-server sik-server sik-common)
