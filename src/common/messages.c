#include <messages.h>
#include <string.h>


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



int serialize_game_event(mess_binary_t *m_binary, game_event_t *event, int8_t *names) {
	switch (event->type) {

	}
}