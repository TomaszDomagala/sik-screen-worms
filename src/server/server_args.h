#ifndef SIK_SCREEN_WORMS_SERVER_ARGS_H
#define SIK_SCREEN_WORMS_SERVER_ARGS_H

#include <stdint.h>

typedef struct {
	int32_t server_port;
	int64_t rng_seed;
	int32_t turning_speed;
	int32_t rounds_per_sec;
	int32_t board_width;
	int32_t board_height;
} server_args_t;

int parse_server_args(int argc, char **argv, server_args_t *args);

#endif //SIK_SCREEN_WORMS_SERVER_ARGS_H
