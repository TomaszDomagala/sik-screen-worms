#ifndef SIK_SCREEN_WORMS_MESSAGES_H
#define SIK_SCREEN_WORMS_MESSAGES_H

#include <stdint.h>

#define MESS_MAX_SIZE 548

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

void mess_binary_free(mess_binary_t *mess);

int serialize_client_message(mess_binary_t *m_binary, mess_client_server_t *m_client);

int deserialize_client_message(mess_binary_t *m_binary, mess_client_server_t *m_client);


#endif //SIK_SCREEN_WORMS_MESSAGES_H
