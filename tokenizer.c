#include "tokenizer.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

char *source_fp = "test_input.c";
char *buf;

const char *scanp; /* current scanning position */
int line = 1;      /* current line number */
int column = 0;    /* current column number */
int ch;            /* current character */

int curtype;              /* the main info: the current token's type */
unsigned int curval;      /* the current token's value (for numerical constants) */
size_t chars_scanned = 0; /* # of characters scanned so far */

const char* keywords[33] = {
    "auto",     "break",  "case",    "char",   "const",    "continue",
    "default",  "do",     "double",  "else",   "enum",     "extern",
    "float",    "for",    "goto",    "if",     "int",      "long",
    "print",
    "register", "return", "short",   "signed", "sizeof",   "static",
    "struct",   "switch", "typedef", "union",  "unsigned", "void",
    "volatile", "while"
};

int find_keyword(char *word)
{
    int mid, result;
    int first = 0;
    int last = 32;
    while (first <= last) {
        mid = (first + last) / 2;
        result = strcmp(word, keywords[mid]);
        if (result == 0) {
            return mid;
        }
        else if (result < 0) {
            last = mid - 1;
        }
        else {
            first = mid + 1;
        }
    }
    return -1;
}

char next_char(void)
{
    ch = *scanp++;
    if (ch == '\n') {
        column = 0;
        line++;
    }
    else {
        column++;
    }
    chars_scanned++;
    return ch;
}

void print_token(token t)
{
    if (t.type < 32) {
        printf("Token: (Keyword) %s\n", keywords[t.type]);
    }
    else if (t.type == TK_ID) {
        printf("Token: (Identifier) %s\n", t.text);
    }
    else if (t.type == TK_INT) {
        printf("Token: (Integer) %d\n", t.i);
    }
    else if (t.type == TK_REAL) {
        printf("Token: (Real) %f\n", t.f);
    }
    else if (t.type == TK_STR) {
        printf("Token: (String) \"%s\"\n", t.s);
    }
    else if (t.type == TK_CHAR) {
        printf("Token: (Character) '%c'\n", t.c);
    }
    else {
        switch (t.type) {
            case TK_LPAREN:
                printf("Token: (\n"); break;
            case TK_RPAREN:
                printf("Token: )\n"); break;
            case TK_LBRAC:
                printf("Token: [\n"); break;
            case TK_RBRAC:
                printf("Token: ]\n"); break;
            case TK_LCURL:
                printf("Token: {\n"); break;
            case TK_RCURL:
                printf("Token: }\n"); break;
            case TK_PERIOD:
                printf("Token: .\n"); break;
            case TK_COMMA:
                printf("Token: ,\n"); break;
            case TK_COLON:
                printf("Token: :\n"); break;
            case TK_SEMICOLON:
                printf("Token: ;\n"); break;
            case TK_INC:
                printf("Token: ++\n"); break;
            case TK_DEC:
                printf("Token: --\n"); break;
            case TK_PLUS:
                printf("Token: +\n"); break;
            case TK_MINUS:
                printf("Token: -\n"); break;
            case TK_MULT:
                printf("Token: *\n"); break;
            case TK_DIV:
                printf("Token: /\n"); break;
            case TK_MOD:
                printf("Token: %%\n"); break;
            case TK_ASSIGN:
                printf("Token: =\n"); break;
            case TK_PLUS_ASSIGN:
                printf("Token: +=\n"); break;
            case TK_MINUS_ASSIGN:
                printf("Token: -=\n"); break;
            case TK_MULT_ASSIGN:
                printf("Token: *=\n"); break;
            case TK_DIV_ASSIGN:
                printf("Token: /=\n"); break;
            case TK_MOD_ASSIGN:
                printf("Token: %%=\n"); break;
            case TK_EQ:
                printf("Token: ==\n"); break;
            case TK_NEQ:
                printf("Token: !=\n"); break;
            case TK_LESS:
                printf("Token: <\n"); break;
            case TK_GREATER:
                printf("Token: >\n"); break;
            case TK_LESS_EQ:
                printf("Token: <=\n"); break;
            case TK_GREATER_EQ:
                printf("Token: >=\n"); break;
            case TK_NOT:
                printf("Token: !\n"); break;
            case TK_QUESTION:
                printf("Token: ?\n"); break;
            case TK_AND:
                printf("Token: &&\n"); break;
            case TK_OR:
                printf("Token: ||\n"); break;
            case TK_BIT_AND:
                printf("Token: &\n"); break;
            case TK_BIT_OR:
                printf("Token: |\n"); break;
            case TK_BIT_XOR:
                printf("Token: ^\n"); break;
            case TK_BIT_NOT:
                printf("Token: ~\n"); break;
            case TK_LEFTSHIFT:
                printf("Token: <<\n"); break;
            case TK_RIGHTSHIFT:
                printf("Token: >>\n"); break;
            case TK_LEFTSHIFT_ASSIGN:
                printf("Token: <<=\n"); break;
            case TK_RIGHTSHIFT_ASSIGN:
                printf("Token: >>=\n"); break;
            case TK_BIT_AND_ASSIGN:
                printf("Token: &=\n"); break;
            case TK_BIT_OR_ASSIGN:
                printf("Token: |=\n"); break;
            case TK_BIT_XOR_ASSIGN:
                printf("Token: ^=\n"); break;
            case TK_EOL:
                printf("Token: End of line\n"); break;
            case TK_EOF:
                printf("Token: End of file\n"); break;
            case TK_BADCHAR:
                printf("Token: Bad char (e.g. `, @, $)\n"); break;
        }
    }
}

void print_tokens(token* toks)
{
    int i = 0;
    while (toks[i].type != TK_EOF) {
        print_token(toks[i]);
        i++;
    }
    print_token(toks[i]);
}

token next_token(void)
{
    token t;
    /* skip any white space (except for \n which is returned) */
    while (isspace(ch)) {
        if ('\n' == ch) {
            curtype = TK_EOL;
            t = (token) {TK_EOL, line, column};
            next_char();
            return t;
        }
        next_char();
    }
    int start_line = line;
    int start_column = column;
    /* read a number or a dot */
    if ('.' == ch || '0' <= ch && ch <= '9') {
        curval = 0;
        float f;
        float fact = 1.0;
        /* process any numbers in front (before a period) */
        while ('0' <= ch && ch <= '9') { 
            curval = (ch - '0') + curval * 10;
            next_char();
        }
        if ('.' == ch) {
            f = curval;
            /* test if we should return a dot token instead of a number */
            int dot_line = line;
            int dot_column = column;
            next_char();
            if (curval == 0 && isalpha(ch)) {
                t = (token) {TK_PERIOD, dot_line, dot_column};
                return t; /* return dot token */
            }

            while ('0' <= ch && ch <= '9') {
                fact /= 10.0;
                f = f + (float) (ch - '0') * fact;
                next_char();
            }
            curtype = TK_REAL;
        }
        else {
            curtype = TK_INT;
        }
        
        /* check for proper termination character: either white space or punctuation */
        if (isspace(ch) || ispunct(ch)) {
            t = (token) {curtype, start_line, start_column};
            if (curtype == TK_INT) {
                t.i = curval;
            }
            else {
                t.f = f;
            }
        }
        else {
            /* (ch must be alphabetic) */
            printf("Error: invalid suffix in integer constant at line %d, column %d\n", start_line, start_column);
            exit(EXIT_FAILURE);
        }
        return t; /* return the number */
    }
    /* read a string */
    if ('"' == ch) {
        curtype = TK_STR;
        next_char();
        int d = 0;
        while ('\0' != ch) {
            if (ch == '"') {
                char *string_text = malloc(d+1);
                if (!string_text) {
                    printf("malloc() failed\n");
                    exit(EXIT_FAILURE);
                }
                const char *temp = scanp;
                temp--;
                for (int i = 0; i < d; i++) {
                    string_text[d-1-i] = *(--temp);
                }
                string_text[d] = '\0';
                t = (token) {TK_STR, start_line, start_column};
                t.s = string_text;
                next_char();
                return t;
            }
            else {
                d++;
                next_char();
            }
        }
        printf("Error: missing terminating character: line %d, column %d\n", start_line, start_column);
    }
    /* read a character */
    if ('\'' == ch) {
        curtype = TK_CHAR;
        char c = next_char();
        if ('\'' == c) {
            printf("Error: empty character at line %d, column %d\n", start_line, start_column);
            exit(EXIT_FAILURE);
        }
        next_char();
        if ('\'' == ch) {
            t = (token) {TK_CHAR, start_line, start_column};
            t.c = c;
            next_char();
        }
        else {
            /* search for terminating ' */
            while ('\0' != ch) {
                if ('\'' == ch) {
                    printf("Error: multi-character character at line %d, column %d\n", start_line, start_column);
                    exit(EXIT_FAILURE);
                }
                next_char();
            }
            printf("Error: missing terminating ' character: line %d, column %d\n", start_line, start_column);
            exit(EXIT_FAILURE);
        }
        return t; /* return the character */
    }
    /* read an identifier/keyword */
    if (('a' <= ch && ch <= 'z') || ('A' <= ch && ch <= 'Z') || '_' == ch) {
        t.line = line;
        t.column = column;

        int count = 0;
        while (('a' <= ch && ch <= 'z') || ('A' <= ch && ch <= 'Z') ||
               ('0' <= ch && ch <= '9') || '_' == ch) {
            t.text[count] = ch;
            count++;
            if (count > 31) {
                printf("Error: identifier over 31 chars long at line %d, column %d\n", start_line, start_column);
                exit(EXIT_FAILURE);
            }
            next_char();
        }
        t.text[count] = '\0'; /* terminate the text with a null char */
        int k = find_keyword(t.text); /* check if it's a keyword */
        if (k != -1) {
            curtype = k;
            t.type = curtype;
        }
        else {
            curtype = TK_ID;
            t.type = curtype;
        }
        /* the current char can't be alphanumeric, so it must be a proper termination
        character (whitespace or punctuation) */
        return t; /* return the identifier/keyword */
    }
    /* read everything else */
    switch (ch) {
        case '/':
            /* could be / or /= or // or /* */
            next_char();
            if ('=' == ch) {
                t = (token) {TK_DIV_ASSIGN, start_line, start_column};
                next_char();
            }
            else if ('/' == ch) {
                /* ignore the comment: skip until EOF and EOL */
                while ('\0' != ch && '\n' != ch) {
                    next_char();
                }
                return next_token();
            }
            else if ('*' == ch) {
                /* ignore the comment: skip until a terminating * and / */
                while ('\0' != ch) {
                    if ('*' == ch) {
                        next_char();
                        if ('/' == ch) {
                            next_char();
                            return next_token();
                        }
                    }
                    else {
                        next_char();
                    }
                }
                printf("Unterminated comment at line %d, column %d.\n", start_line, start_column);
                exit(EXIT_FAILURE);
            }
            else {
                t = (token) {TK_DIV, start_line, start_column};
            }
            break;
        case ';':
            t = (token) {TK_SEMICOLON, line, column};
            next_char();
            break;
        case ',':
            t = (token) {TK_COMMA, line, column};
            next_char();
            break;
        case ':':
            t = (token) {TK_COLON, line, column};
            next_char();
            break;
        case '(':
            t = (token) {TK_LPAREN, line, column};
            next_char();
            break;
        case ')':
            t = (token) {TK_RPAREN, line, column};
            next_char();
            break;
        case '{':
            t = (token) {TK_LCURL, line, column};
            next_char();
            break;
        case '}':
            t = (token) {TK_RCURL, line, column};
            next_char();
            break;
        case '[':
            t = (token) {TK_LBRAC, line, column};
            next_char();
            break;
        case ']':
            t = (token) {TK_RBRAC, line, column};
            next_char();
            break;
        case '=':
            /* could be = or == */
            next_char();
            if ('=' == ch) {
                t = (token) {TK_EQ, start_line, start_column};
                next_char();
            }
            else {
                t = (token) {TK_ASSIGN, start_line, start_column};
            }
            break;
        case '<':
            /* could be < or <= or << or <<= */
            next_char();
            if ('=' == ch) {
                t = (token) {TK_LESS_EQ, start_line, start_column};
                next_char();
            }
            else if ('<' == ch) {
                /* either << or <<= */
                next_char();
                if ('=' == ch) {
                    t = (token) {TK_LEFTSHIFT_ASSIGN, start_line, start_column};
                    next_char();
                }
                else {
                    t = (token) {TK_LEFTSHIFT, start_line, start_column};
                }
            }
            else {
                t = (token) {TK_LESS, start_line, start_column};
            }
            break;
        case '>':
            /* could be > or >= or >> or >>= */
            next_char();
            if ('=' == ch) {
                t = (token) {TK_GREATER_EQ, start_line, start_column};
                next_char();
            }
            else if ('>' == ch) {
                /* either >> or >>= */
                next_char();
                if ('=' == ch) {
                    t = (token) {TK_RIGHTSHIFT_ASSIGN, start_line, start_column};
                    next_char();
                }
                else {
                    t = (token) {TK_RIGHTSHIFT, start_line, start_column};
                }
            }
            else {
                t = (token) {TK_GREATER, start_line, start_column};
            }
            break;
        case '+':
            /* could be + or ++ or += */
            next_char();
            if ('+' == ch) {
                t = (token) {TK_INC, start_line, start_column};
                next_char();
            }
            else if ('=' == ch) {
                t = (token) {TK_PLUS_ASSIGN, start_line, start_column};
                next_char();
            }
            else {
                t = (token) {TK_PLUS, start_line, start_column};
            }
            break;
        case '-':
            /* could be - or -- or -= */
            next_char();
            if ('-' == ch) {
                t = (token) {TK_DEC, start_line, start_column};
                next_char();
            }
            else if ('=' == ch) {
                t = (token) {TK_MINUS_ASSIGN, start_line, start_column};
                next_char();
            }
            else {
                t = (token) {TK_MINUS, start_line, start_column};
            }
            break;
        case '*':
            /* could be * or *= */
            next_char();
            if ('=' == ch) {
                t = (token) {TK_MULT_ASSIGN, start_line, start_column};
                next_char();
            }
            else {
                t = (token) {TK_MULT, start_line, start_column};
            }
            break;
        case '%':
            /* could be % or %= */
            next_char();
            if ('=' == ch) {
                t = (token) {TK_MOD_ASSIGN, start_line, start_column};
                next_char();
            }
            else {
                t = (token) {TK_MOD, start_line, start_column};
            }
            break;
        case '|':
            /* could be | or || or |= */
            next_char();
            if ('|' == ch) {
                t = (token) {TK_OR, start_line, start_column};
                next_char();
            }
            else if ('=' == ch) {
                t = (token) {TK_BIT_OR_ASSIGN, start_line, start_column};
                next_char();
            }
            else {
                t = (token) {TK_BIT_OR, start_line, start_column};
            }
            break;
        case '^':
            /* could be ^ or ^= */
            next_char();
            if ('=' == ch) {
                t = (token) {TK_BIT_XOR_ASSIGN, start_line, start_column};
                next_char();
            }
            else {
                t = (token) {TK_BIT_XOR, start_line, start_column};
            }
            break;
        case '~':
            t = (token) {TK_BIT_NOT, line, column};
            next_char();
            break;
        case '&':
            /* could be & or && or &= */
            next_char();
            if ('&' == ch) {
                t = (token) {TK_AND, start_line, start_column};
                next_char();
            }
            else if ('=' == ch) {
                t = (token) {TK_BIT_AND_ASSIGN, start_line, start_column};
                next_char();
            }
            else {
                t = (token) {TK_BIT_AND, start_line, start_column};
            }
            break;
        case '!':
            /* could be ! or != */
            next_char();
            if ('=' == ch) {
                t = (token) {TK_NEQ, start_line, start_column};
                next_char();
            }
            else {
                t = (token) {TK_NOT, start_line, start_column};
            }
            break;
        case '?':
            t = (token) {TK_QUESTION, line, column};
            next_char();
            break;
        case '\0':
            t = (token) {TK_EOF, line, column};            
            break;
        default:
            printf("Error: bad char %c at line %d, column %d.\n", ch, line, column);
            exit(EXIT_FAILURE);
    }
    curtype = t.type;
    return t;
}

FILE* read_file(char *source)
{
    source_fp = source;
    /* open the source file and read its contents into a buffer */
    FILE *f = fopen(source_fp, "rb");
    if (f == NULL) {
        printf("fopen() failed (does the file exist?)\n");
        exit(EXIT_FAILURE);
    }
    /* computing the file size */
    fseek(f, 0, SEEK_END);
    long file_size = ftell(f);
    fseek(f, 0, SEEK_SET); // or rewind(f);
    /* creating and writing to a buffer */
    buf = malloc(file_size + 1);
    fread(buf, file_size, 1, f);
    fclose(f);
    buf[file_size] = '\0';
    /* set the pointer to the beginning of the buffer */
    scanp = buf;
    return f;
}

token* scan(void)
{
    /* start scanning */
    next_char();
    int count = 0;
    size_t toks_size = 50000;
    token* toks = malloc(toks_size * sizeof (token));
    token tok;
    while (curtype != TK_EOF) {
        tok = next_token();
        //print_token(tok); /* print the token */
        toks[count] = tok; /* store the token in an array */
        count++;
        if (count > toks_size) {    /* reallocate if necessary */
            toks_size += 500;
            token* temp = realloc(toks, toks_size * sizeof (token));
            if (temp == NULL) {
                printf("realloc() failed\n");
                exit(EXIT_FAILURE);
            }
            else {
                toks = temp;
            }
        }
    }
    free(buf);
    return toks;
}
