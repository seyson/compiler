#ifndef PARSER_H
#define PARSER_H

#include "tokenizer.h"
#include "symtab.h"
/*
    A syntax analyzer (parser) for a subset of C.

    [✔] Expressions
    [✔] Operators
    [✔] Procedures
    [✔] Arrays
    [ ] Statements
    - [✔] declarations
    - [✔] assignments
    - [✔] case
    - [ ] goto 
    - [✔] if
    - [✔] while
    - [✔] do...while
    - [ ] for
    - [✔] print
*/

enum {
    op_push, op_fpush, op_pushi, op_fpushi, op_pop, op_fpop,
    op_neg, op_fneg, op_dup, op_exch, op_remove, op_reverse, 
    op_add, op_fadd, op_sub, op_fsub, op_mul, op_fmul, op_div, op_fdiv, op_mod,
    op_and, op_or, op_eq, op_neq, op_less, op_leq, op_greater, op_geq,
    op_fand, op_for, op_feq, op_fneq, op_fless, op_fleq, op_fgreater, op_fgeq,
    op_conv_to_float, op_conv_to_int,
    op_jmp, op_jfalse, op_jtrue, 
    op_call, op_return,
    op_put, op_fput, op_get, op_fget,
    op_printint, op_printfloat, op_printchar, op_println,
    op_halt,
};

char* sourcefile;

unsigned int ip; /* instruction pointer */
unsigned int dp; /* data pointer, or number of bytes to allocate to data array */
unsigned char* code; /* code array: a list of operations and their parameters */

// TokenTypes:  TK_ID, TK_INT, TK_REAL, TK_STR, TK_CHAR, TK_ARR, TK_FUNC,
typedef TokenType Type;

/* get the next token */
void gettoken(void);

/* compare the current token to a specified token and halt execution unless they match */
void match(TokenType);

/* print an error message about the current token, line number and column number; and exit */
void error(void);

/* Return the type of xRy, where the types of x, y are given and R, also given,
is one of the following relations:
+ - * / % || && == != < <= > >=
Note: +, -, * and / produce either reals or ints
(for example, real + int = real, whereas int + int = int);
the others always produce ints (e.g., a > b is either 1 or 0).
*/
Type combine(Type, Type, Type);

void begin_scope(void); /* push a symbol table onto stack */
void end_scope(void); /* pop a symbol table from stack */
void gen_op(unsigned char); /* write a specified operation (1 byte) to the code array */ 
void gen_addr(int); /* write a specified address (4 bytes) to the code array */
void gen_addr_rel(int, unsigned int); /* same as gen_addr, but input a location in the code array to write to */
void gen_int(int); /* write an int to the code array */
void gen_float(float); /* write a float to the code array */

void G(void); /* generates any number of procedure or variable declarations, i.e., the "translation unit" */
Type O(void); /* generates an or expression: || */
Type A(void); /* generates an and expression: && */
Type Q(void); /* generates an equality expression: == != */
Type R(void); /* generates a relational expression: > >= < <= */
Type E(void); /* generates an addition or subtraction expression: + - */
Type T(void); /* generates a modulo, multiplication or division expression: % * / */
/*
F ::= + F
    | - F
    | ( O )
    | <constant>
    | <identifier>
    | <identifier> [ <O> ]
*/
Type F(void);

/* <declaration> ::= <type-specifier> {<init-declarator>}* ;
where <type-specifier> ::= void | char | int | float */
Type declaration(void);

/* <init-declarator> ::= <declarator>
                       | <declarator> = <initializer>

Three calls to <init-declarator> would process 'x', 'y = 5' and 'z[2] = {9, 10}' in
the statement 'int x, y = 5, z[2] = {9, 10};' (equivalent to 'int x; int y = 5; int z[2] = {9, 10};').
Each call adds a new entry to the symbol table.

If called, <initializer> goes to the same entry in the symbol table and updates it with the initial value(s).
More importantly, it outputs an instruction to put the value (or values, for an array with more than 1 element)
that was computed on the stack in the address
in memory where the variable is stored.
*/
void init_declarator(int);

/* <declarator> ::= <identifier>
                  | ( <declarator> )
                  | <identifier> [ {<O>}? ] */
Node* declarator(int); /* return the newly inserted entry from the symtab */

/* <initializer> ::= <O>
                   | { <O-list> }

where

<O-list> ::= <O>
           | <O-list> , <O>

initializer() returns the number of initializations.
*/
int initializer(Node*);

/* 
we will forgo the idea of assignment expressions (which would allow x = y = z),
and just think of assignments as statements.

<assignment-statement> ::= {<assignment-list>}? ;
<assignment-list> ::= <assignment>
                    | <assignment-list> , <assignment>
<assignment> ::= <identifier> = <O>
               | <identifier> [ <O> ] = <O>
*/
void assignment(void);

/* <assignment-statement> ::= {<assignment-list>}? ; */
void assignment_statement(void);

/* <compound-statement> ::= { {<declaration-or-statement>}* } */
void compound_statement(void);

/* <statement> ::= <labeled-statement>
                 | <assignment-statement>
                 | <compound-statement>
                 | <selection-statement>
                 | <iteration-statement>
                 | <jump-statement> 
                 | <print-statement>
                 | <proc-call> */
void statement(void);

/* <labeled-statement> ::= <identifier> : <statement>
                         | case <O> : <statement>
                         | default : <statement> */
void labeled_statement(void);

/* <selection-statement> ::= if ( <O> ) <statement>
                           | if ( <O> ) <statement> else <statement>
                           | switch ( <O> ) <statement> */
void selection_statement(void);

void if_statement(void);
void switch_statement(void);
void switch_body(void);

/* <iteration-statement> ::= while ( <O> ) <statement>
                           | do <statement> while ( <O> ) ;
                           | for ( {<O>}? ; {<O>}? ; {<O>}? ) <statement> */
void iteration_statement(void);

void while_loop(void);
void do_while(void);

/* <jump-statement> ::= goto <identifier> ;
                      | continue ;
                      | break ;
                      | return {<O>}? ; */
void jump_statement(void);

void procedure_definition(void);

/* <procedure-call> ::= id ( <O-list> ) ; */
void procedure_call(void);

/* <print-statement> ::= print <O> ;
                       | print <string> ; */
void print_statement(void);

void conv(Type, Type);

void start(void);
void parse(void);

#endif