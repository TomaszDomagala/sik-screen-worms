#include "gui_messages.h"
#include <string.h>
#include <assert.h>
#include <stdio.h>

int serialize_client_gui_message_new_game(int8_t *buffer, game_event_t *event, int8_t **names) {
	int8_t *buffer_start = buffer;

	buffer += sprintf(buffer, "NEW_GAME");
	buffer += sprintf(buffer, " %d", event->data.new_game.max_x);
	buffer += sprintf(buffer, " %d", event->data.new_game.max_y);

	for (int i = 0; i < event->data.new_game.players_num; ++i) {
		buffer += sprintf(buffer, " %s", names[i]);
	}
	buffer += sprintf(buffer, "\n");

	return buffer - buffer_start;
}

int serialize_client_gui_message_pixel(int8_t *buffer, game_event_t *event, int8_t **names) {
	int8_t *buffer_start = buffer;

	buffer += sprintf(buffer, "PIXEL ");
	buffer += sprintf(buffer, "%d ", event->data.pixel.x);
	buffer += sprintf(buffer, "%d ", event->data.pixel.y);
	buffer += sprintf(buffer, "%s\n", names[event->data.pixel.player_number]);

	return buffer - buffer_start;
}

int serialize_client_gui_message_player_eliminated(int8_t *buffer, game_event_t *event, int8_t **names) {
	int8_t *buffer_start = buffer;

	buffer += sprintf(buffer, "PLAYER_ELIMINATED ");
	buffer += sprintf(buffer, "%s\n", names[event->data.pixel.player_number]);

	return buffer - buffer_start;
}

int serialize_client_gui_message(int8_t *buffer, game_event_t *event, int8_t **names) {
	switch (event->type) {
		case GE_NEW_GAME:
			return serialize_client_gui_message_new_game(buffer, event, names);
		case GE_PIXEL:
			return serialize_client_gui_message_pixel(buffer, event, names);
		case GE_PLAYER_ELIMINATED:
			return serialize_client_gui_message_player_eliminated(buffer, event, names);
	}
	assert(0);
	return 0;
}

int deserialize_gui_client_message(int8_t *buffer, size_t buffer_len, gui_message_t *message) {
	int8_t *message_end = memchr(buffer, '\n', buffer_len);
	if (message_end == NULL) {
		message->action = GA_ERROR;
		return buffer_len;
	}
	int64_t action_len = message_end - buffer + 1;

	if (action_len == 14 && strncmp(buffer, "LEFT_KEY_DOWN\n", 14) == 0) {
		message->action = GA_LEFT_KEY_DOWN;
	} else if (action_len == 12 && strncmp(buffer, "LEFT_KEY_UP\n", 12) == 0) {
		message->action = GA_LEFT_KEY_UP;
	} else if (action_len == 15 && strncmp(buffer, "RIGHT_KEY_DOWN\n", 15) == 0) {
		message->action = GA_RIGHT_KEY_DOWN;
	} else if (action_len == 13 && strncmp(buffer, "RIGHT_KEY_UP\n", 13) == 0) {
		message->action = GA_RIGHT_KEY_UP;
	} else {
		message->action = GA_ERROR;
	}

	return action_len;
}