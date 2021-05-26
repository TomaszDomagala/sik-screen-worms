#include "game_client.h"

#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "err.h"
#include <errno.h>
#include "utils/list.h"
#include "utils/utils.h"
#include "messages.h"
#include <assert.h>

#define CLIENT_BUFFER_SIZE 1024

struct game_client_s {
	int sock_fd;
	int8_t buffer[CLIENT_BUFFER_SIZE];
};


game_client_t *game_client_connect(char *address, char *port) {
	game_client_t *client = malloc(sizeof(game_client_t));
	if (client == NULL)
		return NULL;

	struct addrinfo hints, *gui_info, *info;
	int status, sock_fd, tcp_no_delay_on = 1;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;

	if ((status = getaddrinfo(address, port, &hints, &gui_info)) != 0) {
		syserr("game_client_connect: getaddrinfo error: %s\n", gai_strerror(status));
	}

	for (info = gui_info; info != NULL; info = info->ai_next) {
		if ((sock_fd = socket(info->ai_family, info->ai_socktype, info->ai_protocol)) == -1) {
			perror("game_client_connect: socket");
			continue;
		}
		if (connect(sock_fd, info->ai_addr, info->ai_addrlen) == -1) {
			perror("game_client_connect: connect");
			close(sock_fd);
			continue;
		}
		break;
	}

	if (info == NULL) {
		fatal("game_client_connect: failed to connect\n");
	}

	char addr_str[INET6_ADDRSTRLEN];
	inet_ntop(info->ai_family, get_in_addr(info->ai_addr), addr_str, sizeof(addr_str));
	printf("game_client_connect: connected to %s", addr_str);

	freeaddrinfo(gui_info);

	client->sock_fd = sock_fd;
	return client;
}

void game_client_disconnect(game_client_t *client) {

}

/**
 * Serialize and send single client message to the connected server.
 * @param client
 * @param client_message
 * @return number of bytes sent or -1 in case of error or partial send.
 */
int game_client_send(game_client_t *client, mess_client_server_t *client_message) {
	int num_bytes = serialize_client_message(client->buffer, client_message);
	if (num_bytes == send(client->sock_fd, client->buffer, num_bytes, MSG_DONTWAIT)) {
		return num_bytes;
	}
	if (num_bytes == -1 && errno != EWOULDBLOCK && errno != EAGAIN) {
		perror("game_client_send: send");
	}
	return -1;
}

/**
 * Receive and deserialize single server message.
 * Set game_id field to game_id received in the message.
 * Add all events from the message to the events list.
 * Stop message deserialization as soon as first event failed to deserialize.
 * @param client
 * @param game_id
 * @param events
 * @return
 */
int game_client_recv(game_client_t *client, uint32_t *game_id, list_t *events) {
	assert(events != NULL);
	assert(list_size(events) == 0);

	int num_bytes, serialized, total_serialized = 0;

	num_bytes = recv(client->sock_fd, client->buffer, CLIENT_BUFFER_SIZE, MSG_DONTWAIT);
	if (num_bytes == -1) {
		if (errno != EWOULDBLOCK && errno != EAGAIN)
			perror("game_client_recv: recv");
		return -1;
	}
	if (num_bytes < sizeof(uint32_t)) {
		return -1;
	}

	int8_t *buffer = client->buffer;
	memcpy(game_id, buffer, sizeof(uint32_t));
	total_serialized += sizeof(uint32_t);
	*game_id = be32toh(*game_id);

	game_event_t event;
	while (total_serialized < num_bytes) {
		serialized = deserialize_game_event(buffer + total_serialized, num_bytes - total_serialized, &event);
		if (serialized == -1) {
			return 0;
		}
		total_serialized += serialized;
		list_add(events, &event);
	}
	return 0;
}

int game_client_socket(game_client_t *client) {
	return client->sock_fd;
}
