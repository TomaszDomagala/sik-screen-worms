#ifndef SIK_SCREEN_WORMS_GAME_BOARD_H
#define SIK_SCREEN_WORMS_GAME_BOARD_H

#include <stdlib.h>
#include <stdbool.h>

typedef struct game_board_s game_board_t;

game_board_t *game_board_create(size_t width, size_t height);

void game_board_free(game_board_t *board);

void game_board_set(game_board_t *board, size_t x, size_t y);

bool game_board_get(game_board_t *board, size_t x, size_t y);

#endif //SIK_SCREEN_WORMS_GAME_BOARD_H
