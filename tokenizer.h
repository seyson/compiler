#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
/*
A lexical analyzer for C.

1. Keywords are just keywords.
2. Identifiers are a letter or an underscore followed by any combination of letters, digits and underscores.
   (constraints: not keywords, and only up to 31 characters long)
3. Numbers are digits including an optional period in any position amongst those digits.
   (exceptions: hexadecimals and exponential numbers, which have an 'x' and an 'E', respectively)
   (constraints: hexadecimal and octal numbers don't have a fractional part -- if a period occurs in the structure 
    of a hexadecimal number, it's a mistake, but if a period occurs within an octal number, it gets evaluated as
    if it were a decimal number)
4. String literals are any characters except " surrounded by "'s.
   (Strings cannot contain a new line character unless the new line character is preceded by '\'.)
5. Operators are symbols like -, ->, <, <=, ==, and !=.
6. Punctuators include left and right brackets, comma, and semicolon.
7. Comments are characters inside // and a new line or inside /* and * /
EOL
EOF is a distinct character that marks the end of the source file.

An identifier begins with a letter or underscore followed by zero or more letters, numbers and underscores
and is terminated by whitespace or punctuation.

Integers begin with a digit, and the starting characters determine the type:
0 means octal,
0x or 0X means hexadecimal,
1-9 means decimal.
Numbers are terminated by whitespace or punctuation.

What wasn't included:
- hexadecimal and exponential numbers
- numerical types other than (signed) int and float
- restrictions on the size of numbers/strings (however, an error is thrown if identifiers are > 31 chars long)
(i.e., it is assumed that the source program will not declare any numbers that won't compile)
- escape characters in strings or chars, or line breaks: e.g., "hello\nworld" and '\n'
(i.e., it is assumed that the source program will not escape or line-break strings)
- contiguous strings aren't merged into a single string (e.g., "a" "b" doesn't get processed into "ab")
- pound symbols (#include and #define statements)
- the operator -> (the dot operator is already included)
- (there are probably more that I haven't mentioned)

*/

typedef enum {
    TK_auto, TK_break, TK_case, TK_char, TK_const, TK_continue, TK_default, TK_do,
    TK_double, TK_else, TK_enum, TK_extern, TK_float, TK_for, TK_goto, TK_if, TK_int,
    TK_long, TK_print, TK_register, TK_return, TK_short, TK_signed, TK_sizeof, TK_static,
    TK_struct, TK_switch, TK_typedef, TK_union, TK_unsigned, TK_void, TK_volatile, TK_while,

    TK_ID, TK_INT, TK_REAL, TK_STR, TK_CHAR,
    TK_ARR, TK_FUNC,

    TK_LPAREN, TK_RPAREN, TK_LBRAC, TK_RBRAC, TK_LCURL, TK_RCURL,                                       /*  ( ) [ ] { }       */
    TK_PERIOD, TK_COMMA, TK_COLON, TK_SEMICOLON, TK_INC, TK_DEC,                                        /*  , : ; ++ --       */
    TK_PLUS, TK_MINUS, TK_MULT, TK_DIV, TK_MOD,                                                         /*  + - * / %         */
    TK_ASSIGN, TK_PLUS_ASSIGN, TK_MINUS_ASSIGN, TK_MULT_ASSIGN, TK_DIV_ASSIGN, TK_MOD_ASSIGN,           /*  = += -= *= /= %=  */
    TK_EQ, TK_NEQ, TK_LESS, TK_GREATER, TK_LESS_EQ, TK_GREATER_EQ,                                      /*  == != < > <= >=   */
    TK_NOT, TK_QUESTION, TK_AND, TK_OR,                                                                 /*  ! ? && ||         */
    TK_BIT_AND, TK_BIT_OR, TK_BIT_XOR, TK_BIT_NOT, TK_LEFTSHIFT, TK_RIGHTSHIFT,                         /*  & | ^ ~ << >>     */
    TK_LEFTSHIFT_ASSIGN, TK_RIGHTSHIFT_ASSIGN, TK_BIT_AND_ASSIGN, TK_BIT_OR_ASSIGN, TK_BIT_XOR_ASSIGN,  /* <<= >>= &= |= ^=   */

    TK_EOL, TK_EOF, TK_BADCHAR /* bad chars: ` @ $  */
} TokenType;

typedef struct {
    TokenType type;
    unsigned int line;
    unsigned int column;
    union {
        int i;             /* value for integer constants */
        float f;          /* value for floating-point constants */
        char text[32];     /* text for identifiers */
        char c;            /* characters */
        char* s;           /* strings */
    };
} token;

/* return the index of the keyword in the keyword array or -1 if it doesn't exist */
int find_keyword(char*);

/* get the next character in the buffer */
char next_char(void);

/* print info about a given token (for testing purposes) */
void print_token(token);

/* Print complete output, i.e., an array of tokens (again, for testing purposes) */
void print_tokens(token*);

/* Return the next token which starts from the current scanning position */
token next_token(void);

FILE* read_file(char*);

token* scan(void);

#endif