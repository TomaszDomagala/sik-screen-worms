#ifndef SIK_SCREEN_WORMS_PARAMS_H
#define SIK_SCREEN_WORMS_PARAMS_H

#include <stdint.h>

typedef struct {
	char *player_name;

	char *server_address;
	int32_t server_port;

	char *gui_address;
	int32_t gui_port;
} client_parameters_t;

typedef enum {
	cpe_Success, cpe_MissingPlayerName, cpe_MissingServerAddress, cpe_InvalidPlayerName, cpe_InvalidArgument
} client_parse_error_t;

client_parse_error_t parse_client_args(int argc, char **argv, client_parameters_t *params);

#endif //SIK_SCREEN_WORMS_PARAMS_H
