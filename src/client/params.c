#include "params.h"
#include <unistd.h>
#include <stdlib.h>
#include <getopt.h>
#include <stdbool.h>
#include <string.h>

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


client_parse_error_t parse_client_args(int argc, char **argv, client_parameters_t *params) {
	bool player_name_set = false;
	int opt;
	char *err_ptr = NULL;

	params->server_port = 2021;
	params->gui_address = "localhost";
	params->gui_port = 20210;

	while ((opt = getopt(argc, argv, "n:p:i:r:")) != -1) {
		switch (opt) {
			case 'n': // player_name
				if (!player_name_valid(optarg)) {
					return cpe_InvalidPlayerName;
				}
				player_name_set = true;
				params->player_name = optarg;
				break;
			case 'p': // server_port
				params->server_port = strtol(optarg, &err_ptr, 10);
				if (err_ptr != NULL) {
					return cpe_InvalidArgument;
				}
				break;
			case 'i': // gui_server_address
				params->gui_address = optarg;
				break;
			case 'r': // gui_port
				params->gui_port = strtol(optarg, &err_ptr, 10);
				if (err_ptr != NULL) {
					return cpe_InvalidArgument;
				}
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
	params->server_address = argv[optind];
	return cpe_Success;
}