#ifndef SIK_SCREEN_WORMS_LIST_H
#define SIK_SCREEN_WORMS_LIST_H

#include "stdlib.h"
#include <stdbool.h>

typedef struct list_s list_t;

list_t *list_create(size_t limit);

ssize_t list_add(list_t *list, void *element);

void *list_get(list_t *list, size_t index);

size_t list_size(list_t *list);

size_t list_capacity(list_t *list);

void list_destroy(list_t *list);

#endif //SIK_SCREEN_WORMS_LIST_H
