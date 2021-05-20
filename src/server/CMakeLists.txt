add_library(sik-server server_args.c clients.c game.c game_board.c config.h)
target_include_directories(sik-server PUBLIC ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(sik-server sik-common)

add_executable(screen-worms-server main.c)
target_link_libraries(screen-worms-server sik-server sik-common)
