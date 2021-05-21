#include "stack.h"

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define INITIAL_CAPACITY 4

struct stack_s {
    size_t element_size;
    size_t size;
    size_t capacity;
    int8_t *data;
};

size_t stack_size(stack_t *stack) {
    return stack->size;
}

void stack_free(stack_t *stack) {
    if (stack == NULL) return;
    free(stack->data);
    free(stack);
}

stack_t *stack_create(size_t element_size) {
    stack_t *stack = malloc(sizeof(stack_t));
    if (stack == NULL)
        return NULL;

    stack->element_size = element_size;
    stack->size = 0;
    stack->capacity = INITIAL_CAPACITY;
    stack->data = malloc(stack->capacity * stack->element_size);

    if (stack->data == NULL) {
        free(stack);
        return NULL;
    }
    return stack;
}

void *stack_get(stack_t *stack, size_t index) {
    if (stack->size <= index)
        return NULL;
    return (stack->data + stack->element_size * index);
}


ssize_t stack_push(stack_t *stack, void *element) {
    if (stack->size == stack->capacity) {
        int8_t *new_block = realloc(stack->data, 2 * stack->capacity * stack->element_size);
        if (new_block == NULL)
            return -1;
        stack->capacity *= 2;
        stack->data = new_block;
    }
    memcpy((stack->data + stack->element_size * stack->size), element, stack->element_size);
    return stack->size++;
}