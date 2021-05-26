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
#include "messages.h"

struct game_client_s {
	int sock_fd;
};

// TODO export this function to commons
void *get_in_addr(struct sockaddr *addr) {
	switch (addr->sa_family) {
		case AF_INET:
			return &(((struct sockaddr_in *) addr)->sin_addr);
		case AF_INET6:
			return &(((struct sockaddr_in6 *) addr)->sin6_addr);
	}
	fatal("get_in_addr: invalid sa_family");
	return NULL;
}

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

int game_client_send(game_client_t *client, mess_client_server_t *client_message) {

}

int game_client_recv(game_client_t *client, game_event_t *event) {

}
