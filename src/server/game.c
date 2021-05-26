#include "game.h"

#include <stdbool.h>
#include <stdint.h>
#include "config.h"
#include "rand.h"
#include "list.h"
#include "game_board.h"
#include <assert.h>
#include "string.h"
#include "messages.h"
#include <math.h>
#include <stdio.h>

#define MAX_PLAYERS_NUM SERVER_MAX_CAPACITY


struct game_s {
	bool in_progress;
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

	game->in_progress = false;
	game->game_id = 0;

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

bool game_in_progress(game_t *game) {
	return game->in_progress;
}

player_t player_create(uint64_t session_id, int8_t *player_name) {
	player_t player;
	player.session_id = session_id;
	player.status = PS_WAITING;
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

void game_restart(game_t *game) {
	game->in_progress = false;
	game->players_ready = 0;
	game->players_alive = 0;
	game->next_event_no = 0;

	game_board_free(game->board);
	game->board = game_board_create(game->width, game->height);

	free(game->players_names);
	game->players_names = NULL;

	for (list_node_t *node = list_head(game->players); node != NULL; node = list_next(node)) {
		player_t *player = list_element(node);
		if (!(player->status & PS_DISCONNECTED))
			list_add(game->waiting, player);
	}
	list_remove_all(game->players);
	for (list_node_t *node = list_head(game->waiting); node != NULL; node = list_next(node)) {
		player_t *player = list_element(node);
		player->turn_direction = 0;
		player->status = PS_WAITING;
	}

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

	if (game->in_progress) {
		player_node = find_player(game->players, session_id);
		if (player_node == NULL)
			return false;
		player = list_element(player_node);
		player->turn_direction = turn_direction;
		return true;
	} else {
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
}

void save_players_names(game_t *game) {
	game->players_names = calloc(list_size(game->players), sizeof(int8_t *));

	int i = 0;
	for (list_node_t *node = list_head(game->players); node != NULL; node = list_next(node), ++i) {
		player_t *player = list_element(node);
		game->players_names[i] = player->player_name;
	}
}

/**
 * Sort players list in alphabetical order.
 * @param players - list of players to sort.
 */
void sort_players(list_t *players) {
	list_t *sorted = list_create(sizeof(player_t));
	list_node_t *min_node;
	player_t *min_player;

	while (list_size(players)) {
		min_node = list_head(players);
		min_player = list_element(min_node);

		for (list_node_t *node = list_next(min_node); node != NULL; node = list_next(node)) {
			player_t *player = list_element(node);
			if (strcasecmp(player->player_name, min_player->player_name) < 0) {
				min_node = node;
				min_player = player;
			}
		}
		list_add(sorted, min_player);
		list_remove(players, min_node);
	}
	for (list_node_t *node = list_head(sorted); node != NULL; node = list_next(node)) {
		list_add(players, list_element(node));
	}
	list_remove_all(sorted);
	list_free(sorted);
}

list_t *game_tick_waiting(game_t *game) {
	if (game->players_ready > 1 && list_size(game->waiting) == game->players_ready) {
		// If there are at least 2 players and all of them are ready, start the game.
		game->in_progress = true;
	} else {
		return NULL;
	}
	game->game_id = rand_get();

	list_t *temp = game->players;
	game->players = game->waiting;
	game->waiting = temp;
	game->players_alive = list_size(game->players);
	sort_players(game->players);

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

		if (x == new_x && y == new_y)
			continue;

		// Player is dead if out of the board.
		bool dead = player->x_pos < 0 || player->x_pos >= game->width;
		dead = dead || player->y_pos < 0 || player->y_pos >= game->height;

		// Player is dead if went to new pixel and it is already eaten.
		dead = dead || game_board_get(game->board, new_x, new_y);

		if (dead) {
			player->status |= PS_DEAD;
			game->players_alive--;
			event.type = GE_PLAYER_ELIMINATED;
			event.data.player_eliminated.player_number = player_num;
		} else {
			event.type = GE_PIXEL;
			event.data.pixel.player_number = player_num;
			event.data.pixel.x = new_x;
			event.data.pixel.y = new_y;
			game_board_set(game->board, new_x, new_y);
		}
		event.event_no = game->next_event_no++;
		list_add(events, &event);
	}

	if (game->players_alive <= 1) {
		// Game end.
		event.type = GE_GAME_OVER;
		event.event_no = game->next_event_no++;
		list_add(events, &event);
		game_restart(game);
	}

	return events;
}

list_t *game_tick(game_t *game) {
//	printf("game_tick: pr=%d p=%zu w=%zu o=%zu\n",
//		   game->players_ready,
//		   list_size(game->players),
//		   list_size(game->waiting),
//		   list_size(game->observers)); // TODO remove.
	if (game->in_progress) {
		return game_tick_in_progress(game);
	} else {
		return game_tick_waiting(game);
	}
}


uint32_t game_get_id(game_t *game) {
	return game->game_id;
}

