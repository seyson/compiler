#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stack.h"
#include "parser.h"

/*
(1) Code array (byte array): written by the compiler and then executed during runtime
(2) Data array (byte array): compiler only allocates it (for reading/writing during runtime)
(3) Stack: not used by compiler, but the compiler can estimate how much the stack needs (again, runtime only)

e.g., int x, y; // x gets addr 0, y gets addr 4, and thus the data array should get 8 bytes.

Byte arrays are used to accommodate different types.

An address is 4 bytes.
An op is 1 byte.

e.g., 
int x, y, z; // x gets address 0, y gets address 4, z gets address 8
x = y;
| push (1 byte) | @y (4 bytes) | pop (1 byte) | @x (4 bytes)  <-- code array
(push y; pop; assign the popped value to x)

e.g.,
x = y + z;
| push | @y | push | @z | add | pop | @x  <-- code array
(push y; push z; add; pop the result of the add; assign that value to x)
*/

Stack stack;
unsigned char* data;

/* Store an int in the first four bytes of a byte array */
void store_int(unsigned char* bytes, int n)
{
    bytes[0] = (n >> 24) & 0xFF;
    bytes[1] = (n >> 16) & 0xFF;
    bytes[2] = (n >> 8)  & 0xFF;
    bytes[3] = n         & 0xFF;
}

/* Read an int from the first four bytes of a byte array */
int get_int(unsigned char* bytes)
{
    return (bytes[0] << 24) + (bytes[1] << 16) + (bytes[2] << 8) + bytes[3];
}

/* Store a float in the first four bytes of a byte array */
char* store_float(unsigned char* bytes, float f)
{
    for (int i = 0; i < 4; i++) {
        bytes[i] = *((unsigned char*) &f + i);
    }
}

/* Read a float from the first four bytes of a byte array */
float get_float(unsigned char* bytes)
{
    float f;
    for (int i = 0; i < 4; i++) {
        *((unsigned char*) &f + i) = bytes[i];
    }
    return f;
}

/* reverse n items on the stack */
void reverse(int n)
{
    item_t temp;
    int end = stack.top;
    int start = stack.top - n + 1;
    while (start < end)
    {
        temp = stack.items[start]; /* save the original first elt */
        stack.items[start] = stack.items[end]; /* move the last elt to the start */
        stack.items[end] = temp; /* move the original first elt to the end */
        start++;
        end--;
    }
}

void main(int argc, char* argv[])
{
    if (argc == 1) {
        sourcefile = "test_input.c";
    }
    else if (argc == 2) {
        sourcefile = argv[1];
    }
    else {
        fprintf(stderr, "usage: <program> <input file>\n");
        exit(EXIT_FAILURE);
    }

    parse();
    printf("\n");
    printf("ip: %i, dp: %i\n\n", ip, dp);
    data = malloc(dp);
    if (data == NULL) {
        printf("malloc() failed for data array\n");
        exit(EXIT_FAILURE);
    }
    ip = 0;
    dp = 0;
    stack_init(&stack, 400); /* initialize stack */
    printf("\n;;;;;;;;;;;;;; BEGINNING OF OUTPUT ;;;;;;;;;;;;;;;;\n");
    while (1) {
/*        printf("--------------- ip: %i -----------\n" , ip);*/
/*        stack_print(&stack);*/
        switch(code[ip++]) {
            /* op_push is used for both ints (4 bytes) and chars (also 4 bytes) */
            case op_push: { /* input a 4-byte address and push the integer value in that address to stack */
                dp = get_int(code+ip); /* get the address of the variable */
                ip += 4;
                int val = get_int(data+dp); /* get the value from the data array */
                /* push the value onto the stack */
                item_t stack_item;
                stack_item.i = val;
                stack_item.type = INT;
                push(&stack, stack_item);
                break;
            }
            case op_fpush: {
                dp = get_int(code+ip); /* get the address of the variable */
                ip += 4;
                float val = get_float(data+dp); /* get the value from the data array */
                /* push the value onto the stack */
                item_t stack_item;
                stack_item.f = val;
                stack_item.type = REAL;
                push(&stack, stack_item);
                break;
            }
            case op_pushi: {
                int val = get_int(code+ip); /* get the value directly from the code array */
                ip += 4;
                /* push the value onto the stack */
                item_t stack_item;
                stack_item.i = val;
                stack_item.type = INT;
                push(&stack, stack_item);
                break;
            }
            case op_fpushi: {
                float val = get_float(code+ip);
                ip += 4;
                item_t stack_item;
                stack_item.f = val;
                stack_item.type = REAL;
                push(&stack, stack_item);
                break;
            }
            case op_pop: { /* pop <arg> assigns the top value to the address given by <arg> */
                item_t popped_item = pop(&stack); 
                dp = get_int(code+ip); /* get the address in which to store the value */
                ip += 4;
                store_int(data+dp, popped_item.i); /* store assignment (an integer) */
                break;
            }
            case op_fpop: {
                item_t popped_item = pop(&stack);
                dp = get_int(code+ip);
                ip += 4;
                store_float(data+dp, popped_item.f); /* store assignment (a float) */
                break;
            }
            case op_exch: {
                item_t item1 = pop(&stack);
                item_t item2 = pop(&stack);
                push(&stack, item1);
                push(&stack, item2);
                break;
            }
            case op_remove: {
                pop(&stack);
                break;
            }
            case op_dup: {
                item_t popped_item = pop(&stack);
                push(&stack, popped_item);
                push(&stack, popped_item);
                break;
            }
            case op_add: {
                item_t item1 = pop(&stack);
                item_t item2 = pop(&stack);
                item_t item3;
                item3.i = item2.i + item1.i;
                item3.type = INT;
                push(&stack, item3);
                break;
            }
            case op_fadd: {
                item_t item1 = pop(&stack);
                item_t item2 = pop(&stack);
                item_t item3;
                item3.f = item2.f + item1.f;
                item3.type = REAL;
                push(&stack, item3);
                break;
            }
            case op_sub: {
                item_t item1 = pop(&stack);
                item_t item2 = pop(&stack);
                item_t item3;
                item3.i = item2.i - item1.i;
                item3.type = INT;
                push(&stack, item3);
                break;
            }
            case op_fsub: {
                item_t item1 = pop(&stack);
                item_t item2 = pop(&stack);
                item_t item3;
                item3.f = item2.f - item1.f;
                item3.type = REAL;
                push(&stack, item3);
                break;
            }
            case op_mul: {
                item_t item1 = pop(&stack);
                item_t item2 = pop(&stack);
                item_t item3;
                item3.i = item2.i * item1.i;
                item3.type = INT;
                push(&stack, item3);
                break;
            }
            case op_fmul: {
                item_t item1 = pop(&stack);
                item_t item2 = pop(&stack);
                item_t item3;
                item3.f = item2.f * item1.f;
                item3.type = REAL;
                push(&stack, item3);
                break;
            }
            case op_div: {
                item_t item1 = pop(&stack);
                item_t item2 = pop(&stack);
                item_t item3;
                /* check for division by zero */
                if (item1.i == 0) {
                    printf("Error: division by zero\n");
                    exit(EXIT_FAILURE);
                }
                item3.i = item2.i / item1.i;
                item3.type = INT;
                push(&stack, item3);
                break;
            }
            case op_fdiv: {
                item_t item1 = pop(&stack);
                item_t item2 = pop(&stack);
                item_t item3;
                if (item1.f == 0) {
                    printf("Error: division by zero\n");
                    exit(EXIT_FAILURE);
                }
                item3.f = item2.f / item1.f;
                item3.type = REAL;
                push(&stack, item3);
                break;
            }
            case op_mod: {
                item_t item1 = pop(&stack);
                item_t item2 = pop(&stack);
                item_t item3;
                item3.i = item2.i % item1.i;
                item3.type = INT;
                push(&stack, item3);
                break;
            }
            case op_neg: {
                item_t item = pop(&stack);
                item.i = -item.i;
                push(&stack, item);
                break;
            }
            case op_fneg: {
                item_t item = pop(&stack);
                item.f = -item.f;
                push(&stack, item);
                break;
            }
            case op_conv_to_float: {
                item_t item = pop(&stack);
                if (item.type == INT || item.type == CHAR) {
                    item.f = item.i;
                    item.type = REAL;
                }
                push(&stack, item);
                break;
            }
            case op_conv_to_int: {
                item_t item = pop(&stack);
                if (item.type == REAL) {
                    item.i = item.f;
                    item.type = INT;
                }
                push(&stack, item);
                break;
            }
            case op_jmp: {
                /* current ip: one past the actual jmp instruction */
                int start = ip-1;
                int offset = get_int(code+ip);  //printf("jmp offset: %i\n", offset);
                ip += offset-1;
                break;
            }
            case op_jtrue: {
                /* current ip: one past the actual jmp instruction */
                int start = ip-1;
                int offset = get_int(code+ip); //printf("jtrue offset: %i\n", offset);
                /* an item is on the stack that needs to be evaluated */
                item_t item = pop(&stack);
                if (item.f) {
                    ip += offset-1; 
                }
                else {
                    ip += 4;
                }
                break;
            }
            case op_jfalse: {
                /* current ip: one past the actual jmp instruction */
                int start = ip-1;
                int offset = get_int(code+ip); //printf("jfalse offset: %i\n", offset);
                /* an item is on the stack that needs to be evaluated */
                item_t item = pop(&stack);
                if (!item.f) {
                    ip += offset-1; 
                }
                else {
                    ip += 4;
                }
                break;
            }
            case op_and: {
                item_t item1 = pop(&stack);
                item_t item2 = pop(&stack);
                item_t item3;
                item3.i = item2.f && item1.f;
                item3.type = INT;
                push(&stack, item3);
                break;
            }
            case op_or: {
                item_t item1 = pop(&stack);
                item_t item2 = pop(&stack);
                item_t item3;
                item3.i = item2.f || item1.f;
                item3.type = INT;
                push(&stack, item3);
                break;
            }
            case op_eq: {
                item_t item1 = pop(&stack);
                item_t item2 = pop(&stack);
                item_t item3;
                item3.i = item2.f == item1.f;
                item3.type = INT;
                push(&stack, item3);
                break;
            }
            case op_neq: {
                item_t item1 = pop(&stack);
                item_t item2 = pop(&stack);
                item_t item3;
                item3.i = item2.f != item1.f;
                item3.type = INT;
                push(&stack, item3);
                break;
            }
            case op_less: {
                item_t item1 = pop(&stack);
                item_t item2 = pop(&stack);
                item_t item3;
                item3.i = item2.f < item1.f;
                item3.type = INT;
                push(&stack, item3);
                break;
            }
            case op_leq: {
                item_t item1 = pop(&stack);
                item_t item2 = pop(&stack);
                item_t item3;
                item3.i = item2.f <= item1.f;
                item3.type = INT;
                push(&stack, item3);
                break;
            }
            case op_greater: {
                item_t item1 = pop(&stack);
                item_t item2 = pop(&stack);
                item_t item3;
                item3.i = item2.f > item1.f;
                item3.type = INT;
                push(&stack, item3);
                break;
            }
            case op_geq: {
                item_t item1 = pop(&stack);
                item_t item2 = pop(&stack);
                item_t item3;
                item3.i = item2.f >= item1.f;
                item3.type = INT;
                push(&stack, item3);
                break;
            }
            case op_printint: {
                item_t popped_item = pop(&stack);
                printf("%i", popped_item.i);
                break;
            }
            case op_printfloat: {
                item_t popped_item = pop(&stack);
                printf("%f", popped_item.f);
                break;
            }
            case op_printchar: {
                item_t popped_item = pop(&stack);
                printf("%c", popped_item.i);
                break;
            }
            case op_println: {
                printf("\n");
                break;
            }
            case op_reverse: { /* reverse top n items on stack */
                int n = get_int(code+ip);
                ip += 4;
                reverse(n);
                break;
            }
            case op_call: { /* takes one arg: the procedure address */
                /* get instruction to top of procedure instructions */
                int proc_ip = get_int(code+ip);
                ip += 4;
/*                item_t return_addr;*/
/*                return_addr.i = ip;*/
/*                push(&stack, return_addr);*/
                /* start executing the procedure */
                ip = proc_ip; 
                break;
            }
            case op_return: { /* takes one arg: the return address */
                /* pop the return addr from stack */ 
                item_t return_addr = pop(&stack);
                ip = return_addr.i;
                break;
            }
            case op_put: { /* pop a value and addr from stack; assign a value to the array elt at the addr */
                item_t assignval = pop(&stack);
                item_t addr = pop(&stack);
                store_int(data + addr.i, assignval.i);
                break;
            }
            case op_fput: {
                item_t assignval = pop(&stack);
                item_t addr = pop(&stack);
                store_float(data + addr.i, assignval.f);
                break;
            }
            case op_get: { /* pop an array elt's addr from stack and leave its value */
                item_t popped = pop(&stack);
                item_t val;
                val.i = get_int(data+popped.i);
                val.type = INT;
                push(&stack, val);
                break;
            }
            case op_fget: {
                item_t popped = pop(&stack);
                item_t val;
                val.f = get_float(data+popped.i);
                val.type = REAL;
                push(&stack, val);
                break;
            }
            case op_halt:
                printf(";;;;;;;;;;;;;;;;; END OF OUTPUT ;;;;;;;;;;;;;;;;;;;\n");
                exit(0);
        }
    }

    free(code);
    free(data);
    stack_free(&stack);
}

