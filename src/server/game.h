#ifndef SIK_SCREEN_WORMS_GAME_H
#define SIK_SCREEN_WORMS_GAME_H

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

// Player status flags.
#define PS_NULL 0x0
#define PS_WAITING 0x1
#define PS_ALIVE 0x2
#define PS_DEAD 0x4

// Game states.
#define GS_WAITING 0
#define GS_IN_PROGRESS 1
#define GS_OVER 2

#define PD_FORWARD 0
#define PD_RIGHT 1
#define PD_LEFT 2

typedef struct game_s game_t;

typedef struct {
    uint8_t status;
    uint64_t session_id;
    int8_t player_name[21];

    uint8_t turn_direction;
    double x_pos;
    double y_pos;
} player_t;

game_t *game_create(uint32_t game_id, size_t width, size_t height);

uint8_t game_state(game_t *game);

bool game_add_player(game_t *game, uint64_t session_id, int8_t *player_name);

bool game_set_direction(game_t *game, uint64_t session_id, uint8_t turn_direction);

bool game_remove_player(game_t *game, uint64_t session_id);

bool game_tick(game_t *game);


#endif //SIK_SCREEN_WORMS_GAME_H
