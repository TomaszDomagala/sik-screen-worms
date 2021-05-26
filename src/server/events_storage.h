#ifndef SIK_SCREEN_WORMS_EVENTS_STORAGE_H
#define SIK_SCREEN_WORMS_EVENTS_STORAGE_H

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include "messages.h"

/**
 * Event storage serializes and saves events in one chunk of memory for fast access.
 * It assumes that events are saved in the same order as they are generated.
 */
typedef struct event_storage_s event_storage_t;

typedef struct {
	size_t len;
	size_t offset;
} serialized_event_t;

event_storage_t *event_storage_create();

bool event_storage_push(event_storage_t *storage, game_event_t *event);

serialized_event_t *event_storage_get(event_storage_t *storage, size_t index);

ssize_t event_storage_last(event_storage_t *storage);

void event_storage_free(event_storage_t *storage);

int8_t *event_storage_get_data(event_storage_t *storage, size_t offset);

#endif //SIK_SCREEN_WORMS_EVENTS_STORAGE_H
