#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "server_args.h"
#include "err.h"
#include <stdbool.h>
#include <assert.h>
#include "clients.h"

#define MAX_EVENTS 32
#define BUFFER_SIZE 512

#define LA_NEW_CLIENT 0x1
#define LA_DATA 0x2
#define LA_TIMEOUT 0x4

char buffer[BUFFER_SIZE];

int main(int argc, char *argv[]) {

	server_args_t args;
	int parse_result = parse_server_args(argc, argv, &args);
	if (parse_result != 0) {
		fatal("Usage: [-p n] [-s n] [-t n] [-v n] [-w n] [-h n]");
	}

	int main_sock_fd, status;
	struct addrinfo hints, *server_info, *info;
	char port[6];
	int on = 1;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE;

	sprintf(port, "%d", args.server_port);

	if ((status = getaddrinfo(NULL, port, &hints, &server_info)) != 0) {
		syserr("server: getaddrinfo: %s\n", gai_strerror(status));
	}

	for (info = server_info; info != NULL; info = info->ai_next) {
		if ((main_sock_fd = socket(info->ai_family, info->ai_socktype, info->ai_protocol)) == -1) {
			perror("server: socket");
			continue;
		}
		if (setsockopt(main_sock_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) == -1) {
			perror("server: setsockopt");
			close(main_sock_fd);
			continue;
		}
		if (bind(main_sock_fd, info->ai_addr, info->ai_addrlen) == -1) {
			perror("server: bind");
			close(main_sock_fd);
			continue;
		}
		break;
	}
	if (info == NULL) {
		fatal("server: failed to bind\n");
	}

//	freeaddrinfo(server_info);

	clients_collection_t *clients = clients_create();
	if (clients == NULL) {
		fatal("server: clients");
	}

	struct epoll_event ev, events[MAX_EVENTS];

	int main_epoll_fd, data_epoll_fd, timeout_epoll_fd;

	if ((main_epoll_fd = epoll_create1(0)) == -1)
		syserr("server: epoll");
	if ((data_epoll_fd = epoll_create1(0)) == -1)
		syserr("server: epoll");
	if ((timeout_epoll_fd = epoll_create1(0)) == -1)
		syserr("server: epoll");

	// Wait for new clients.
	ev.events = EPOLLIN;
	ev.data.fd = main_sock_fd;
	if (epoll_ctl(main_epoll_fd, EPOLL_CTL_ADD, main_sock_fd, &ev) == -1) {
		syserr("server: epoll_ctl");
	}
	// Wait for data from connected clients.
	ev.events = EPOLLIN;
	ev.data.fd = data_epoll_fd;
	if (epoll_ctl(main_epoll_fd, EPOLL_CTL_ADD, data_epoll_fd, &ev) == -1) {
		syserr("server: epoll_ctl");
	}
	// Wait for timeouts;
	ev.events = EPOLLIN;
	ev.data.fd = timeout_epoll_fd;
	if (epoll_ctl(main_epoll_fd, EPOLL_CTL_ADD, timeout_epoll_fd, &ev) == -1) {
		syserr("server: epoll_ctl");
	}

	struct sockaddr_storage new_client_addr;

	socklen_t new_client_addr_len;
	int events_num, i, num_bytes;

	printf("listening at port %s\n", port);

	int loop_actions, action_types;
	for (;;) {
		// Wait for new clients, new data from clients or clients timeouts.
		action_types = epoll_wait(main_epoll_fd, events, MAX_EVENTS, -1);
		loop_actions = 0;
		printf("actions: %d\n", action_types);
		for (i = 0; i < action_types; ++i) {
			if (events[i].data.fd == main_sock_fd) {
				loop_actions |= LA_NEW_CLIENT;
			} else if (events[i].data.fd == data_epoll_fd) {
				loop_actions |= LA_DATA;
			} else if (events[i].data.fd == timeout_epoll_fd) {
				loop_actions |= LA_TIMEOUT;
			} else {
				assert(false);
			}
		}

		if (loop_actions & LA_TIMEOUT) {
			// TODO implement timeouts
		}
		if (loop_actions & LA_NEW_CLIENT) {
			memset(buffer, 0, BUFFER_SIZE);
			new_client_addr_len = sizeof(new_client_addr);

			num_bytes = recvfrom(main_sock_fd, buffer, BUFFER_SIZE, 0, (struct sockaddr *) &new_client_addr,
								 &new_client_addr_len);

			int new_client_sock = socket(new_client_addr.ss_family, SOCK_DGRAM, 0);
			if (new_client_sock == -1) {
				syserr("server: socket");
			}
			if (setsockopt(new_client_sock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) == -1) {
				syserr("server: setsockopt");
			}
			if (bind(new_client_sock, info->ai_addr, info->ai_addrlen) == -1) {
				syserr("server: bind");
			}
			if (connect(new_client_sock, (struct sockaddr *) &new_client_addr, new_client_addr_len) == -1) {
				syserr("server: connect");
			}


			client_t *new_client = clients_new_client(clients, 1, new_client_sock);
			assert(new_client != NULL); // TODO handle it better.

			ev.events = EPOLLIN;
			ev.data.ptr = new_client;

			if (epoll_ctl(data_epoll_fd, EPOLL_CTL_ADD, new_client->sock_fd, &ev) == -1) {
				syserr("server: epoll_ctl");
			}
			// TODO add to timeout epoll.

			printf("new client:\n");
			printf("read %d bytes:\n", num_bytes);
			printf("%s\n", buffer);
		}
		if (loop_actions & LA_DATA) {
			while ((events_num = epoll_wait(data_epoll_fd, events, MAX_EVENTS, 0)) > 0) {
				for (i = 0; i < events_num; ++i) {
					memset(buffer, 0, BUFFER_SIZE);
					new_client_addr_len = sizeof(new_client_addr);

					client_t *client = events[i].data.ptr;

					num_bytes = recv(client->sock_fd, buffer, BUFFER_SIZE, 0);
					printf("registered client:\n");
					printf("read %d bytes:\n", num_bytes);
					printf("%s\n", buffer);
				}

			}
			if (events_num == -1) {
				syserr("server: epoll_wait");
			}
		}

//		for (i = 0; i < events_num; ++i) {
//			memset(buffer, 0, BUFFER_SIZE); // TODO remove
//			if (events[i].data.ptr == NULL) { /* main socket, new client */
//
//				new_client_addr_len = sizeof(new_client_addr);
//				num_bytes = recvfrom(main_sock_fd, buffer, BUFFER_SIZE, 0, (struct sockaddr *) &new_client_addr,
//									 &new_client_addr_len);
//
//				int32_t new_sock_fd = socket(new_client_addr.ss_family, SOCK_DGRAM, 0);
//
//
//				printf("read %d bytes:\n", num_bytes);
//				printf("%s\n", buffer);
//
//
//			} else { /* data or timeout */
//
//			}
//		}
	}


	return 0;
}
