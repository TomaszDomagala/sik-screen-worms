#include "clients.h"
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>


#define MAX_CLIENTS_NUM 32

struct clients_collection_s {
	client_t arr[MAX_CLIENTS_NUM];
	uint32_t num;
};

clients_collection_t *clients_create() {
	clients_collection_t *clients = malloc(sizeof(clients_collection_t));
	if (clients == NULL) {
		return NULL;
	}
	for (int i = 0; i < MAX_CLIENTS_NUM; ++i) {
		clients->arr[i].sock_fd = -1;
		clients->arr[i].timer_fd = -1;
		clients->arr[i].session_id = 0;

	}
	clients->num = 0;
	return clients;
}

client_t *clients_new_client(clients_collection_t *clients, uint64_t session_id, int32_t sock_fd) {
	assert(clients != NULL);
	if (clients->num == MAX_CLIENTS_NUM) {
		return NULL;
	}
	int i;
	for (i = 0; i < MAX_CLIENTS_NUM; ++i) {
		if (clients->arr[i].sock_fd == -1) {
			clients->arr[i].session_id = session_id;
			clients->arr[i].sock_fd = sock_fd;
			// TODO add timerfd initialization.
			break;
		}
	}
	assert(i < MAX_CLIENTS_NUM);
	clients->num++;
	return &clients->arr[i];
}