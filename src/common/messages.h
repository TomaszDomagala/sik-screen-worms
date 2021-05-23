#ifndef SIK_SCREEN_WORMS_MESSAGES_H
#define SIK_SCREEN_WORMS_MESSAGES_H

#include <stdint.h>

#define MESS_MAX_SIZE 550

// TODO delete this struct and refactor functions to return number of bytes and accept buffer pointers.
typedef struct {
	int32_t size;
	int8_t *data;
} mess_binary_t;

typedef struct {
	uint64_t session_id;
	uint8_t turn_direction;
	uint32_t next_expected_event_no;
	int8_t player_name[21];
} mess_client_server_t;

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


typedef struct {
	uint8_t type;
	game_event_data_t data;
} game_event_t;

void mess_binary_free(mess_binary_t *mess);

int serialize_client_message(mess_binary_t *m_binary, mess_client_server_t *m_client);

int deserialize_client_message(mess_binary_t *m_binary, mess_client_server_t *m_client);

int serialize_game_event(mess_binary_t *m_binary, game_event_t *event, int8_t *names);

int deserialize_game_event(mess_binary_t *m_binary, game_event_t *event, int8_t *names);

#endif //SIK_SCREEN_WORMS_MESSAGES_H
