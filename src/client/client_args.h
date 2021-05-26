#ifndef SIK_SCREEN_WORMS_CLIENT_ARGS_H
#define SIK_SCREEN_WORMS_CLIENT_ARGS_H

#include <stdint.h>

typedef struct {
	char *player_name;

	char *server_address;
	int32_t server_port;

	char *gui_address;
	int32_t gui_port;
} client_args_t;

typedef enum {
	cpr_Success, cpr_MissingServerAddress, cpr_InvalidPlayerName, cpr_InvalidArgument
} cp_res_t;

cp_res_t parse_client_args(int argc, char **argv, client_args_t *args);

#endif //SIK_SCREEN_WORMS_CLIENT_ARGS_H
