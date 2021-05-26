#include "unity.h"
#include "messages.h"
#include "string.h"

void setUp() {

}

void tearDown() {

}

void test_client_to_server_messages() {
	int8_t buffer[MESS_MAX_SIZE];
	int ser_len, des_len;

	mess_client_server_t m_client;
	m_client.session_id = 400;
	m_client.turn_direction = 1;
	m_client.next_expected_event_no = 4000111000;
	strcpy(m_client.player_name, "player1");


	ser_len = serialize_client_message(buffer, &m_client);
	TEST_ASSERT_EQUAL(20, ser_len);

	mess_client_server_t deserialized;
	des_len = deserialize_client_message(buffer, ser_len, &deserialized);
	TEST_ASSERT_EQUAL(20,des_len);

	TEST_ASSERT_EQUAL(400, deserialized.session_id);
	TEST_ASSERT_EQUAL(1, deserialized.turn_direction);
	TEST_ASSERT_EQUAL(4000111000, deserialized.next_expected_event_no);
	TEST_ASSERT_EQUAL_STRING("player1", deserialized.player_name);
}

void test_event_pixel_message() {
	game_event_t in_event, out_event;
	int ser_len, des_len;
	int8_t buffer[550];

	in_event.event_no = 1;
	in_event.type = GE_PIXEL;
	in_event.data.pixel.player_number = 2;
	in_event.data.pixel.x = 3;
	in_event.data.pixel.y = 4;

	ser_len = serialize_game_event(buffer, &in_event);
	TEST_ASSERT_EQUAL(22, ser_len);

	des_len = deserialize_game_event(buffer, ser_len, &out_event);
	TEST_ASSERT_EQUAL(22, des_len);

	TEST_ASSERT_EQUAL(1, out_event.event_no);
	TEST_ASSERT_EQUAL(GE_PIXEL, out_event.type);
	TEST_ASSERT_EQUAL(2, out_event.data.pixel.player_number);
	TEST_ASSERT_EQUAL(3, out_event.data.pixel.x);
	TEST_ASSERT_EQUAL(4, out_event.data.pixel.y);
}

void test_event_player_eliminated_message() {
	game_event_t in_event, out_event;
	int ser_len, des_len;
	int8_t buffer[550];

	in_event.event_no = 1;
	in_event.type = GE_PLAYER_ELIMINATED;
	in_event.data.player_eliminated.player_number = 3;

	ser_len = serialize_game_event(buffer, &in_event);
	TEST_ASSERT_EQUAL(14, ser_len);

	des_len = deserialize_game_event(buffer, ser_len, &out_event);
	TEST_ASSERT_EQUAL(14, des_len);

	TEST_ASSERT_EQUAL(1, out_event.event_no);
	TEST_ASSERT_EQUAL(GE_PLAYER_ELIMINATED, out_event.type);
	TEST_ASSERT_EQUAL(3, out_event.data.player_eliminated.player_number);
}

void test_event_game_over_message() {
	game_event_t in_event, out_event;
	int ser_len, des_len;
	int8_t buffer[550];

	in_event.event_no = 1;
	in_event.type = GE_GAME_OVER;

	ser_len = serialize_game_event(buffer, &in_event);
	TEST_ASSERT_EQUAL(13, ser_len);

	des_len = deserialize_game_event(buffer, ser_len, &out_event);
	TEST_ASSERT_EQUAL(13, des_len);

	TEST_ASSERT_EQUAL(1, out_event.event_no);
	TEST_ASSERT_EQUAL(GE_GAME_OVER, out_event.type);
}

void test_event_new_game_message() {
	game_event_t in_event, out_event;
	int ser_len, des_len;
	int8_t buffer[550];

	int8_t *names[3];
	names[0] = "adam"; // len 5
	names[1] = "eve"; // len 4
	names[2] = "cain1234"; // len 9

	in_event.event_no = 20;
	in_event.type = GE_NEW_GAME;
	in_event.data.new_game.max_x = 300;
	in_event.data.new_game.max_y = 400;
	in_event.data.new_game.players_num = 3;
	in_event.data.new_game.players_names = names;

	ser_len = serialize_game_event(buffer, &in_event);
	// 13 + 8 + 5 + 4 + 9 = 39
	TEST_ASSERT_EQUAL(39, ser_len);

	des_len = deserialize_game_event(buffer, ser_len, &out_event);
	TEST_ASSERT_EQUAL(39, des_len);

	TEST_ASSERT_EQUAL(20, out_event.event_no);
	TEST_ASSERT_EQUAL(GE_NEW_GAME, out_event.type);
	TEST_ASSERT_EQUAL(300, out_event.data.new_game.max_x);
	TEST_ASSERT_EQUAL(400, out_event.data.new_game.max_y);
	TEST_ASSERT_EQUAL(3, out_event.data.new_game.players_num);

	int8_t **out_names = out_event.data.new_game.players_names;
	TEST_ASSERT_EQUAL_STRING("adam", out_names[0]);
	TEST_ASSERT_EQUAL_STRING("eve", out_names[1]);
	TEST_ASSERT_EQUAL_STRING("cain1234", out_names[2]);
}

int main() {
	UNITY_BEGIN();
	RUN_TEST(test_client_to_server_messages);

	RUN_TEST(test_event_pixel_message);
	RUN_TEST(test_event_player_eliminated_message);
	RUN_TEST(test_event_game_over_message);
	RUN_TEST(test_event_new_game_message);

	return UNITY_END();
}
