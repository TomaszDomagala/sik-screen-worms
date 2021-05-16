#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include "client_args.h"
#include "err.h"

/**
 * Returns if result is success.
 * Prints error message and exits on error.
 */
void handle_parse_result(cp_res_t result) {
	if (result == cpr_Success)
		return;
	fprintf(stderr, "Usage: ./screen-worms-client game_server [-n player_name] [-p n] [-i gui_server] [-r n]\n");
	switch (result) {
		case cpr_MissingPlayerName:
			fatal("missing [ -n player_name ] argument\n");
		case cpr_MissingServerAddress:
			fatal("missing game_server argument\n");
		case cpr_InvalidPlayerName:
			fatal("invalid [ -n player_name ] argument\n");
		case cpr_InvalidArgument:
			fatal("invalid argument\n");
		default:
			assert(false);
	}
}

int main(int argc, char *argv[]) {
	client_args_t args;

	cp_res_t res = parse_client_args(argc, argv, &args);
	handle_parse_result(res);

}
