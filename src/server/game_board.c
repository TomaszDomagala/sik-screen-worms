#include "game_board.h"

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

struct game_board_s {
    size_t width;
    size_t height;
    uint8_t data[];
};

game_board_t *game_board_create(size_t width, size_t height) {
    size_t pixel_num = width * height;
    size_t board_size = pixel_num / 8;
    if (pixel_num % 8 != 0)
        board_size++;

    game_board_t *board = malloc(sizeof(game_board_t) + sizeof(uint8_t) * board_size);
    if (board == NULL)
        return NULL;

    board->width = width;
    board->height = height;
    memset(board->data, 0, board_size);

    return board;
}

void game_board_free(game_board_t *board) {
    free(board);
}

void game_board_set(game_board_t *board, size_t x, size_t y) {
    assert(x < board->width);
    assert(y < board->height);

    size_t index = board->width * y + x;
    size_t cell_index = index / 8;
    size_t pixel_index = index % 8;

    board->data[cell_index] |= 1 << pixel_index;
}

bool game_board_get(game_board_t *board, size_t x, size_t y) {
    assert(x < board->width);
    assert(y < board->height);

    size_t index = board->width * y + x;
    size_t cell_index = index / 8;
    size_t pixel_index = index % 8;

    return (board->data[cell_index] & 1 << pixel_index) > 0;
}




