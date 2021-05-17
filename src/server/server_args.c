#include "server_args.h"
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <getopt.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include "utils/parse.h"

#define MAX_PORT 65535
#define MAX_TURNING_SPEED 30000
#define MAX_ROUNDS_PER_SEC 1000
#define MAX_WIDTH 3840
#define MAX_HEIGHT 2160

int parse_server_args(int argc, char **argv, server_args_t *args) {
	int opt;

	// Set default values.
	args->server_port = 2021;
	args->rng_seed = time(NULL);
	args->turning_speed = 6;
	args->rounds_per_sec = 50;
	args->board_width = 640;
	args->board_height = 480;

	optind = 1;
	while ((opt = getopt(argc, argv, "p:s:t:v:w:h:")) != -1) {
		switch (opt) {
			case 'p': {
				p_u64_t port = str_to_u64(optarg);
				if (!port.success || port.value > MAX_PORT)
					return -1;
				args->server_port = port.value;
				break;
			}
			case 's': {
				p_i64_t seed = str_to_i64(optarg);
				if (!seed.success)
					return -1;
				args->rng_seed = seed.value;
				break;
			}
			case 't': {
				p_u64_t turning_speed = str_to_u64(optarg);
				if (!turning_speed.success || turning_speed.value > MAX_TURNING_SPEED)
					return -1;
				args->turning_speed = turning_speed.value;
				break;
			}
			case 'v': {
				p_u64_t rounds_per_sec = str_to_u64(optarg);
				if (!rounds_per_sec.success || rounds_per_sec.value > MAX_ROUNDS_PER_SEC)
					return -1;
				args->rounds_per_sec = rounds_per_sec.value;
				break;
			}
			case 'w': {
				p_u64_t width = str_to_u64(optarg);
				if (!width.success || width.value > MAX_WIDTH)
					return -1;
				args->board_width = width.value;
				break;
			}
			case 'h': {
				p_u64_t height = str_to_u64(optarg);
				if (!height.success || height.value > MAX_HEIGHT)
					return -1;
				args->board_height = height.value;
				break;
			}
			default:
				return -1;
		}
	}

	return 0;
}


