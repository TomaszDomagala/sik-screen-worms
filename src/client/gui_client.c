#include "gui_client.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "err.h"
#include <errno.h>
#include <assert.h>
#include "utils/utils.h"
#include "gui_messages.h"

#define GUI_BUFFER_SIZE 2048

struct gui_client_s {
	int sock_fd;
	int8_t buffer[GUI_BUFFER_SIZE];
};


gui_client_t *gui_client_connect(char *address, char *port) {
	gui_client_t *client = malloc(sizeof(gui_client_t));
	if (client == NULL)
		return NULL;

	struct addrinfo hints, *gui_info, *info;
	int status, sock_fd, tcp_no_delay_on = 1;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((status = getaddrinfo(address, port, &hints, &gui_info)) != 0) {
		syserr("gui_client_connect: getaddrinfo error: %s\n", gai_strerror(status));
	}

	for (info = gui_info; info != NULL; info = info->ai_next) {
		if ((sock_fd = socket(info->ai_family, info->ai_socktype, info->ai_protocol)) == -1) {
			perror("gui_client_connect: socket");
			continue;
		}
		// TODO add TCP_NODELAY option.
		if (setsockopt(sock_fd, IPPROTO_TCP, TCP_NODELAY, &tcp_no_delay_on, sizeof(tcp_no_delay_on)) == -1) {
			perror("gui_client_connect: setsockopt");
			continue;
		}
		if (connect(sock_fd, info->ai_addr, info->ai_addrlen) == -1) {
			perror("gui_client_connect: connect");
			close(sock_fd);
			continue;
		}
		break;
	}
	if (info == NULL) {
		fatal("gui client: failed to connect\n");
	}

	char addr_str[INET6_ADDRSTRLEN];
	inet_ntop(info->ai_family, get_in_addr(info->ai_addr), addr_str, sizeof(addr_str));
	printf("gui client: connected to %s", addr_str);

	freeaddrinfo(gui_info);

	client->sock_fd = sock_fd;
	return client;
}

void gui_client_disconnect(gui_client_t *client) {
	close(client->sock_fd);
}

int gui_client_socket(gui_client_t *client) {
	return client->sock_fd;
}

int gui_client_send_event(gui_client_t *client, game_event_t *event, int8_t **names) {
	int size = serialize_client_gui_message(client->buffer, event, names);
	int total_sent = 0, sent;

	while (total_sent < size) {
		sent = send(client->sock_fd, client->buffer + total_sent, size - total_sent, 0);
		if (sent == -1)
			break;
		total_sent += sent;
	}
	return sent == -1 ? -1 : 0;
}

// TODO handle errors.
// TODO non blocking sockets
int gui_client_recv_event(gui_client_t *client, list_t *gui_messages) {
	assert(gui_messages != NULL);
	assert(list_size(gui_messages) == 0);

	int total_rec = 0, space_left = GUI_BUFFER_SIZE, rec;
	memset(client->buffer, 0, GUI_BUFFER_SIZE);

	while ((rec = recv(client->sock_fd, client->buffer + total_rec, space_left, MSG_DONTWAIT)) > 0) {
		printf("rec %d\n", rec);
		total_rec += rec;
		space_left -= rec;
	}
	if (rec == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
		printf("would block\n");
	}

	int8_t *buffer = client->buffer;
	int total_des = 0, des;
	gui_message_t message;

	while (total_des < total_rec) {
		des = deserialize_gui_client_message(buffer + total_des, total_rec - total_des, &message);
		total_des += des;
		list_add(gui_messages, &message);
	}

	return 0;
}


