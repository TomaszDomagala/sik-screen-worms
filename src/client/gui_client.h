#ifndef SIK_SCREEN_WORMS_GUI_CLIENT_H
#define SIK_SCREEN_WORMS_GUI_CLIENT_H

#include "messages.h"
#include "gui_messages.h"

typedef struct gui_client_s gui_client_t;

gui_client_t *gui_client_connect(char *address, char *port);

void gui_client_disconnect(gui_client_t *client);

int gui_client_send_event(gui_client_t *client, game_event_t *event, int8_t **names);

int gui_client_recv_event(gui_client_t *client, gui_message_t *gui_message);

int gui_client_socket(gui_client_t *client);


#endif //SIK_SCREEN_WORMS_GUI_CLIENT_H
