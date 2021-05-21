#include "game.h"

#include <stdint.h>
#include "config.h"
#include "utils/stack.h"
#include "utils/list.h"
#include "game_board.h"
#include <assert.h>
#include "string.h"

#define MAX_PLAYERS_NUM SERVER_MAX_CAPACITY

#define GE_NEW_GAME 0
#define GE_PIXEL 1
#define GE_PLAYER_ELIMINATED 2
#define GE_GAME_OVER 3

typedef struct {
    uint32_t max_x;
    uint32_t max_y;
} ge_data_new_game_t;

typedef struct {
    uint8_t player_number;
    uint32_t x;
    uint32_t y;
} ge_data_pixel_t;

typedef struct {
    uint8_t player_number;
} ge_data_player_eliminated_t;

typedef union {
    ge_data_new_game_t new_game;
    ge_data_pixel_t pixel;
    ge_data_player_eliminated_t player_eliminated;
} game_event_data_t;


struct {
    uint8_t type;
    game_event_data_t data;
} game_event_t;

struct game_s {
    uint8_t state;
    uint32_t game_id;

    list_t *players;
    list_t *observers;

    stack_t *events;
    game_board_t *board;

    uint32_t players_all;
    uint32_t players_alive;
    uint32_t players_ready;
};

game_t *game_create(uint32_t game_id, size_t width, size_t height) {
    game_t *game = malloc(sizeof(game_t));
    if (game == NULL)
        return NULL;

    game->state = GS_WAITING;
    game->game_id = game_id;

    game->players = list_create(sizeof(player_t));
    game->observers = list_create(sizeof(player_t));
    game->events = stack_create(sizeof(game_event_t));
    game->board = game_board_create(width, height);

    game->players_all = 0;
    game->players_alive = 0;
    game->players_ready = 0;

    if (game->players == NULL || game->observers == NULL || game->events == NULL || game->board == NULL) {
        list_free(game->players);
        list_free(game->observers);
        stack_free(game->events);
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

bool game_add_player(game_t *game, uint64_t session_id, int8_t *player_name) {
    if (find_player(game->players, session_id) != NULL)
        return true; // Player is already in game.
    if (find_player(game->observers, session_id) != NULL)
        return true; // Player is already an observer.

    player_t player = player_create(session_id, player_name);

    return list_add(game->observers, &player) != NULL;
}

bool game_remove_player(game_t *game, uint64_t session_id) {
    list_node_t *player_node = find_player(game->observers, session_id);
    if (player_node != NULL) {
        list_remove(game->observers, player_node);
        return true;
    }
    return false;
}

bool game_set_direction(game_t *game, uint64_t session_id, uint8_t turn_direction) {
    list_node_t *player_node;
    player_t *player;

    switch (game->state) {
        case GS_WAITING: {
            player_node = find_player(game->observers, session_id);
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
