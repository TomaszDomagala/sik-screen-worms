#ifndef SIK_SCREEN_WORMS_GAME_H
#define SIK_SCREEN_WORMS_GAME_H

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include "utils/list.h"
#include "messages.h"

// Player status flags.
#define PS_WAITING 0x1
#define PS_IN_GAME 0x2
#define PS_DEAD 0x4
#define PS_DISCONNECTED 0x8


#define PD_FORWARD 0
#define PD_RIGHT 1
#define PD_LEFT 2

typedef struct game_s game_t;

typedef struct {
	uint8_t status;
	uint64_t session_id;
	int8_t player_name[21];

	uint8_t turn_direction;
	uint32_t direction;
	double x_pos;
	double y_pos;
} player_t;

game_t *game_create(size_t width, size_t height);

bool game_in_progress(game_t *game);

bool game_add_player(game_t *game, uint64_t session_id, int8_t *player_name);

bool game_set_turn_direction(game_t *game, uint64_t session_id, uint8_t turn_direction);

bool game_remove_player(game_t *game, uint64_t session_id);

list_t *game_tick(game_t *game);

bool game_restart(game_t *game);

uint32_t game_get_id(game_t *game);


#endif //SIK_SCREEN_WORMS_GAME_H
