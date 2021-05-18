#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "server_args.h"
#include "err.h"

int main(int argc, char *argv[]) {

	server_args_t args;
	int parse_result = parse_server_args(argc, argv, &args);
	if (parse_result != 0) {
		fatal("Usage: [-p n] [-s n] [-t n] [-v n] [-w n] [-h n]");
	}

	int main_sock_fd, status;
	struct addrinfo hints, *server_info, *info;
	char port[6];

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

	freeaddrinfo(server_info);

	


	return 0;
}
