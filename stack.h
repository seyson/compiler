#ifndef STACK_H
#define STACK_H

#define INT     0
#define REAL    1
#define CHAR    2
#define STR     3

typedef struct item_t item_t;
typedef struct Stack Stack;

struct item_t {
    char type;
    union {
        int i;
        float f;
        char* s;
    };
};

struct Stack {
    item_t *items;
    int top; /* -1 if empty (otherwise between 0 and capacity-1) */
    int capacity;
};

/* Initialize a stack */
void stack_init(Stack*, int);

/* Remove all of a stack's items */
void stack_free(Stack*);

/* Test if a stack is empty */
int stack_is_empty(Stack*);

/* Test if a stack is full */
int stack_is_full(Stack*);

/* Push item onto stack */
void push(Stack*, item_t);

/* Remove top item from stack */
item_t pop(Stack*);

/* Print an item */
void item_print(item_t);

/* Print a stack */
void stack_print(Stack*);

#endif