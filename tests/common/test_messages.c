#include "unity.h"
#include "messages.h"
#include "string.h"

void setUp() {

}

void tearDown() {

}

void client_to_server_messages() {
	int8_t buffer[MESS_MAX_SIZE];


	mess_client_server_t m_client;
	m_client.session_id = 400;
	m_client.turn_direction = 1;
	m_client.next_expected_event_no = 4000111000;
	strcpy(m_client.player_name, "player1");

	mess_binary_t m_binary;
	m_binary.data = buffer;

	serialize_client_message(&m_binary, &m_client);

	mess_client_server_t deserialized;
	deserialize_client_message(&m_binary, &deserialized);

	TEST_ASSERT_EQUAL(400, deserialized.session_id);
	TEST_ASSERT_EQUAL(1, deserialized.turn_direction);
	TEST_ASSERT_EQUAL(4000111000, deserialized.next_expected_event_no);
	TEST_ASSERT_EQUAL_STRING("player1", deserialized.player_name);
}


int main() {
	UNITY_BEGIN();
	RUN_TEST(client_to_server_messages);

	return UNITY_END();
}
