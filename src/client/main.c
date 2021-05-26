#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include <sys/epoll.h>
#include <time.h>
#include <sys/timerfd.h>
#include <string.h>
#include "client_args.h"
#include "err.h"
#include "gui_client.h"
#include "gui_messages.h"
#include "game_client.h"

#define MAX_EVENTS 32
#define BUFFER_SIZE 1024

#define PD_FORWARD 0
#define PD_RIGHT 1
#define PD_LEFT 2

int8_t buffer[BUFFER_SIZE];
struct epoll_event ev, epoll_events[MAX_EVENTS];

client_args_t args;

gui_client_t *gui_client = NULL;
game_client_t *game_client = NULL;

int gui_sock_fd, game_sock_fd, timer_fd, epoll_fd;

uint32_t session_id;
mess_client_server_t client_message;

uint32_t next_expected_event_no = 0;
uint32_t current_game_id = 0;
int8_t **players_names;
uint8_t players_num;

uint32_t width, height;
uint8_t turn_direction = 0;
bool left_key_down = false, right_key_down = false;

/**
 * Returns if result is success.
 * Prints error message and exits on error.
 */
void check_arguments(cp_res_t result) {
	if (result == cpr_Success)
		return;
	fprintf(stderr, "Usage: ./screen-worms-client game_server [-n player_name] [-p n] [-i gui_server] [-r n]\n");
	switch (result) {
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

void init_game_client() {
	char port[6];
	sprintf(port, "%d", args.server_port);
	if ((game_client = game_client_connect(args.server_address, port)) == NULL) {
		syserr("init_game_client: game_client_connect");
	}
	game_sock_fd = game_client_socket(game_client);
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

	ev.events = EPOLLIN;
	ev.data.fd = game_sock_fd;
	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, game_sock_fd, &ev) == -1) {
		syserr("init_epoll: epoll_ctl");
	}

}

void init_client_session() {
	session_id = time(NULL);
	memset(&client_message, 0, sizeof(client_message));

	client_message.session_id = session_id;
	if (args.player_name != NULL && strlen(args.player_name) > 0)
		memcpy(client_message.player_name, args.player_name, strlen(args.player_name));

}

void handle_tick() {
	uint64_t ticks;
	if (read(timer_fd, &ticks, sizeof(ticks)) == -1)
		syserr("handle_timer: read");
	if (ticks > 1)
		fprintf(stderr, "client is lagging: %lu ticks skipped\n", ticks - 1);

	client_message.turn_direction = turn_direction;
	client_message.next_expected_event_no = next_expected_event_no;

	game_client_send(game_client,&client_message);
}

void handle_gui_message() {
	list_t *gui_messages = list_create(sizeof(gui_message_t));
	gui_client_recv_event(gui_client, gui_messages);

	for (list_node_t *node = list_head(gui_messages); node != NULL; node = list_next(node)) {
		gui_message_t *message = list_element(node);
		switch (message->action) {
			case GA_LEFT_KEY_DOWN:
				left_key_down = true;
				turn_direction = PD_LEFT;
				break;
			case GA_RIGHT_KEY_DOWN:
				right_key_down = true;
				turn_direction = PD_RIGHT;
				break;
			case GA_LEFT_KEY_UP:
				left_key_down = false;
				turn_direction = right_key_down ? PD_RIGHT : PD_FORWARD;
				break;
			case GA_RIGHT_KEY_UP:
				right_key_down = false;
				turn_direction = left_key_down ? PD_LEFT : PD_FORWARD;
				break;
		}
	}
	list_remove_all(gui_messages);
	list_free(gui_messages);
}

void player_names_free(int8_t **names, uint8_t num) {
	for (int i = 0; i < num; ++i) {
		free(names[i]);
	}
	free(names);
}

void on_game_restart(uint32_t game_id) {
	current_game_id = game_id;
	next_expected_event_no = 0;
	player_names_free(players_names, players_num);
	players_num = 0;
	players_names = NULL;
}

void on_event_new_game(game_event_t *event) {
	ge_data_new_game_t data = event->data.new_game;

	if (event->event_no != 0) {
		fatal("illegal event: NEW_GAME with event_no %u", event->event_no);
	}
	players_num = data.players_num;
	players_names = data.players_names;
}

void on_event_pixel(game_event_t *event) {
	ge_data_pixel_t data = event->data.pixel;

	if (data.player_number >= players_num) {
		fatal("illegal event: PIXEL player_number=%u out of range", data.player_number);
	}
	if (data.x > width) {
		fatal("illegal event: PIXEL x=%u out of range", data.x);
	}
	if (data.y > height) {
		fatal("illegal event: PIXEL y=%u out of range", data.y);
	}
}

void on_event_player_eliminated(game_event_t *event) {
	ge_data_player_eliminated_t data = event->data.player_eliminated;

	if (data.player_number >= players_num) {
		fatal("illegal event: PLAYER_ELIMINATED player_number=%u out of range", data.player_number);
	}
}

void handle_server_message() {
	uint32_t message_game_id;
	list_t *message_events = list_create(sizeof(game_event_t));

	if (game_client_recv(game_client, &message_game_id, message_events) == -1) {
		return;
	}
	if (list_size(message_events) == 0) {
		list_free(message_events);
		return;
	}

	if (current_game_id != message_game_id) {
		on_game_restart(message_game_id);
	}

	for (list_node_t *node = list_head(message_events); node != NULL; node = list_next(node)) {
		game_event_t *event = list_element(node);
		if (event->event_no != next_expected_event_no) {
			// Not the event that we want.
			break;
		}
		switch (event->type) {
			case GE_NEW_GAME:
				on_event_new_game(event);
				break;
			case GE_PIXEL:
				on_event_pixel(event);
				break;
			case GE_PLAYER_ELIMINATED: {
				on_event_player_eliminated(event);
				break;
			}
			case GE_GAME_OVER:
				break;
			default: {
				// Ignore correct event with unknown type.
				next_expected_event_no++;
				continue;
			}
		}

		if (gui_client_send_event(gui_client, event, players_names) == -1) {
			break;
		}
		next_expected_event_no++;
	}

	list_remove_all(message_events);
	list_free(message_events);
}

int main(int argc, char *argv[]) {
	cp_res_t res = parse_client_args(argc, argv, &args);
	check_arguments(res);

	init_gui_client();
	init_game_client();
	init_timer();
	init_epoll();
	init_client_session();

	printf("client session id: %u\n", session_id);
	if (args.player_name[0] != 0) {
		printf("player name: %s\n", args.player_name);
	}

	int actions;
	for (;;) {
		actions = epoll_wait(epoll_fd, epoll_events, MAX_EVENTS, -1);
		for (int i = 0; i < actions; ++i) {

			int event_fd = epoll_events[i].data.fd;

			if (event_fd == timer_fd) {
				handle_tick();
			} else if (event_fd == gui_sock_fd) {
				// TODO add disconnect handler.
				if (epoll_events[i].events & EPOLLHUP) {
					fatal("connection with gui lost");
				}
				handle_gui_message();
			} else if (event_fd == game_sock_fd) {
				handle_server_message();
			}
		}
	}
}
