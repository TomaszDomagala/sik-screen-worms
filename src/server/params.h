#ifndef SIK_SCREEN_WORMS_CLIENT_ARGS_H
#define SIK_SCREEN_WORMS_PARAMS_H

#include <stdint.h>

typedef struct {
	int32_t server_port;
	int32_t rng_seed;
	int32_t turning_speed;
	int32_t rounds_per_sec;
	int32_t board_width;
	int32_t board_height;
} server_parameters_t;

int parse_server_args(int argc, char **argv, server_parameters_t *params);

#endif //SIK_SCREEN_WORMS_CLIENT_ARGS_H
