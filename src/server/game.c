#include "game.h"

#include <stdbool.h>
#include <stdint.h>
#include "config.h"
#include "rand.h"
#include "utils/list.h"
#include "game_board.h"
#include <assert.h>
#include "string.h"
#include "messages.h"
#include <math.h>
#include <stdio.h>

#define MAX_PLAYERS_NUM SERVER_MAX_CAPACITY


struct game_s {
	uint8_t state;
	uint32_t game_id;
	size_t width;
	size_t height;
	uint32_t next_event_no;

	list_t *players;
	list_t *waiting;
	list_t *observers;

	game_board_t *board;

	uint32_t players_alive;
	uint32_t players_ready;

	int8_t **players_names;
};

game_t *game_create(size_t width, size_t height) {
	game_t *game = malloc(sizeof(game_t));
	if (game == NULL)
		return NULL;

	game->state = GS_WAITING;
	game->game_id = rand_get();

	game->players = list_create(sizeof(player_t));
	game->waiting = list_create(sizeof(player_t));
	game->observers = list_create(sizeof(player_t));
	game->board = game_board_create(width, height);

	game->width = width;
	game->height = height;
	game->next_event_no = 0;

	game->players_alive = 0;
	game->players_ready = 0;
	game->players_names = NULL;

	if (!game->players || !game->waiting || !game->observers || !game->board) {
		list_free(game->players);
		list_free(game->waiting);
		list_free(game->observers);
		game_board_free(game->board);
		free(game);
		return NULL;
	}
	return game;
}

uint8_t game_state(game_t *game) {
	return game->state;
}

player_t player_create(uint64_t session_id, int8_t *player_name) {
	player_t player;
	player.session_id = session_id;
	player.status |= PS_WAITING;
	player.turn_direction = 0;
	player.x_pos = 0;
	player.y_pos = 0;
	memcpy(player.player_name, player_name, 21);
	return player;
}

list_node_t *find_player(list_t *players, uint64_t session_id) {
	for (list_node_t *node = list_head(players); node != NULL; node = list_next(node)) {
		player_t *player = list_element(node);
		if (player->session_id == session_id)
			return node;
	}

	return NULL;
}

bool add_observer(game_t *game, uint64_t session_id, int8_t *player_name) {
	if (find_player(game->observers, session_id) != NULL)
		return true;
	player_t observer = player_create(session_id, player_name);
	return list_add(game->observers, &observer) != NULL;
}

bool add_player(game_t *game, uint64_t session_id, int8_t *player_name) {
	if (find_player(game->players, session_id) != NULL)
		return true; // Player is already in game.
	if (find_player(game->waiting, session_id) != NULL)
		return true; // Player is already waiting.

	player_t player = player_create(session_id, player_name);

	return list_add(game->waiting, &player) != NULL;
}

bool game_add_player(game_t *game, uint64_t session_id, int8_t *player_name) {
	printf("game_add_player: name=%s\n", player_name);
	if (player_name[0] == 0) {
		return add_observer(game, session_id, player_name);
	}
	return add_player(game, session_id, player_name);
}

bool game_remove_player(game_t *game, uint64_t session_id) {
	list_node_t *player_node = NULL;

	if ((player_node = find_player(game->observers, session_id)) != NULL) {
		list_remove(game->observers, player_node);
		return true;
	}
	if ((player_node = find_player(game->waiting, session_id)) != NULL) {
		list_remove(game->waiting, player_node);
		return true;
	}
	if ((player_node = find_player(game->players, session_id)) != NULL) {
		player_t *player = list_element(player_node);
		player->status |= PS_DISCONNECTED;
	}
	return false;
}

bool game_set_turn_direction(game_t *game, uint64_t session_id, uint8_t turn_direction) {
	list_node_t *player_node;
	player_t *player;

	switch (game->state) {
		case GS_WAITING: {
			player_node = find_player(game->waiting, session_id);
			if (player_node == NULL)
				return false;
			player = list_element(player_node);

			if (player->turn_direction == 0 && turn_direction > 0) {
				game->players_ready++;
			} else if (player->turn_direction > 0 && turn_direction == 0) {
				game->players_ready--;
			}
			player->turn_direction = turn_direction;
			return true;
		}
		case GS_IN_PROGRESS: {
			player_node = find_player(game->players, session_id);
			if (player_node == NULL)
				return false;
			player = list_element(player_node);
			player->turn_direction = turn_direction;
			return true;
		}
		case GS_OVER: {
			return true;
		}
		default:
			assert(false);
			break;
	}
}

void save_players_names(game_t *game) {
	game->players_names = calloc(list_size(game->players), sizeof(int8_t *));

	int i = 0;
	for (list_node_t *node = list_head(game->players); node != NULL; node = list_next(node), ++i) {
		player_t *player = list_element(node);
		game->players_names[i] = player->player_name;
	}
}

list_t *game_tick_waiting(game_t *game) {
	assert(game->state == GS_WAITING);
	if (game->players_ready > 1 && list_size(game->waiting) == game->players_ready) {
		// If there are at least 2 players and all of them are ready, start the game.
		game->state = GS_IN_PROGRESS;
	} else {
		return NULL;
	}
	printf("starting game\n");

	list_t *temp = game->players;
	game->players = game->waiting;
	game->waiting = temp;
	game->players_alive = list_size(game->players);

	list_t *events = list_create(sizeof(game_event_t));

	game_event_t event;

	event.type = GE_NEW_GAME;
	event.event_no = game->next_event_no++;
	event.data.new_game.max_x = game->width;
	event.data.new_game.max_y = game->height;
	event.data.new_game.players_num = list_size(game->players);
	save_players_names(game);
	event.data.new_game.players_names = game->players_names;

	list_add(events, &event);


	uint32_t x, y;
	uint8_t player_num = 0;
	for (list_node_t *node = list_head(game->players); node != NULL; node = list_next(node), player_num++) {
		player_t *player = list_element(node);
		player->x_pos = 0.5 + ((double) (rand_get() % (game->width)));
		player->y_pos = 0.5 + ((double) (rand_get() % (game->height)));
		player->direction = rand_get() % 360;
		player->status |= PS_IN_GAME;

		x = (double) player->x_pos;
		y = (double) player->y_pos;
		assert(x < game->width);
		assert(y < game->height);

		if (game_board_get(game->board, x, y)) {
			// Player eliminated.
			player->status |= PS_DEAD;
			game->players_alive--;
			event.type = GE_PLAYER_ELIMINATED;
			event.data.player_eliminated.player_number = player_num;
		} else {
			game_board_set(game->board, x, y);
			event.type = GE_PIXEL;
			event.data.pixel.player_number = player_num;
			event.data.pixel.x = x;
			event.data.pixel.y = y;
		}
		event.event_no = game->next_event_no++;
		list_add(events, &event);

	}

	return events;
}

list_t *game_tick_in_progress(game_t *game) {
	assert(game->state == GS_IN_PROGRESS);

	game_event_t event;
	list_t *events = list_create(sizeof(game_event_t));

	uint8_t player_num = 0;
	for (list_node_t *node = list_head(game->players); node != NULL; node = list_next(node), player_num++) {
		player_t *player = list_element(node);
		if (player->status & PS_DEAD) {
			continue;
		}
		assert(player->status & PS_IN_GAME);
		// Change player direction.
		switch (player->turn_direction) {
			case PD_RIGHT:
				player->direction++;
				if (player->direction >= 360)
					player->direction = 0;
				break;
			case PD_LEFT:
				player->direction--;
				if (player->direction >= 360)
					player->direction = 359;
				break;
		}
		uint32_t x = (uint32_t) player->x_pos, y = (uint32_t) player->y_pos;
		assert(x < game->width);
		assert(y < game->height);

		double player_direction_rad = ((double) player->direction) * M_PI / 180;
		player->x_pos += cos(player_direction_rad);
		player->y_pos += sin(player_direction_rad);

		uint32_t new_x = floor(player->x_pos), new_y = floor(player->y_pos);

		// Player is dead if out of the board.
		bool dead = player->x_pos < 0 || player->x_pos >= game->width;
		dead = dead || player->y_pos < 0 || player->y_pos >= game->height;

		// Player is dead if went to new pixel and it is already eaten.
		dead = dead || ((x != new_x || y != new_y) && game_board_get(game->board, new_x, new_y));

		if (dead) {
			player->status &= PS_DEAD;
			game->players_alive--;
			event.type = GE_PLAYER_ELIMINATED;
			event.data.player_eliminated.player_number = player_num;
		} else {
			event.type = GE_PIXEL;
			event.data.pixel.player_number = player_num;
			event.data.pixel.x = new_x;
			event.data.pixel.y = new_y;
		}
		event.event_no = game->next_event_no++;
		list_add(events, &event);
	}

	if (game->players_alive <= 1) {
		event.type = GE_GAME_OVER;
		event.event_no = game->next_event_no++;
		list_add(events, &event);
	}

	return events;
}

list_t *game_tick(game_t *game) {
//	printf("game_tick: pr=%d p=%zu w=%zu o=%zu\n",
//		   game->players_ready,
//		   list_size(game->players),
//		   list_size(game->waiting),
//		   list_size(game->observers)); // TODO remove.

	switch (game->state) {
		case GS_WAITING: {
			return game_tick_waiting(game);
		}
		case GS_IN_PROGRESS: {
			return game_tick_in_progress(game);
		}
		case GS_OVER: {
			return NULL;
		}
	}
	assert(false);
	return NULL;
}

bool game_restart(game_t *game) {
	game->game_id = rand_get();
	game->state = GS_WAITING;
	game->players_ready = 0;
	game->players_alive = 0;

	free(game->players_names);
	game->players_names = NULL;

	for (list_node_t *node = list_head(game->players); node != NULL; node = list_next(node)) {
		player_t *player = list_element(node);
		if (!(player->status & PS_DISCONNECTED))
			list_add(game->waiting, player);
	}
	list_remove_all(game->players);
}
