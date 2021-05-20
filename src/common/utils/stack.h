#ifndef SIK_SCREEN_WORMS_STACK_H
#define SIK_SCREEN_WORMS_STACK_H

#include <stdlib.h>

typedef struct stack_s stack_t;

stack_t *stack_create(size_t element_size);

ssize_t stack_push(stack_t *stack, void *element);

size_t stack_size(stack_t *stack);

void *stack_get(stack_t *stack, size_t index);

void stack_free(stack_t *stack);


#endif //SIK_SCREEN_WORMS_STACK_H
