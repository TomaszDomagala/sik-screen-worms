#include "events_storage.h"

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "utils/stack.h"
#include "messages.h"

#define DEFAULT_BLOCK_SIZE 1024

struct event_storage_s {
	size_t block_size;
	size_t block_capacity;

	int8_t *block;
	int8_t *next_event_pos;
	stack_t *events;


	int8_t buffer[550];
};

event_storage_t *event_storage_create() {
	event_storage_t *storage = malloc(sizeof(event_storage_t));
	if (storage == NULL)
		return NULL;

	storage->block = malloc(DEFAULT_BLOCK_SIZE);
	storage->events = stack_create(sizeof(serialized_event_t));
	storage->block_size = 0;
	storage->block_capacity = DEFAULT_BLOCK_SIZE;
	storage->next_event_pos = storage->block;

	if (storage->block == NULL || storage->events == NULL) {
		stack_free(storage->events);
		free(storage->block);
		free(storage);
		return NULL;
	}

	return storage;
}

ssize_t event_storage_last(event_storage_t *storage) {
	ssize_t last = stack_size(storage->events);
	return last - 1;
}

bool event_storage_push(event_storage_t *storage, game_event_t *event) {
	size_t event_size = serialize_game_event(storage->buffer, event);

	if (storage->block_size + event_size > storage->block_capacity) {
		size_t new_capacity = 2 * storage->block_capacity;
		int8_t *new_block = realloc(storage->block, new_capacity);
		if (new_block == NULL) {
			return false;
		}
		storage->block = new_block;
		storage->block_capacity = new_capacity;
	}
	assert(storage->block_size + event_size <= storage->block_capacity);


	memcpy(storage->next_event_pos, storage->buffer, event_size);

	serialized_event_t s_event;
	s_event.data = storage->next_event_pos;
	s_event.len = event_size;
	if (stack_push(storage->events, &s_event) == -1) {
		return false;
	}
	storage->block_size += event_size;
	storage->next_event_pos += event_size;

	return true;
}

serialized_event_t *event_storage_get(event_storage_t *storage, size_t index) {
	return stack_get(storage->events, index);
}
