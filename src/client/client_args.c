#include "client_args.h"
#include <unistd.h>
#include <stdlib.h>
#include <getopt.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>

/**
 * Valid player name is a 1-20 long ascii string with values between 33-126.
 */
bool player_name_valid(char *player_name) {
	if (player_name == NULL)
		return false;
	unsigned long len = strlen(player_name);
	if (len == 0 || len > 20)
		return false;
	for (int i = 0; i < len; ++i) {
		if (player_name[i] < 33 || player_name[i] > 126)
			return false;
	}
	return true;
}

uint32_t read_port(char *port_str) {
	char *err_ptr = NULL;

	errno = 0;
	long res = strtol(port_str, &err_ptr, 10);
	if (errno != 0 || err_ptr == port_str || *err_ptr != '\0') {
		return -1;
	}
	if (res < 0) {
		return -1;
	}
	return res;
}


client_parse_error_t parse_client_args(int argc, char **argv, client_args_t *args) {
	bool player_name_set = false;
	int opt;

	args->server_port = 2021;
	args->gui_address = "localhost";
	args->gui_port = 20210;

	optind = 1;
	while ((opt = getopt(argc, argv, "n:p:i:r:")) != -1) {
		switch (opt) {
			case 'n': // player_name
				if (!player_name_valid(optarg)) {
					return cpe_InvalidPlayerName;
				}
				player_name_set = true;
				args->player_name = optarg;
				break;
			case 'p': // server_port
				args->server_port = read_port(optarg);
				if (args->server_port == -1)
					return cpe_InvalidArgument;
				break;
			case 'i': // gui_server_address
				args->gui_address = optarg;
				break;
			case 'r': // gui_port
				args->gui_port = read_port(optarg);
				if (args->gui_port == -1)
					return cpe_InvalidArgument;
				break;
			default: // ?
				return cpe_InvalidArgument;
		}
	}
	if (!player_name_set) {
		return cpe_MissingPlayerName;
	}
	if (optind >= argc) {
		return cpe_MissingServerAddress;
	}
	args->server_address = argv[optind];
	return cpe_Success;
}