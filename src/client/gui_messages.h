#ifndef SIK_SCREEN_WORMS_GUI_MESSAGES_H
#define SIK_SCREEN_WORMS_GUI_MESSAGES_H

#include <stdint.h>
#include <stdlib.h>
#include "messages.h"

// gui_message_t actions
#define GA_ERROR 0
#define GA_LEFT_KEY_DOWN 1
#define GA_LEFT_KEY_UP 2
#define GA_RIGHT_KEY_DOWN 3
#define GA_RIGHT_KEY_UP 4

typedef struct {
	int action;
} gui_message_t;

int serialize_client_gui_message(int8_t *buffer, game_event_t *event, int8_t **names);

int deserialize_gui_client_message(int8_t *buffer, size_t buffer_len, gui_message_t *message);

#endif //SIK_SCREEN_WORMS_GUI_MESSAGES_H
