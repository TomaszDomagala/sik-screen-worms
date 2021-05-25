#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include <sys/epoll.h>
#include <sys/timerfd.h>
#include <string.h>
#include "client_args.h"
#include "err.h"
#include "gui_client.h"
#include "gui_messages.h"

#define MAX_EVENTS 32
#define BUFFER_SIZE 1024

int8_t buffer[BUFFER_SIZE];
struct epoll_event ev, epoll_events[MAX_EVENTS];

client_args_t args;

gui_client_t *gui_client = NULL;

int gui_sock_fd, game_sock_fd, timer_fd, epoll_fd;

/**
 * Returns if result is success.
 * Prints error message and exits on error.
 */
void check_arguments(cp_res_t result) {
	if (result == cpr_Success)
		return;
	fprintf(stderr, "Usage: ./screen-worms-client game_server [-n player_name] [-p n] [-i gui_server] [-r n]\n");
	switch (result) {
		case cpr_MissingPlayerName:
			fatal("missing [ -n player_name ] argument\n");
		case cpr_MissingServerAddress:
			fatal("missing game_server argument\n");
		case cpr_InvalidPlayerName:
			fatal("invalid [ -n player_name ] argument\n");
		case cpr_InvalidArgument:
			fatal("invalid argument\n");
		default:
			assert(false);
	}
}

void init_gui_client() {
	char port[6];
	sprintf(port, "%d", args.gui_port);
	if ((gui_client = gui_client_connect(args.gui_address, port)) == NULL) {
		syserr("init_gui_client: gui_client_connect");
	}
	gui_sock_fd = gui_client_socket(gui_client);
}

void init_timer() {
	struct itimerspec spec;
	memset(&spec, 0, sizeof(spec));
	spec.it_value.tv_nsec = 1;
	spec.it_interval.tv_nsec = 30000000; // 30 ms.

	if ((timer_fd = timerfd_create(CLOCK_BOOTTIME, 0)) == -1) {
		syserr("init_timer: timerfd_create");
	}

	timerfd_settime(timer_fd, 0, &spec, NULL);
}

void init_epoll() {
	if ((epoll_fd = epoll_create1(0)) == -1)
		syserr("init_epoll: epoll_create1");

	ev.events = EPOLLIN;
	ev.data.fd = timer_fd;
	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, timer_fd, &ev) == -1) {
		syserr("init_epoll: epoll_ctl");
	}

	ev.events = EPOLLIN;
	ev.data.fd = gui_sock_fd;
	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, gui_sock_fd, &ev) == -1) {
		syserr("init_epoll: epoll_ctl");
	}

//	ev.events = EPOLLIN;
//	ev.data.fd = game_sock_fd;
//	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, game_sock_fd, &ev) == -1) {
//		syserr("init_epoll: epoll_ctl");
//	}

}

void handle_timer() {
	printf("timer\n");
}

void handle_gui_message() {
	gui_message_t message;
	gui_client_recv_event(gui_client, &message);
}

void handle_server_message() {

}

int main(int argc, char *argv[]) {
	cp_res_t res = parse_client_args(argc, argv, &args);
	check_arguments(res);

	init_gui_client();
	// init_game_client();
	init_timer();
	init_epoll();

	int actions;
	for (;;) {
		actions = epoll_wait(epoll_fd, epoll_events, MAX_EVENTS, -1);

		for (int i = 0; i < actions; ++i) {
			int event_fd = epoll_events[i].data.fd;

			if (event_fd == timer_fd) {
				handle_timer();
			} else if (event_fd == gui_sock_fd) {
				handle_gui_message();
			} else if (event_fd == game_sock_fd) {
				handle_server_message();
			}
		}

	}
}
