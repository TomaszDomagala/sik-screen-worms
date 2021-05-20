#include "clients.h"
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/timerfd.h>
#include <unistd.h>
#include "config.h"


#define MAX_CLIENTS_NUM SERVER_MAX_CAPACITY

struct clients_collection_s {
    client_t arr[MAX_CLIENTS_NUM];
    uint32_t num;
};

clients_collection_t *clients_create() {
    clients_collection_t *clients = malloc(sizeof(clients_collection_t));
    if (clients == NULL) {
        return NULL;
    }
    for (int i = 0; i < MAX_CLIENTS_NUM; ++i) {
        clients->arr[i].sock_fd = -1;
    }
    clients->num = 0;
    return clients;
}

client_t *clients_new_client(clients_collection_t *clients, uint64_t session_id, int32_t sock_fd) {
    assert(clients != NULL);
    if (clients->num == MAX_CLIENTS_NUM) {
        return NULL;
    }
    int i;
    for (i = 0; i < MAX_CLIENTS_NUM; ++i) {
        if (clients->arr[i].sock_fd == -1) {
            clients->arr[i].session_id = session_id;
            clients->arr[i].sock_fd = sock_fd;
            clients->arr[i].timer_fd = timerfd_create(CLOCK_BOOTTIME, 0);
            break;
        }
    }
    assert(i < MAX_CLIENTS_NUM);
    clients->num++;
    return &clients->arr[i];
}

void clients_delete_client(clients_collection_t *clients, int32_t sock_fd) {
    assert(clients != NULL);

    for (int i = 0; i < MAX_CLIENTS_NUM; ++i) {
        client_t *client = &clients->arr[i];
        if (client->sock_fd == sock_fd) {
            close(client->sock_fd);
            close(client->timer_fd);

            client->sock_fd = -1;
        }
    }

}

client_t *clients_find_client_by_session_id(clients_collection_t *clients, uint64_t session_id) {
    for (int i = 0; i < MAX_CLIENTS_NUM; ++i) {
        client_t *client = &clients->arr[i];
        if (client->sock_fd != -1 && client->session_id == session_id) {
            return client;
        }
    }
    return NULL;
}