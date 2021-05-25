#include "unity.h"
#include "gui_messages.h"
#include <string.h>

void setUp() {

}

void tearDown() {

}

void test_serialize_gui_pixel_message() {
	game_event_t in_event, out_event;
	int ser_len, des_len;
	int8_t buffer[550];
	memset(buffer, 0, sizeof(buffer));

	int8_t *names[3];
	names[0] = "adam"; // len 5
	names[1] = "eve"; // len 4
	names[2] = "cain1234"; // len 9


	in_event.event_no = 1;
	in_event.type = GE_PIXEL;
	in_event.data.pixel.player_number = 2;
	in_event.data.pixel.x = 3;
	in_event.data.pixel.y = 4;

	ser_len = serialize_client_gui_message(buffer, &in_event, names);

	TEST_ASSERT_EQUAL(19, ser_len);
	TEST_ASSERT_EQUAL_STRING("PIXEL 3 4 cain1234\n", buffer);
}

void test_serialize_gui_player_eliminated_message() {
	game_event_t in_event, out_event;
	int ser_len, des_len;
	int8_t buffer[550];
	memset(buffer, 0, sizeof(buffer));

	int8_t *names[3];
	names[0] = "adam"; // len 5
	names[1] = "eve"; // len 4
	names[2] = "cain1234"; // len 9


	in_event.event_no = 1;
	in_event.type = GE_PLAYER_ELIMINATED;
	in_event.data.player_eliminated.player_number = 1;

	ser_len = serialize_client_gui_message(buffer, &in_event, names);

	TEST_ASSERT_EQUAL(22, ser_len);
	TEST_ASSERT_EQUAL_STRING("PLAYER_ELIMINATED eve\n", buffer);
}


void test_serialize_gui_new_game_message() {
	game_event_t in_event;
	int ser_len;
	int8_t buffer[550];
	memset(buffer, 0, sizeof(buffer));

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

	ser_len = serialize_client_gui_message(buffer, &in_event, names);

	TEST_ASSERT_EQUAL(9 + 4 + 4 + 5 + 4 + 9, ser_len); // 35
	TEST_ASSERT_EQUAL_STRING("NEW_GAME 300 400 adam eve cain1234\n", buffer);
}

void test_deserialize_gui_message() {
	int8_t *buffer;
	gui_message_t message;
	int des_len;

	// Correct messages.
	buffer = "LEFT_KEY_DOWN\n";
	des_len = deserialize_gui_client_message(buffer, 14, &message);
	TEST_ASSERT_EQUAL(14, des_len);
	TEST_ASSERT_EQUAL(GA_LEFT_KEY_DOWN, message.action);

	buffer = "LEFT_KEY_UP\n";
	des_len = deserialize_gui_client_message(buffer, 12, &message);
	TEST_ASSERT_EQUAL(12, des_len);
	TEST_ASSERT_EQUAL(GA_LEFT_KEY_UP, message.action);

	buffer = "RIGHT_KEY_DOWN\n";
	des_len = deserialize_gui_client_message(buffer, 15, &message);
	TEST_ASSERT_EQUAL(15, des_len);
	TEST_ASSERT_EQUAL(GA_RIGHT_KEY_DOWN, message.action);

	buffer = "RIGHT_KEY_UP\n";
	des_len = deserialize_gui_client_message(buffer, 13, &message);
	TEST_ASSERT_EQUAL(13, des_len);
	TEST_ASSERT_EQUAL(GA_RIGHT_KEY_UP, message.action);

	// Buffer too short.
	des_len = deserialize_gui_client_message(buffer, 7, &message);
	TEST_ASSERT_EQUAL(7, des_len);
	TEST_ASSERT_EQUAL(GA_ERROR, message.action);

	// No newline character.
	buffer = "RIGHT_KEY_UP";
	des_len = deserialize_gui_client_message(buffer, 12, &message);
	TEST_ASSERT_EQUAL(12, des_len);
	TEST_ASSERT_EQUAL(GA_ERROR, message.action);
}


int main() {
	UNITY_BEGIN();

	RUN_TEST(test_serialize_gui_new_game_message);
	RUN_TEST(test_serialize_gui_pixel_message);
	RUN_TEST(test_serialize_gui_player_eliminated_message);
	RUN_TEST(test_deserialize_gui_message);

	return UNITY_END();
}
