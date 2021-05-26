#ifndef SIK_SCREEN_WORMS_GAME_CLIENT_H
#define SIK_SCREEN_WORMS_GAME_CLIENT_H

#include "messages.h"
#include "list.h"

typedef struct game_client_s game_client_t;

game_client_t *game_client_connect(char *address, char *port);

void game_client_disconnect(game_client_t *client);

int game_client_send(game_client_t *client, mess_client_server_t *client_message);

int game_client_recv(game_client_t *client, uint32_t *game_id, list_t *events);

int game_client_socket(game_client_t *client);

#endif //SIK_SCREEN_WORMS_GAME_CLIENT_H
