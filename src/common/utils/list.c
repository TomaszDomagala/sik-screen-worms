#include "list.h"

#include <stdlib.h>
#include <string.h>

struct list_node_s {
    list_node_t *prev;
    list_node_t *next;
    int8_t element[];
};

struct list_s {
    size_t element_size;
    size_t size;
    list_node_t *head;
    list_node_t *tail;
};

list_t *list_create(size_t element_size) {
    list_t *list = malloc(sizeof(list_t));
    if (list == NULL)
        return NULL;

    list->element_size = element_size;
    list->size = 0;
    list->head = NULL;
    list->tail = NULL;
    return list;
}

void list_free(list_t *list) {
    free(list);
}

size_t list_size(list_t *list) {
    return list->size;
}

void *list_element(list_node_t *node) {
    return node->element;
}

list_node_t *list_add(list_t *list, void *element) {
    list_node_t *node = malloc(sizeof(list_node_t) + list->element_size);
    if (node == NULL)
        return NULL;

    memcpy(node->element, element, list->element_size);

    if (list->tail == NULL) {
        list->head = node;
    } else {
        list->tail->next = node;
        node->prev = list->tail;
    }
    list->tail = node;

    list->size++;
    return node;
}

void list_remove(list_t *list, list_node_t *node) {
    list_node_t **prev_next, **next_prev;

    if (node->prev == NULL) {
        prev_next = &list->head;
    } else {
        prev_next = &node->prev->next;
    }

    if (node->next == NULL) {
        next_prev = &list->tail;
    } else {
        next_prev = &node->next->prev;
    }

    *prev_next = node->next;
    *next_prev = node->prev;

    free(node);
    list->size--;
}

list_node_t *list_head(list_t *list) {
    return list->head;
}

list_node_t *list_tail(list_t *list) {
    return list->tail;
}

list_node_t *list_next(list_node_t *node) {
    return node->next;
}

list_node_t *list_prev(list_node_t *node) {
    return node->prev;
}