#include "clients.h"
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/timerfd.h>
#include <unistd.h>
#include "config.h"
#include "utils/list.h"
#include "err.h"
#include <string.h>


#define MAX_PLAYERS_NUM 25

struct clients_collection_s {
	list_t *clients_list;
	int players_number;
};

clients_collection_t *clients_create() {
	clients_collection_t *clients = malloc(sizeof(clients_collection_t));
	if (clients == NULL) {
		return NULL;
	}
	clients->clients_list = list_create(sizeof(client_t));
	clients->players_number = 0;
	if (clients->clients_list == NULL) {
		free(clients);
		return NULL;
	}
	return clients;
}

//list_node_t *find_client_by_sock_fd(clients_collection_t *clients, int32_t sock_fd) {
//	for (list_node_t *node = list_head(clients->clients_list); node != NULL; node = list_next(node)) {
//		client_t *client = list_element(node);
//		if (client->sock_fd == sock_fd)
//			return node;
//	}
//	return NULL;
//}

list_node_t *find_by_session_id(clients_collection_t *clients, uint64_t session_id) {
	for (list_node_t *node = list_head(clients->clients_list); node != NULL; node = list_next(node)) {
		client_t *client = list_element(node);
		if (client->session_id == session_id)
			return node;
	}
	return NULL;
}

client_t *clients_new_client(clients_collection_t *clients, uint64_t session_id, int32_t sock_fd, int8_t *name) {
	assert(clients != NULL);

	if (name[0] != 0 && clients->players_number == MAX_PLAYERS_NUM)
		return NULL; // Too many players.

	client_t client;
	client.session_id = session_id;
	client.sock_fd = sock_fd;
	if ((client.timer_fd = timerfd_create(CLOCK_BOOTTIME, 0)) == -1) {
		syserr("clients_new_client: timerfd_create");
	}
	strcpy(client.player_name, name);

	list_node_t *node = list_add(clients->clients_list, &client);
	if (node == NULL)
		return NULL;
	if (name[0] != 0)
		clients->players_number++;

	return list_element(node);
}

void clients_delete_client(clients_collection_t *clients, uint64_t session_id) {
	assert(clients != NULL);

	list_node_t *node = find_by_session_id(clients, session_id);
	assert(node != NULL);
	client_t *client = list_element(node);
	if (client->player_name[0] != 0)
		clients->players_number--;

	close(client->sock_fd);
	close(client->timer_fd);
	list_remove(clients->clients_list, node);
}

client_t *clients_find_client_by_session_id(clients_collection_t *clients, uint64_t session_id) {
	list_node_t *node = find_by_session_id(clients, session_id);
	return node != NULL ? list_element(node) : NULL;
}