#include "symtab.h"

#define TABLE_SIZE 27

int h(char* id)
{
    if (id[0] == '_') {
        return 26;
    }
    else {
        return tolower(id[0]) - 'a';
    }
}

List* create_table(void)
{
    List* table = malloc(TABLE_SIZE * sizeof (List));
    if (table == NULL) {
        printf("malloc() failed\n");
        exit(EXIT_FAILURE);
    }
    /* initialize the table's lists to NULL */
    for (int i = 0; i < TABLE_SIZE; i++) {
        table[i].head = NULL;
    }
    return table;
}

void free_table(List* table)
{
    for (int i = 0; i < TABLE_SIZE; i++) {
        Node* head = table[i].head;
        Node* copy;
        while (head != NULL) {
            copy = head;
            head = (*head).next;
            free(copy);
        }
    }
    free(table);
}

Node* search_id(List* table, char* id)
{
    /* get the appropriate list */
    List l = table[h(id)];
    /* search the list */
    Node* head = l.head;
    int result;
    while (head != NULL) {
        result = strcmp(id, (*head).name);
        if (result == 0) {
            return head;
        }
        else {
            head = (*head).next;
        }
    }
    return NULL;
}

Node* install_id(List* table, char* id)
{
    /* check if the id exists */
    Node* node = search_id(table, id);
    /* if it doesn't exist, insert it */
    if (node == NULL) {
        /* create a new node and put it in the front of the list */
        node = malloc(sizeof (Node));
        if (node == NULL) {
            printf("malloc() failed\n");
            exit(EXIT_FAILURE);
        }
        char* id_copy = malloc(strlen(id) + 1);
        (*node).name = strcpy(id_copy, id);
        (*node).next = table[h(id)].head;
        table[h(id)].head = node;
    }
    else {
        printf("Error: '%s' already declared\n", id);
        exit(EXIT_FAILURE);
    }
    return node;
}

void print_table(List* table)
{
    for (int i = 0; i < TABLE_SIZE; i++) {
        Node* head = table[i].head;
        char c = i + 'a';
        if (i == 26) {
            c = '_';
        }
        printf("%c -> ", c);
        while (head != NULL) {
            printf("%s -> ", (*head).name);
            head = (*head).next;
        }
        printf("\n");
    }
    printf("\n");
}

void push_symtab(List* table, SymbolTableStack* stack)
{
    if ((*stack).top >= STACK_SIZE) {
        printf("Too many symbol tables on the symbol table stack (max: 100)\n");
        exit(EXIT_FAILURE);
    }
    else {
        (*stack).table[++((*stack).top)] = table;
    }
}

void remove_symtab(SymbolTableStack* stack)
{
    free_table((*stack).table[(*stack).top--]);
}

Node* search_symtabs(SymbolTableStack* stack, char* id)
{
    int i = (*stack).top;
    Node* node = NULL;
    while (i >= 0 && node == NULL) {
        node = search_id((*stack).table[i], id);
        i--;
    }
    return node;
}

void print_stack(SymbolTableStack stack)
{
    for (int i = stack.top; i >= 0; i--) {
        print_table(stack.table[i]);
    }
}

