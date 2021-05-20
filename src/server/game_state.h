#ifndef SIK_SCREEN_WORMS_GAME_STATE_H
#define SIK_SCREEN_WORMS_GAME_STATE_H

#include <stdint.h>

// Player status flags.
#define PS_NULL 0x0
#define PS_READY 0x1
#define PS_ALIVE 0x2
#define PS_DEAD 0x4

typedef struct game_state_s game_state_t;

typedef struct {
    uint8_t status;
    uint64_t session_id;
    int8_t player_name[21];

    uint8_t turn_direction;
    double x_pos;
    double y_pos;
} player_t;


#endif //SIK_SCREEN_WORMS_GAME_STATE_H
