#include "gui_client.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "err.h"

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


int gui_client_connect(char *address, char *port, gui_client_t *client) {
	struct addrinfo hints, *gui_info, *info;
	int status, sock_fd;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((status = getaddrinfo(address, port, &hints, &gui_info)) != 0) {
		syserr("gui client: getaddrinfo error: %s\n", gai_strerror(status));
	}

	for (info = gui_info; info != NULL; info = info->ai_next) {
		if ((sock_fd = socket(info->ai_family, info->ai_socktype, info->ai_protocol)) == -1) {
			perror("gui client: socket");
			continue;
		}
		// TODO add TCP_NODELAY option.
		if (connect(sock_fd, info->ai_addr, info->ai_addrlen) == -1) {
			perror("gui client: connect");
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
	return 0;
}

int gui_client_disconnect(gui_client_t *client) {
	close(client->sock_fd);
	return 0;
}

