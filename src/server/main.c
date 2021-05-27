#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/timerfd.h>
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
#include "messages.h"
#include "game.h"
#include "rand.h"
#include <errno.h>
#include "events_storage.h"

#if defined(__linux__)

#  include <endian.h>

#elif defined(__FreeBSD__) || defined(__NetBSD__)
#  include <sys/endian.h>
#elif defined(__OpenBSD__)
#  include <sys/types.h>
#  define be16toh(x) betoh16(x)
#  define be32toh(x) betoh32(x)
#  define be64toh(x) betoh64(x)
#endif


#define MAX_EVENTS 4
#define BUFFER_SIZE 1024
#define MAX_UDP_SIZE 550

// loop actions
#define LA_NEW_CLIENT 0x1
#define LA_DATA 0x2
#define LA_TIMEOUT 0x4
#define LA_ROUND 0x8

int8_t buffer[BUFFER_SIZE];
struct epoll_event ev, epoll_events[MAX_EVENTS];

clients_collection_t *clients = NULL;
event_storage_t *event_storage = NULL;

int server_sock_fd, round_fd;
int main_epoll_fd, data_epoll_fd, timeout_epoll_fd;

struct addrinfo *gai_all_server_info, *s_info;

struct sockaddr_storage new_client_addr;
socklen_t new_client_addr_len;

struct itimerspec timer;

server_args_t args;

game_t *game;
bool is_game_in_progress = false;


void init_server() {
	int gai_result, on = 1;
	struct addrinfo hints;

	char port[6];
	sprintf(port, "%d", args.server_port);

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET6;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE;

	if ((gai_result = getaddrinfo(NULL, port, &hints, &gai_all_server_info)) != 0) {
		syserr("init_server: getaddrinfo: %s\n", gai_strerror(gai_result));
	}

	for (s_info = gai_all_server_info; s_info != NULL; s_info = s_info->ai_next) {
		if ((server_sock_fd = socket(s_info->ai_family, s_info->ai_socktype, s_info->ai_protocol)) == -1) {
			perror("init_server: socket");
			continue;
		}
		if (setsockopt(server_sock_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) == -1) {
			perror("init_server: setsockopt");
			close(server_sock_fd);
			continue;
		}
		if (bind(server_sock_fd, s_info->ai_addr, s_info->ai_addrlen) == -1) {
			perror("init_server: bind");
			close(server_sock_fd);
			continue;
		}
		break;
	}
	if (s_info == NULL) {
		fatal("init_server: failed to bind\n");
	}
}

void init_clients() {
	if ((clients = clients_create()) == NULL) {
		fatal("server: clients_create");
	}
}

void init_event_storage() {
	if ((event_storage = event_storage_create()) == NULL) {
		fatal("server: event_storage_create");
	}
}

void init_round_timer() {
	struct itimerspec spec;
	memset(&spec, 0, sizeof(spec));

	spec.it_value.tv_nsec = 1;
	if (args.rounds_per_sec == 1) {
		spec.it_interval.tv_sec = 1;
	} else {
		spec.it_interval.tv_nsec = (1000000000 / args.rounds_per_sec);
	}

	round_fd = timerfd_create(CLOCK_BOOTTIME, 0);

	timerfd_settime(round_fd, 0, &spec, NULL);
}

void init_epoll() {
	if ((main_epoll_fd = epoll_create1(0)) == -1)
		syserr("init_epoll: epoll_create1");
	if ((data_epoll_fd = epoll_create1(0)) == -1)
		syserr("init_epoll: epoll_create1");
	if ((timeout_epoll_fd = epoll_create1(0)) == -1)
		syserr("init_epoll: epoll_create1");

	// Wait for game round tick.
	ev.events = EPOLLIN;
	ev.data.fd = round_fd;
	if (epoll_ctl(main_epoll_fd, EPOLL_CTL_ADD, round_fd, &ev) == -1) {
		syserr("init_epoll: epoll_ctl");
	}

	// Wait for new clients.
	ev.events = EPOLLIN;
	ev.data.fd = server_sock_fd;
	if (epoll_ctl(main_epoll_fd, EPOLL_CTL_ADD, server_sock_fd, &ev) == -1) {
		syserr("init_epoll: epoll_ctl");
	}
	// Wait for data from connected clients.
	ev.events = EPOLLIN;
	ev.data.fd = data_epoll_fd;
	if (epoll_ctl(main_epoll_fd, EPOLL_CTL_ADD, data_epoll_fd, &ev) == -1) {
		syserr("init_epoll: epoll_ctl");
	}
	// Wait for timeouts;
	ev.events = EPOLLIN;
	ev.data.fd = timeout_epoll_fd;
	if (epoll_ctl(main_epoll_fd, EPOLL_CTL_ADD, timeout_epoll_fd, &ev) == -1) {
		syserr("init_epoll: epoll_ctl");
	}
}

void init_game() {
	rand_set(args.rng_seed);
	game = game_create(args.board_width, args.board_height, args.turning_speed);
}


void client_timer_restart(client_t *client) {
	memset(&timer, 0, sizeof(timer));
	timer.it_value.tv_sec = 2;
	timerfd_settime(client->timer_fd, 0, &timer, NULL);
}

void send_events(client_t *client, size_t event_no) {
	ssize_t last = event_storage_last(event_storage);

	if (last == -1 || event_no > last)
		return;

	uint32_t be_game_id = htobe32(game_get_id(game));
	memcpy(buffer, &be_game_id, sizeof(be_game_id));

	size_t data_size = sizeof(uint32_t);
	serialized_event_t *event;

	event = event_storage_get(event_storage, event_no);
	size_t data_start = event->offset;
	data_size += event->len;

	for (size_t i = event_no + 1; i <= last; ++i) {
		event = event_storage_get(event_storage, i);

		if (data_size + event->len > MAX_UDP_SIZE) {
			// Copy data to the buffer and send.
			int8_t *data = event_storage_get_data(event_storage, data_start);
			memcpy(buffer + sizeof(uint32_t), data, data_size - sizeof(uint32_t));
			if (send(client->sock_fd, buffer, data_size, MSG_DONTWAIT) != data_size) {
				// Would block, error or partial write. In every case wait for next client request.
				return;
			}
			data_start = event->offset;
			data_size = sizeof(uint32_t);
		}
		data_size += event->len;
	}

	if (data_size > sizeof(uint32_t)) {
		// If something is in the buffer, send it.
		int8_t *data = event_storage_get_data(event_storage, data_start);
		memcpy(buffer + sizeof(uint32_t), data, data_size - sizeof(uint32_t));
		send(client->sock_fd, buffer, data_size, MSG_DONTWAIT);
	}
}

void handle_client_request(client_t *client, mess_client_server_t *m_client) {
	send_events(client, m_client->next_expected_event_no);
}

void handle_timeouts() {
	int events_num;

	while ((events_num = epoll_wait(timeout_epoll_fd, epoll_events, MAX_EVENTS, 0)) > 0) {
		for (int i = 0; i < events_num; ++i) {
			client_t *client = epoll_events[i].data.ptr;

			printf("disconnecting client %d\n", client->sock_fd);
			if (epoll_ctl(data_epoll_fd, EPOLL_CTL_DEL, client->sock_fd, NULL) == -1)
				syserr("server: epoll_ctl");
			if (epoll_ctl(timeout_epoll_fd, EPOLL_CTL_DEL, client->timer_fd, NULL) == -1)
				syserr("server: epoll_ctl");

			game_remove_player(game, client->session_id);
			clients_delete_client(clients, client->session_id);
		}

	}
	if (events_num == -1) {
		syserr("server: epoll_wait");
	}
}

void handle_new_client() {
	int num_bytes, so_reuseaddr_on = 1;

	memset(buffer, 0, BUFFER_SIZE);
	new_client_addr_len = sizeof(new_client_addr);

	num_bytes = recvfrom(server_sock_fd, buffer, BUFFER_SIZE, 0, (struct sockaddr *) &new_client_addr,
						 &new_client_addr_len);

	mess_client_server_t m_client;

	if (deserialize_client_message(buffer, num_bytes, &m_client) == -1) {
		// Invalid message.
		return;
	}
	if (m_client.player_name[0] != 0 && clients_find_by_name(clients, m_client.player_name) != NULL) {
		// Ignore packets from different socket but with name of existing client.
		return;
	}

	// Connect client.
	int new_client_sock = socket(new_client_addr.ss_family, SOCK_DGRAM, 0);
	if (new_client_sock == -1) {
		syserr("handle_new_client: socket");
	}
	if (setsockopt(new_client_sock, SOL_SOCKET, SO_REUSEADDR, &so_reuseaddr_on, sizeof(so_reuseaddr_on)) == -1) {
		syserr("handle_new_client: setsockopt");
	}
	if (bind(new_client_sock, s_info->ai_addr, s_info->ai_addrlen) == -1) {
		syserr("handle_new_client: bind");
	}
	if (connect(new_client_sock, (struct sockaddr *) &new_client_addr, new_client_addr_len) == -1) {
		syserr("handle_new_client: connect");
	}


	client_t *new_client = clients_new_client(clients, m_client.session_id, new_client_sock, m_client.player_name);
	if (new_client == NULL) {
		close(new_client_sock);
		perror("handle_new_client: cannot create new client\n");
		return;
	}
	if (!game_add_player(game, new_client->session_id, new_client->player_name)) {
		clients_delete_client(clients, new_client->session_id);
		perror("handle_new_client: cannot create new player\n");
		return;
	}

	ev.events = EPOLLIN;
	ev.data.ptr = new_client;

	if (epoll_ctl(data_epoll_fd, EPOLL_CTL_ADD, new_client->sock_fd, &ev) == -1) {
		syserr("handle_new_client: epoll_ctl");
	}
	if (epoll_ctl(timeout_epoll_fd, EPOLL_CTL_ADD, new_client->timer_fd, &ev) == -1) {
		syserr("handle_new_client: epoll_ctl");
	}

	printf("new client sock=%d session_id=%lu\n", new_client->sock_fd, new_client->session_id);

	handle_client_request(new_client, &m_client);

	client_timer_restart(new_client);
}


void handle_client_message(client_t *client) {
	client_timer_restart(client);

	memset(buffer, 0, BUFFER_SIZE);
	int num_bytes, r;
	while ((r = recv(client->sock_fd, buffer, BUFFER_SIZE, MSG_DONTWAIT)) > 0) {
		num_bytes = r;
	}
	if (r == -1 && errno != EWOULDBLOCK && errno != EAGAIN) {
		perror("handle_client_message: recv");
		return;
	}

	mess_client_server_t m_client;
	if (deserialize_client_message(buffer, num_bytes, &m_client) == -1) {
		// Ignore invalid message.
		return;
	}
	if (m_client.session_id < client->session_id) {
		// Ignore message.
		return;
	}
	if (m_client.session_id < client->session_id) {
		// Reconnect client.
		game_remove_player(game, client->session_id);
		game_add_player(game, m_client.session_id, client->player_name);

		client->session_id = m_client.session_id;
		return;
	}

	game_set_turn_direction(game, client->session_id, m_client.turn_direction);
	handle_client_request(client, &m_client);
}

void handle_clients() {
	int events_num;

	while ((events_num = epoll_wait(data_epoll_fd, epoll_events, MAX_EVENTS, 0)) > 0) {
		for (int i = 0; i < events_num; ++i) {
			client_t *client = epoll_events[i].data.ptr;
			assert(client != NULL);
			handle_client_message(client);
		}
	}
	if (events_num == -1) {
		syserr("handle_clients: epoll_wait");
	}
}

void handle_round() {
	uint64_t rounds;
	if (read(round_fd, &rounds, sizeof(uint64_t)) == -1)
		syserr("handle_round: read");
	if (rounds > 1)
		fprintf(stderr, "server is lagging: %lu rounds skipped\n", rounds - 1);

	list_t *game_events = game_tick(game);
	if (game_events == NULL)
		return;

	if (is_game_in_progress == false && game_in_progress(game) == true) {
		// Game started.
		printf("Game start id=%u\n", game_get_id(game));
		is_game_in_progress = true;
		event_storage_free(event_storage);
		init_event_storage();
	} else if (is_game_in_progress == true && game_in_progress(game) == false) {
		// Game ended.
		printf("Game end id=%u\n", game_get_id(game));
		is_game_in_progress = false;
	}


	for (list_node_t *node = list_head(game_events); node != NULL; node = list_next(node)) {
		game_event_t *g_event = list_element(node);
		event_storage_push(event_storage, g_event);
	}

	if (list_size(game_events) > 0) {
		// Send new events to evert client.
		list_t *clients_list = clients_get_all(clients);
		game_event_t *first_new_event = list_element(list_head(game_events));

		for (list_node_t *node = list_head(clients_list); node != NULL; node = list_next(node)) {
			client_t *client = list_element(node);
			send_events(client, first_new_event->event_no);
		}
	}

	list_remove_all(game_events);
	list_free(game_events);
}

int main(int argc, char *argv[]) {

	int parse_result = parse_server_args(argc, argv, &args);
	if (parse_result != 0) {
		fatal("Usage: [-p n] [-s n] [-t n] [-v n] [-w n] [-h n]");
	}

	init_server();
	init_clients();
	init_event_storage();
	init_round_timer();
	init_game();
	init_epoll();

	printf("listening on %d\n", args.server_port);

	int loop_actions, action_types, i;
	for (;;) {
		// Wait for new clients, new data from clients or clients timeouts.
		action_types = epoll_wait(main_epoll_fd, epoll_events, MAX_EVENTS, -1);
		loop_actions = 0;

		for (i = 0; i < action_types; ++i) {
			if (epoll_events[i].data.fd == server_sock_fd) {
				loop_actions |= LA_NEW_CLIENT;
			} else if (epoll_events[i].data.fd == data_epoll_fd) {
				loop_actions |= LA_DATA;
			} else if (epoll_events[i].data.fd == timeout_epoll_fd) {
				loop_actions |= LA_TIMEOUT;
			} else if (epoll_events[i].data.fd == round_fd) {
				loop_actions |= LA_ROUND;
			} else {
				assert(false);
			}
		}

		if (loop_actions & LA_ROUND) {
			handle_round();
		}
		if (loop_actions & LA_TIMEOUT) {
			handle_timeouts();
		}
		if (loop_actions & LA_NEW_CLIENT) {
			handle_new_client();
		}
		if (loop_actions & LA_DATA) {
			handle_clients();
		}
	}

	return 0;
}
