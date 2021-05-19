#ifndef SIK_SCREEN_WORMS_CLIENTS_H
#define SIK_SCREEN_WORMS_CLIENTS_H

#include <stdint.h>

struct clients_collection_s;

typedef struct clients_collection_s clients_collection_t;

typedef struct {
	uint64_t session_id;
	int32_t sock_fd;
	int32_t timer_fd;
} client_t;

clients_collection_t *clients_create();

void clients_free(clients_collection_t *clients);

client_t *clients_new_client(clients_collection_t *clients, uint64_t session_id, int32_t sock_fd);

void clients_delete_client(clients_collection_t *clients, int32_t sock_fd);

#endif //SIK_SCREEN_WORMS_CLIENTS_H
