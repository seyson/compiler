#include <stdio.h>
#include <stdlib.h>
#include "stack.h"

void stack_init(Stack* stack, int size)
{
    item_t *items = malloc(size * sizeof (item_t)); /* allocate space for the stack's items */
    if (items == NULL) {
        printf("malloc() failed to create stack\n");
        exit(EXIT_FAILURE);
    }
    (*stack).items = items;
    (*stack).capacity = size;
    (*stack).top = -1;
}

void stack_free(Stack* stack)
{
    free((*stack).items); /* free the stack's items */
    (*stack).items = NULL;
    (*stack).top = -1;
}

int stack_is_empty(Stack* stack)
{
    return (*stack).top < 0;
}

int stack_is_full(Stack* stack)
{
    return (*stack).top == (*stack).capacity - 1;
}

void push(Stack* stack, item_t item)
{
    if (stack_is_full(stack)) {
        printf("Runtime stack is past its capacity (probably a bug)\n");
        exit(EXIT_FAILURE);
    }
    (*stack).items[++((*stack).top)] = item;
}

item_t pop(Stack* stack)
{
    if (stack_is_empty(stack)) {
        printf("pop() failed (stack is empty)\n");
        exit(EXIT_FAILURE);
    }
    return (*stack).items[(*stack).top--];
}

void item_print(item_t item)
{
    switch (item.type) {
        case INT:
            printf("INT: %d\n", item.i);
            break;
        case REAL:
            printf("FLOAT: %f\n", item.f);
            break;
        case CHAR:
            printf("CHAR: %c\n", item.i);
            break;
        case STR:
            printf("STR: %s\n", item.s);
            break;
    }
}

void stack_print(Stack* stack)
{
    printf("Printing stack contents:\n");
    if (!stack_is_empty(stack)) {
        for (int i = (*stack).top; i >= 0; i--) {
            item_print((*stack).items[i]);
        }
    }
    else {
        printf("<EMPTY>\n");
    }
}
