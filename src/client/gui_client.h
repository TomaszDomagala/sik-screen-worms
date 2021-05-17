#ifndef SIK_SCREEN_WORMS_GUI_CLIENT_H
#define SIK_SCREEN_WORMS_GUI_CLIENT_H


typedef struct {
	int sock_fd;
} gui_client_t;

int gui_client_connect(char *address, char *port, gui_client_t *client);

int gui_client_disconnect(gui_client_t *client);

int gui_client_send_event();


#endif //SIK_SCREEN_WORMS_GUI_CLIENT_H
