#include <messages.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

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


int serialize_client_message(mess_binary_t *m_binary, mess_client_server_t *m_client) {
	uint64_t be_session_id = htobe64(m_client->session_id);
	uint32_t be_next_expected_event_no = htobe32(m_client->next_expected_event_no);

	memcpy(&m_binary->data[0], &be_session_id, 8);
	memcpy(&m_binary->data[8], &m_client->turn_direction, 1);
	memcpy(&m_binary->data[9], &be_next_expected_event_no, 4);

	uint64_t name_len = strlen(m_client->player_name);
	if (name_len)
		memcpy(&m_binary->data[13], m_client->player_name, name_len);

	m_binary->size = 13 + name_len;

	return 0;
}

int deserialize_client_message(mess_binary_t *m_binary, mess_client_server_t *m_client) {
	if (m_binary->size < 13 || m_binary->size > 33)
		return -1;

	memcpy(&m_client->session_id, m_binary->data + 0, 8);
	memcpy(&m_client->turn_direction, m_binary->data + 8, 1);
	memcpy(&m_client->next_expected_event_no, m_binary->data + 9, 4);

	uint64_t name_len = m_binary->size - 13;
	memset(m_client->player_name, 0, 21);
	if (name_len)
		memcpy(m_client->player_name, m_binary->data + 13, name_len);

	m_client->session_id = be64toh(m_client->session_id);
	m_client->next_expected_event_no = be32toh(m_client->next_expected_event_no);

	return 0;
}


int serialize_game_event_new_game(int8_t *buffer, game_event_t *event) {
	int8_t *orig_buff = buffer;

	uint32_t max_x = htobe32(event->data.new_game.max_x);
	uint32_t max_y = htobe32(event->data.new_game.max_y);

	memcpy(buffer, &max_x, sizeof(uint32_t));
	buffer += sizeof(uint32_t);
	memcpy(buffer, &max_y, sizeof(uint32_t));
	buffer += sizeof(uint32_t);

	int len;
	int8_t **names = event->data.new_game.players_names;

	for (int i = 0; i < event->data.new_game.players_num; ++i) {
		len = strlen(names[i]) + 1;
		memcpy(buffer, names[i], len);
		buffer += len;
	}
	return (int) (buffer - orig_buff);
}

int serialize_game_event_pixel(int8_t *buffer, game_event_t *event) {
	uint32_t be_x, be_y;

	memcpy(buffer, &event->data.pixel.player_number, sizeof(uint8_t));
	buffer += sizeof(uint8_t);

	be_x = htobe32(event->data.pixel.x);
	memcpy(buffer, &be_x, sizeof(uint32_t));
	buffer += sizeof(uint32_t);

	be_y = htobe32(event->data.pixel.y);
	memcpy(buffer, &be_y, sizeof(uint32_t));

	return 9;
}

int serialize_game_event_player_eliminated(int8_t *buffer, game_event_t *event) {
	memcpy(buffer, &event->data.player_eliminated.player_number, sizeof(uint8_t));
	return 1;
}

int serialize_game_event_game_over() {
	return 0;
}


int serialize_game_event(int8_t *buffer, game_event_t *event) {
	int8_t *data = buffer + 9; // len + event_no + event_type.
	uint32_t event_len, total_len, be_len, be_event_no, be_crc32;

	switch (event->type) {
		case GE_NEW_GAME:
			event_len = serialize_game_event_new_game(data, event);
			break;
		case GE_PIXEL:
			event_len = serialize_game_event_pixel(data, event);
			break;
		case GE_PLAYER_ELIMINATED:
			event_len = serialize_game_event_player_eliminated(data, event);
			break;
		case GE_GAME_OVER:
			event_len = serialize_game_event_game_over();
			break;
	}

	event_len += 5; // event_no + event_type + event_data.
	be_len = htobe32(event_len);
	memcpy(buffer, &be_len, sizeof(uint32_t));
	buffer += sizeof(uint32_t);

	be_event_no = htobe32(event->event_no);
	memcpy(buffer, &be_event_no, sizeof(uint32_t));
	buffer += sizeof(uint32_t);

	memcpy(buffer, &event->type, sizeof(uint8_t));
	buffer += event_len - 4; // Move buffer to crc32 field.

	// TODO crc32
	be_crc32 = htobe32(7);
	memcpy(buffer, &be_crc32, sizeof(uint32_t));

	return event_len + 2 * sizeof(uint32_t); //
}

int deserialize_game_event_new_game(int8_t *buffer, game_event_t *event, uint32_t data_len) {
	// TODO add error handling.

	memcpy(&event->data.new_game.max_x, buffer, sizeof(uint32_t));
	event->data.new_game.max_x = be32toh(event->data.new_game.max_x);
	buffer += sizeof(uint32_t);

	memcpy(&event->data.new_game.max_y, buffer, sizeof(uint32_t));
	event->data.new_game.max_y = be32toh(event->data.new_game.max_y);
	buffer += sizeof(uint32_t);

	int32_t names_len = data_len - 2 * sizeof(uint32_t);

	int8_t **names = calloc(25, sizeof(int8_t *));
	uint8_t names_num = 0;

	while (names_len > 0) {
		int len = strlen(buffer) + 1; // Length with \0.
		names[names_num] = malloc(names_len);
		memcpy(names[names_num], buffer, len);
		buffer += len;
		names_len -= len;
		names_num++;
	}

	event->data.new_game.players_num = names_num;
	event->data.new_game.players_names = names;

	return data_len;
}

int deserialize_game_event_pixel(int8_t *buffer, game_event_t *event) {
	memcpy(&event->data.pixel.player_number, buffer, sizeof(uint8_t));
	buffer += sizeof(uint8_t);

	memcpy(&event->data.pixel.x, buffer, sizeof(uint32_t));
	event->data.pixel.x = be32toh(event->data.pixel.x);
	buffer += sizeof(uint32_t);

	memcpy(&event->data.pixel.y, buffer, sizeof(uint32_t));
	event->data.pixel.y = be32toh(event->data.pixel.y);

	return 9;
}

int deserialize_game_event_player_eliminated(int8_t *buffer, game_event_t *event) {
	memcpy(&event->data.player_eliminated.player_number, buffer, sizeof(uint8_t));
	return 1;
}

int deserialize_game_event_game_over(int8_t *buffer, game_event_t *event) {
	// TODO maybe remove this func.
	return 0;
}

int deserialize_game_event(int8_t *buffer, game_event_t *event) {
	uint32_t event_len;

	// TODO crc32
	memcpy(&event_len, buffer, sizeof(uint32_t));
	event_len = be32toh(event_len);
	buffer += sizeof(uint32_t);

	memcpy(&event->event_no, buffer, sizeof(uint32_t));
	event->event_no = be32toh(event->event_no);
	buffer += sizeof(uint32_t);

	memcpy(&event->type, buffer, sizeof(uint8_t));
	buffer += sizeof(uint8_t);

	switch (event->type) {
		case GE_NEW_GAME:
			deserialize_game_event_new_game(buffer, event, event_len - 5);
			break;
		case GE_PIXEL:
			deserialize_game_event_pixel(buffer, event);
			break;
		case GE_PLAYER_ELIMINATED:
			deserialize_game_event_player_eliminated(buffer, event);
			break;
		case GE_GAME_OVER:
			deserialize_game_event_game_over(buffer, event);
			break;
	}
	// TODO crc32.
	return event_len + 2 * sizeof(uint32_t);
}
