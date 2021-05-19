#ifndef SIK_SCREEN_WORMS_CLIENTS_H
#define SIK_SCREEN_WORMS_CLIENTS_H

#include <stdint.h>

struct clients_collection_s;

typedef struct clients_collection_s clients_collection_t;

typedef struct {
    uint64_t session_id;
    int32_t sock_fd;
    int32_t timer_fd;
} client_t;

/**
 * Create new clients collection.
 */
clients_collection_t *clients_create();

/**
 * Free clients collection.
 * @param clients
 */
void clients_free(clients_collection_t *clients);

/**
 * Create new client with session_id and sock_fd.
 * New timerfd is created for this client.
 * @param clients - pointer to the clients collection.
 * @param session_id - client's session id.
 * @param sock_fd - client's socket file descriptor.
 * @return pointer to the created client or NULL if client could not be created.
 */
client_t *clients_new_client(clients_collection_t *clients, uint64_t session_id, int32_t sock_fd);

/**
 * Delete client with given sock_fd and close
 * file descriptors associated with this client.
 * @param clients - pointer to the clients collection.
 * @param sock_fd - client's socket file descriptor.
 */
void clients_delete_client(clients_collection_t *clients, int32_t sock_fd);

#endif //SIK_SCREEN_WORMS_CLIENTS_H
