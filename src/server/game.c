#include "game.h"

#include <stdint.h>
#include "config.h"
#include "utils/stack.h"
#include "utils/list.h"
#include "game_board.h"

#define MAX_PLAYERS_NUM SERVER_MAX_CAPACITY

#define GE_NEW_GAME 0
#define GE_PIXEL 1
#define GE_PLAYER_ELIMINATED 2
#define GE_GAME_OVER 3

typedef struct {
    uint32_t max_x;
    uint32_t max_y;
} ge_data_new_game_t;

typedef struct {
    uint8_t player_number;
    uint32_t x;
    uint32_t y;
} ge_data_pixel_t;

typedef struct {
    uint8_t player_number;
} ge_data_player_eliminated_t;

typedef union {
    ge_data_new_game_t new_game;
    ge_data_pixel_t pixel;
    ge_data_player_eliminated_t player_eliminated;
} game_event_data_t;


struct {
    uint8_t type;
    game_event_data_t data;
} game_event_t;

struct game_s {
    uint32_t game_id;
    list_t *players;
    stack_t *events;
    game_board_t *board;
};