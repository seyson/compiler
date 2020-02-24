#include "parser.h"
#include "tokenizer.h"
#include "symtab.h"

unsigned int ip = 0; /* instruction pointer */
unsigned int dp = 0; /* data pointer, or number of bytes to allocate to data array */
token* toks;                 /* list of tokens from tokenizer */
token curtoken;              /* current token being processed */
SymbolTableStack symtabs;    /* a stack of symbol tables to work with */
int inside_switch = 0;       /* indicates whether case and default labels may be used */

void gen_op(unsigned char op)
{
    code[ip++] = op;
}

void gen_addr(int addr)
{
    code[ip++] = (addr >> 24) & 0xFF;
    code[ip++] = (addr >> 16) & 0xFF;
    code[ip++] = (addr >> 8)  & 0xFF;
    code[ip++] = addr         & 0xFF;
}

void gen_addr_rel(int addr, unsigned int loc)
{
    code[loc++] = (addr >> 24) & 0xFF;
    code[loc++] = (addr >> 16) & 0xFF;
    code[loc++] = (addr >> 8)  & 0xFF;
    code[loc++] = addr         & 0xFF;

}

void gen_int(int i)
{
    gen_addr(i);
}

void gen_float(float f)
{
    for (int i = 0; i < 4; i++) {
        code[ip++] = *((unsigned char*) &f + i);
    }
}

/* Get the next token (update curtoken) */
void gettoken(void)
{
    toks++;
    /* skip EOL */
    while (toks[0].type == TK_EOL) {
        toks++;
    }
    curtoken = toks[0];
}

/* Matching the current token to an expected token, halting if there is no match */
void match(TokenType t) 
{
    /* skip EOL */
    if (curtoken.type == TK_EOL) {
        gettoken();
    }
    if (t != curtoken.type) {
        error(); /* print an error message */
        exit(EXIT_FAILURE);
    }
    else {
        gettoken();
    }
}

void error(void)
{
    printf("Unexpected token at line %d, column %d:\n -> ", curtoken.line, curtoken.column);
    print_token(curtoken);
    exit(EXIT_FAILURE);
}

Type combine(Type type1, Type type2, Type op)
{
    if (op == TK_PLUS || op == TK_MINUS || op == TK_MULT || op == TK_DIV) {
        if (type1 == TK_INT || type1 == TK_CHAR) {
            if (type2 == TK_INT || type2 == TK_CHAR) {
                //printf("I (+-*/) I\n");
                return TK_INT;
            }
            else if (type2 == TK_REAL) {
                printf("I (+-*/) R -> (i) exchange, (ii) conv to real, (iii) exchange\n");
                gen_op(op_exch);
                gen_op(op_conv_to_float);
                gen_op(op_exch);
                return TK_REAL;
            }
            else {
                printf("type mismatch on line %i in one of these operators: + - * /\n", curtoken.line);
                exit(EXIT_FAILURE);
            }
        }
        else if (type1 == TK_REAL) {
            if (type2 == TK_REAL) {
                //printf("R (+-*/) R\n");
                return TK_REAL;
            }
            else if (type2 == TK_INT || type2 == TK_CHAR) {
                printf("R (+-*/) I -> (i) conv to real\n");
                gen_op(op_conv_to_float);
                return TK_REAL;
            }
            else {
                printf("type mismatch on line %i in one of these operators: + - * /\n", curtoken.line);
                exit(EXIT_FAILURE);
            }
        }
        else {
            printf("type mismatch on line %i in one of these operators: + - * /\n", curtoken.line);
            exit(EXIT_FAILURE);
        }
    }
    else if (op == TK_MOD) {
        if (type1 != TK_INT || type2 != TK_INT) {
            printf("type mismatch on line %i in operator %%\n", curtoken.line);
            exit(EXIT_FAILURE);
        }
    }
    else {
        /*  %, ||, &&, ==, !=, <, <=, >, >=   */
        if (type1 == TK_INT || type1 == TK_CHAR) {
            if (type2 == TK_INT || type2 == TK_CHAR) {
                printf("I (||&&==!=<<=>>=) I -> (i) conv to real, (ii) exchange, (iii) conv to real, (iv) exchange\n");
                gen_op(op_conv_to_float);
                gen_op(op_exch);
                gen_op(op_conv_to_float);
                gen_op(op_exch);
            }
            else if (type2 == TK_REAL) {
                printf("I (||&&==!=<<=>>=) R -> (i) exchange, (ii) conv to real, (iii) exchange\n");
                gen_op(op_exch);
                gen_op(op_conv_to_float);
                gen_op(op_exch);
            }
            else {
                printf("type mismatch on line %i in one of these operators: || && == != < <= > >=\n", curtoken.line);
                exit(EXIT_FAILURE);
            }
        }
        else if (type1 == TK_REAL) {
            if (type2 == TK_REAL) {
                //printf("R (||&&==!=<<=>>=) R\n");
            }
            else if (type2 == TK_INT || type2 == TK_CHAR) {
                printf("R (||&&==!=<<=>>=) I -> (i) conv to real\n");
                gen_op(op_conv_to_float);
            }
            else {
                printf("type mismatch on line %i in one of these operators: || && == != < <= > >=\n", curtoken.line);
                exit(EXIT_FAILURE);
            }
        }
        else {
            printf("type mismatch on line %i in one of these operators: || && == != < <= > >=\n", curtoken.line);
            exit(EXIT_FAILURE);
        }
        return TK_INT;
    }
}

void G(void)
{
    while (curtoken.type != TK_EOF) {
        if (curtoken.type == TK_int || curtoken.type == TK_float || curtoken.type == TK_char) {
            declaration();
        }
        else if (curtoken.type == TK_void) {
            procedure_definition();
        }
        else {
            error();
        }
    }
    match(TK_EOF);
}

Type O(void)
{
    Type t1, t2;
    t1 = A();
    while (curtoken.type == TK_OR) {
        gettoken();
        t2 = A();
        t1 = combine(t1, t2, TK_OR);
        printf("or\n");
        gen_op(op_or);
    }
    return t1;
}

Type A(void)
{
    Type t1, t2;
    t1 = Q();
    while (curtoken.type == TK_AND) {
        gettoken();
        t2 = Q();
        t1 = combine(t1, t2, TK_AND);
        printf("and\n");
        gen_op(op_and);
    }
    return t1;
}

Type Q(void)
{
    Type t1, t2;
    t1 = R();
    while (curtoken.type == TK_EQ || curtoken.type == TK_NEQ) {
        int op_type = curtoken.type;
        gettoken();
        t2 = R();
        t1 = combine(t1, t2, op_type);
        printf("eq\n");
        gen_op(op_eq);
    }
    return t1;
}

Type R(void)
{
    Type t1, t2;
    t1 = E();
    while (curtoken.type == TK_LESS || curtoken.type == TK_LESS_EQ ||
           curtoken.type == TK_GREATER || curtoken.type == TK_GREATER_EQ) {
        int op_type = curtoken.type;
        gettoken();
        t2 = E();
        t1 = combine(t1, t2, op_type);
        if (op_type == TK_LESS) {
            printf("less\n");
            gen_op(op_less);
        }
        else if (op_type == TK_GREATER) {
            printf("greater\n");
            gen_op(op_greater);
        }
        else if (op_type == TK_LESS_EQ) {
            printf("less-eq\n");
            gen_op(op_leq);
        }
        else {
            printf("greater-eq\n");
            gen_op(op_geq);
        }
    }
    return t1;
}

Type E(void)
{
    Type t1, t2;
    t1 = T();
    while (curtoken.type == TK_PLUS || curtoken.type == TK_MINUS) {
        int op_type = curtoken.type;
        gettoken();
        t2 = T();
        t1 = combine(t1, t2, op_type);
        if (op_type == TK_PLUS) {
            if (t1 == TK_INT) {
                printf("add\n");
                gen_op(op_add);
            }
            else {
                printf("fadd\n");
                gen_op(op_fadd);
            }
        }
        else {
            if (t1 == TK_INT) {
                printf("sub\n");
                gen_op(op_sub);
            }
            else {
                printf("fsub\n");
                gen_op(op_fsub);
            }
        }
    }
    return t1;
}

Type T(void)
{
    Type t1, t2;
    t1 = F();
    while (curtoken.type == TK_MULT || curtoken.type == TK_DIV || curtoken.type == TK_MOD) {
        int op_type = curtoken.type;
        gettoken();
        t2 = F();
        t1 = combine(t1, t2, op_type);
        if (op_type == TK_MULT) {
            if (t1 == TK_INT) {
                printf("mul\n");
                gen_op(op_mul);
            }
            else {
                printf("fmul\n");
                gen_op(op_fmul);
            }
        }
        else if (op_type == TK_DIV) {
            if (t1 == TK_INT) {
                printf("div\n");
                gen_op(op_div);
            }
            else {
                printf("fdiv\n");
                gen_op(op_fdiv);
            }
        }
        else {
            printf("mod\n");
            gen_op(op_mod);
        }
    }
    return t1;
}

Type F(void)
{
    Type t;
    if (curtoken.type == TK_PLUS) {
        // F ::= + F
        gettoken();
        t = F();
        /* generate no instruction, but check whether the argument is of the right type */
        if (t != TK_INT && t != TK_CHAR && t != TK_REAL) {
            printf("error: invalid operand type for unary plus (line %i)\n", curtoken.line);
            exit(EXIT_FAILURE);
        }
    }
    else if (curtoken.type == TK_MINUS) {
        // F ::= - F
        gettoken(); 
        t = F();
        if (t == TK_INT || t == TK_CHAR) {
            printf("neg\n");
            gen_op(op_neg);
        }
        else if (t == TK_REAL) {
            printf("fneg\n");
            gen_op(op_fneg);
        }
        else {
            printf("error: invalid operand type for unary plus (line %i)\n", curtoken.line);
            exit(EXIT_FAILURE);
        }
    }
    else if (curtoken.type == TK_LPAREN) {
        // F ::= ( O ) 
        gettoken();
        t = O();
        match(TK_RPAREN);
    }
    else if (curtoken.type == TK_INT) {
        // F ::= constant
        printf("pushi %d\n", curtoken.i); 
        gen_op(op_pushi);
        gen_int(curtoken.i); 
        gettoken();
        t = TK_INT;
    }
    else if (curtoken.type == TK_REAL) {
        // F ::= constant
        printf("fpushi %f\n", curtoken.f);
        gen_op(op_fpushi);
        gen_float(curtoken.f);
        gettoken();
        t = TK_REAL;
    }
    else if (curtoken.type == TK_CHAR) {
        // F ::= constant
        printf("pushi '%c'\n", curtoken.c);
        gen_op(op_pushi);
        gen_int(curtoken.c);
        gettoken();
        t = TK_CHAR;
    }
    else if (curtoken.type == TK_ID) {
        token id = curtoken; /* save id */
        Node* entry = search_symtabs(&symtabs, id.text); /* find entry in symtab */
        if (entry == NULL) {
            printf("error: '%s' undeclared (line: %i, column: %i)\n", id.text, id.line, id.column);
            exit(EXIT_FAILURE);
        }
        t = entry->type;
        gettoken();
        /* check for [ ] (array subscripting) */
        if (curtoken.type == TK_LBRAC) {
            gettoken();
            int index_line = curtoken.line; int index_column = curtoken.column;

            Type itype = O(); /* leave the index on stack */
            printf("pushi %i\n", entry->size / entry->arrlength); /* leave elt size on the stack */
            gen_op(op_pushi);
            gen_int(entry->size / entry->arrlength);
            printf("mul\n");
            gen_op(op_mul); /* multiply the index and the elt size to get the offset (relative address) */
            printf("pushi %i\n", entry->addr); /* push the addr of the array on the stack */
            gen_op(op_pushi);
            gen_addr(entry->addr);
            printf("add\n");
            gen_op(op_add); /* the absolute addr of the array element is now on stack */
            match(TK_RBRAC);
            if (t != TK_ARR) {
                printf("error: '%s' is not an array (line %i, col %i)\n", id.text, id.line, id.column);
                exit(EXIT_FAILURE);
            }
            if (itype != TK_INT && itype != TK_CHAR) {
                printf("error: array subscript is not an integer\n");
                exit(EXIT_FAILURE);
            }
            t = entry->elt_type;
            if (t == TK_REAL) {
                printf("fget\n");
                gen_op(op_fget);
            }
            else {
                printf("get\n");
                gen_op(op_get);
            }
        }
        else { // <constant>
            if (t == TK_INT || t == TK_CHAR) {
                printf("push '%s' (addr: %i)\n", entry->name, entry->addr);
                gen_op(op_push);
                gen_addr(entry->addr);
            }
            else if (t == TK_REAL) {
                printf("fpush '%s' (addr: %i)\n", entry->name, entry->addr);
                gen_op(op_fpush);
                gen_addr(entry->addr);
            }
            else {
                printf("error: invalid type of object '%s' on line %i\n", entry->name, entry->line);
                exit(EXIT_FAILURE);
            }
        }
    }
    else {
        error();
    }
    return t;
}

Type declaration(void)
{
    int declaration_type;
/*    if (curtoken.type == TK_void) {*/
/*        printf("declaration type: void\n");*/
/*        declaration_type = -1;*/
/*    }*/
    if (curtoken.type == TK_char) {
        printf("declaration type: char\n");
        declaration_type = TK_CHAR;
    }
    else if (curtoken.type == TK_int) {
        printf("declaration type: int\n");
        declaration_type = TK_INT;
    }
    else if (curtoken.type == TK_float) {
        printf("declaration type: float\n");    
        declaration_type = TK_REAL;
    }
    else {
        printf("Unknown type name %s\n", curtoken.text);
        exit(EXIT_FAILURE);
    }
    gettoken();
    // check for an early semicolon
    if (curtoken.type == TK_SEMICOLON) {
        printf("empty declaration at line %i, column %i\n", curtoken.line, curtoken.column);
        exit(EXIT_FAILURE);
    }
    /* process all declared names (with optional initializations) */
    init_declarator(declaration_type); /* pass declaration type as argument */
    while (curtoken.type == TK_COMMA) {
        gettoken();
        init_declarator(declaration_type); /* again, pass declaration type as argument */
    }
    /* match a semicolon to terminate the declaration (statement) */
    match(TK_SEMICOLON);
}

void init_declarator(int type)
{
    /* process the declared symbol */
    Node* new_entry = declarator(type); /* create new entry in symtab */
    if (curtoken.type == TK_ASSIGN) {
        gettoken();
        int inits = initializer(new_entry); /* leaves values on the stack */
        int item_size = 0;
        if (new_entry->type == TK_ARR) {
            /* create a pop instruction for each array element that was initialized */
            item_size = new_entry->size / new_entry->arrlength;
            for (int i = inits - 1; i >= 0; i--) {
                if (type == TK_REAL) {
                    printf("fpop '%s[%d]' (addr: %i)\n", new_entry->name, i, i*item_size + new_entry->addr);
                    gen_op(op_fpop);
                }
                else {
                    printf("pop '%s[%d]' (addr: %i)\n", new_entry->name, i, i*item_size + new_entry->addr);
                    gen_op(op_pop);
                }
                gen_addr(i*item_size + new_entry->addr);
            }
        }
        else {
            if (type == TK_REAL) {
                printf("fpop '%s' (addr: %i)\n", new_entry->name, new_entry->addr);
                gen_op(op_fpop);
            }
            else {
                printf("pop '%s' (addr: %i)\n", new_entry->name, new_entry->addr); 
                gen_op(op_pop);
            }
            gen_addr(new_entry->addr);
        }
    }
}

Node* declarator(int type)
{
    Node* inserted;
    if (curtoken.type == TK_ID) {
        /* add the identifier to the symtab (the topmost table) */
        inserted = install_id(symtabs.table[symtabs.top], curtoken.text);
        inserted->type = type; // update the type (int, real, char, array, or func); note that arrays/funcs won't be labeled yet
        inserted->line = curtoken.line; // update the line declared
        inserted->addr = dp; // update the address
        printf("inserted the declared symbol '%s' (addr: %d)\n", inserted->name, inserted->addr);
        /* update the size of the object (if it's an array, size will later be multiplied by length) */
        if (type == TK_INT || type == TK_REAL) {
            inserted->size = 4;
        }
        else if (type == TK_CHAR) {
            inserted->size = 4;
        }
        inserted->arrlength = 1; /* update the length of the object (non-arrays: 1, arrays: n */
        gettoken();
        /* look for brackets (=> it's an array) */
        if (curtoken.type == TK_LBRAC) {
            inserted->elt_type = type; /* indicate the item type (int, float or char) */
            inserted->type = TK_ARR; /* indicate that the object is an array */
            gettoken();
            /* get the specified array size */
            if (curtoken.type == TK_RBRAC) {
                /* deal with cases like 'int x[];' */
                printf("error: array '%s' declared in line %i not given a size\n", inserted->name, inserted->line);
                exit(EXIT_FAILURE);
            }
            else {
                /* process index */
                if (curtoken.type == TK_INT) {
                    inserted->arrlength = curtoken.i; /* update arrlength */
                }
                else if (curtoken.type == TK_CHAR) {
                    inserted->arrlength = curtoken.c; /* update arrlength */
                }
                else {
                    printf("error: size of array '%s' (declared in line %i) must be an integer constant \n",
                           inserted->name, inserted->line);
                    exit(EXIT_FAILURE);
                }
                if (inserted->arrlength <= 0) {
                    printf("error: size of array '%s' (declared in line %i) can't be negative or zero\n", inserted->name, inserted->line);
                    exit(EXIT_FAILURE);
                }
                inserted->size *= inserted->arrlength; /* update size */
                gettoken();
                match(TK_RBRAC);
            }
            printf("...which is an array of %d * %d = %d bytes\n", inserted->size / inserted->arrlength, inserted->arrlength, inserted->size);
        }
        dp += inserted->size; /* increment data counter */
    }
    /* deal with something like 'int (x);' */
    else if (curtoken.type == TK_LPAREN) {
            gettoken();
            declarator(type);
            match(TK_RPAREN);
    }
    return inserted;
}

int initializer(Node* entry)
{
    int inits = 0; /* number of values to initialize (non-array: 1, array: n, where 1 <= n <= size) */
    int capacity = entry->arrlength; /* we'll need to test the object's capacity against the number of inits */
    int l_type, r_type; /* we'll need to test the type of the object we're initializing against the type of each init */
    if (entry->type == TK_ARR) {
        l_type = entry->elt_type;
    }
    else {
        l_type = entry->type;
    }

    if (curtoken.type == TK_LCURL) {
        /* Process something like '{3, 4}' in 'int x[2] = {3, 4}', given the entry to 'x' in the symtab.
        Note that 'int x = {3}' is allowed, so the object we're assigning to may not be an array.
        Maybe a warning (as opposed to an error) could be given for something like 'int x = {3, 4}'
        or 'int x[2] = {3, 4, 5}', i.e., whenever capacity < inits. */
        do {
            gettoken();
            r_type = O(); /* leave a value on the stack */
            if (l_type == TK_INT || l_type == TK_CHAR) {
                if (r_type == TK_REAL) {
                    printf("conv to int\n");
                    gen_op(op_conv_to_int);
                }
            }
            else if (l_type == TK_REAL) {
                if (r_type == TK_INT || r_type == TK_CHAR) {
                    printf("conv to real\n");
                    gen_op(op_conv_to_float);
                }
            }
            if (r_type != TK_INT && r_type != TK_CHAR && r_type != TK_REAL) {
                printf("error: invalid assignment to '%s' in line %i\n", entry->name, entry->line);
                exit(EXIT_FAILURE);
            }
            inits++;
            if (capacity < inits) {
                printf("warning: excess elements in initializer (in line %d, column %d)\n", curtoken.line, curtoken.column);
                printf("delete\n"); /* delete value on stack (give a warning, but don't halt) */
                gen_op(op_remove);
                inits--;
            }
        } while (curtoken.type == TK_COMMA);
        /* now all initialization values (up to capacity) are left on the stack */
        match(TK_RCURL);
    }
    else {
        /* Process something like the 55 in 'int x = 55', given the entry to 'x' in the symtab. */
        if (entry->type == TK_ARR) {
            printf("invalid array initializer (line %d, column %d)\n", curtoken.line, curtoken.column);
            exit(EXIT_FAILURE);
        }
        r_type = O(); /* leave a value on the stack */
        if (l_type == TK_INT || l_type == TK_CHAR) {
            if (r_type == TK_REAL) {
                printf("conv to int\n");
                gen_op(op_conv_to_int);
            }
        }
        else if (l_type == TK_REAL) {
            if (r_type == TK_INT || r_type == TK_CHAR) {
                printf("conv to real\n");
                gen_op(op_conv_to_float);
            }
        }
        if (r_type != TK_INT && r_type != TK_CHAR && r_type != TK_REAL) {
            printf("error: invalid assignment to '%s' in line %i\n", entry->name, entry->line);
            exit(EXIT_FAILURE);
        }
        inits = 1;
    }
    return inits; /* return the number of items left on stack */
}

void assignment(void)
{
    if (curtoken.type == TK_ID) {
        Type l_type, r_type;
        token id_tok = curtoken; /* save info about the identifier */
        gettoken();

        Node* entry = search_symtabs(&symtabs, id_tok.text);
        if (entry == NULL) {
            printf("'%s' undeclared (line %i, col %i)\n", id_tok.text, id_tok.line, id_tok.column);
            exit(EXIT_FAILURE);
        }
        /* deal with an array element such as A[1+i]*/
        if (curtoken.type == TK_LBRAC) {
            gettoken();
            if (curtoken.type == TK_RBRAC) {
                printf("error: expected expression before ']' (line %i, col %i)\n", curtoken.line, curtoken.column);
                exit(EXIT_FAILURE);
            }
            Type t = O(); /* leave the index on stack */
            printf("pushi %i\n", entry->size / entry->arrlength); /* leave elt size on the stack */
            gen_op(op_pushi);
            gen_int(entry->size / entry->arrlength);
            printf("mul\n");
            gen_op(op_mul); /* multiply the index and the elt size to get the offset (relative address) */
            printf("pushi %i\n", entry->addr); /* push the addr of the array on the stack */
            gen_op(op_pushi);
            gen_addr(entry->addr);
            printf("add\n");
            gen_op(op_add); /* the absolute addr of the array element is now on stack */
            match(TK_RBRAC);
            if (entry->type != TK_ARR) {
                printf("error: '%s' is not an array (line %i, col %i)\n", id_tok.text, id_tok.line, id_tok.column);
                exit(EXIT_FAILURE);
            }
            if (t != TK_INT && t != TK_CHAR) {
                printf("error: array subscript is not an integer\n");
                exit(EXIT_FAILURE);
            }
            l_type = entry->elt_type;
            match(TK_ASSIGN);
            r_type = O(); /* value of assignment is left on stack, atop the array addr */
            if (l_type == TK_INT || l_type == TK_CHAR) {
                if (r_type == TK_REAL) {
                    printf("conv to int\n");
                    gen_op(op_conv_to_int);
                }
                printf("put\n");
                gen_op(op_put);
            }
            else if (l_type == TK_REAL) {
                if (r_type == TK_INT || r_type == TK_CHAR) {
                    printf("conv to real\n");
                    gen_op(op_conv_to_float);
                }
                printf("fput\n");
                gen_op(op_fput);
            }
        }
        else if (curtoken.type == TK_ASSIGN) {
            int err_line = curtoken.line;
            int err_column = curtoken.column;
            l_type = entry->type;
            if (l_type == TK_ARR) {
                printf("error: assignment to an array (line %i, col %i)\n", err_line, err_column);
                exit(EXIT_FAILURE);
            }
            if (l_type == TK_INT || l_type == TK_REAL || l_type == TK_CHAR) {
                gettoken();
                r_type = O();
                if (l_type == TK_INT || l_type == TK_CHAR) {
                    if (r_type == TK_REAL) {
                        printf("conv to int\n");
                        gen_op(op_conv_to_int);
                    }
                }
                else if (l_type == TK_REAL) {
                    if (r_type == TK_INT || r_type == TK_CHAR) {
                        printf("conv to real\n");
                        gen_op(op_conv_to_float);
                    }
                }
                else {
                    printf("error: invalid assignment in line %i, col %i\n", err_line, err_column);
                    exit(EXIT_FAILURE);
                }
                if (r_type != TK_INT && r_type != TK_CHAR && r_type != TK_REAL) {
                    printf("error: invalid right operand in assignment to '%s' in line %i\n", entry->name, entry->line);
                    exit(EXIT_FAILURE);
                }
                if (l_type == TK_REAL) {
                    printf("fpop '%s' (addr: %i)\n", entry->name, entry->addr);
                    gen_op(op_fpop);
                }
                else {
                    printf("pop '%s' (addr: %i)\n", entry->name, entry->addr);
                    gen_op(op_pop);
                }
                gen_addr(entry->addr);
            }
            else {
                printf("error: invalid left operand in assignment (line %i, col %i)\n", err_line, err_column);
                exit(EXIT_FAILURE);
            }
        }
    }
    else {
        error();
    }
}

void assignment_statement(void)
{
    /* process zero or more comma-separated assignments terminated by a semicolon */
    if (curtoken.type == TK_SEMICOLON) {
        match(TK_SEMICOLON);
    }
    else {
        assignment();
        while (curtoken.type == TK_COMMA) {
            gettoken();
            assignment();
        }
        match(TK_SEMICOLON);
    }
}

void compound_statement(void)
{
    match(TK_LCURL);
    begin_scope(); /* open a new scope  (new symtab) */
    while (curtoken.type == TK_int || curtoken.type == TK_float || curtoken.type == TK_char) {
        declaration();
    }
    while (curtoken.type != TK_RCURL) {
        if (curtoken.type == TK_int || curtoken.type == TK_float || curtoken.type == TK_char) {
            declaration();
        }
        else {
            statement();
        }
    }
    match(TK_RCURL);
    end_scope(); /* close the scope (delete the symtab) */
}

void statement(void)
{
    if (curtoken.type == TK_ID || curtoken.type == TK_SEMICOLON) {
        if (curtoken.type == TK_ID) {
            Node * entry = search_symtabs(&symtabs, curtoken.text);
            if (entry->type == TK_FUNC) {
                procedure_call();
            }
            else {
                assignment_statement();
            }
        }
        else {
            /* let assignment_statement() deal with a semicolon (empty declaration/statement) */
            assignment_statement(); /* <assignment-statement> ::= {<assignment-list>}? ; */
        }
    }
    else if (curtoken.type == TK_LCURL) {
        compound_statement(); /* <compound-statement> ::= { {<declaration>}* {<statement>}* } */
    }
    else if (curtoken.type == TK_if || curtoken.type == TK_switch) {
        selection_statement(); /* <selection-statement> ::= <if-statement> | <switch-statement> ; */
    }
    else if (curtoken.type == TK_while || curtoken.type == TK_do || curtoken.type == TK_for) {
        iteration_statement(); /* <iteration-statement> ::= <while-loop> | <do-while> */
    }
    else if (curtoken.type == TK_case || curtoken.type == TK_default) {
        if (inside_switch) {
            labeled_statement();
        }
        else {
            printf("case/default label not inside switch\n");
            exit(EXIT_FAILURE);
        }
    }
    else if (curtoken.type == TK_return) {
        jump_statement();
    }
    else if (curtoken.type == TK_print) {
        print_statement();
    }
    else {
        error();
    }
}

void selection_statement(void)
{
    if (curtoken.type == TK_if) {
        if_statement();
    }
    else if (curtoken.type == TK_switch) {
        switch_statement();
    }
    else {
        error();
    }
}

void if_statement(void)
{
    int err_line = curtoken.line;
    int err_column = curtoken.column;
    match(TK_if);
    match(TK_LPAREN);
    Type t = O(); /* leave value of condition expression on stack */
    if (t == TK_INT || t == TK_CHAR || t == TK_REAL) {
        match(TK_RPAREN);
    }
    else {
        printf("error: the condition expression of the if must be of numeric type (line %i, col %i)\n", err_line, err_column);
        exit(EXIT_FAILURE);
    }
    /* insert a jfalse operation (for jumping to the second set of instructions if cond is false) here,
    but we don't know the jump size yet */
    int hole = ip; /* store the location of the jfalse instruction */
    printf("jfalse <arg>\n");
    gen_op(op_jfalse);
    gen_addr(0);

    statement(); /* generate a first set of instructions in case the condition is true */ 
    /* if there is an else, generate a second set of instructions in case the condition is false */
    if (curtoken.type == TK_else) {
        gettoken();
        /* create a second jump here in case the condition is true: we'll want to skip the second set of
        instructions after performing the first one */
        int hole2 = ip;
        printf("jmp <arg>\n");
        gen_op(op_jmp);
        gen_addr(0);
        /* we can update the first jump size in code[hole+1] (jump to current ip <=> jump by ip-hole) */ 
        gen_addr_rel(ip-hole, hole+1); printf("jfalse arg computed: %i\n", ip-hole);  
        statement();
        /* we can update the size of the second jump at code[hole2+1] */
        gen_addr_rel(ip-hole2, hole2+1); printf("jmp arg computed: %i\n", ip-hole2);
    }
    else { 
        gen_addr_rel(ip-hole, hole+1); printf("jfalse arg computed: %i\n", ip-hole); 
    }
}

void iteration_statement(void)
{
    if (curtoken.type == TK_while) {
        while_loop();
    }
    else if (curtoken.type == TK_do) {
        do_while();
    }
    else {
        error();
    }
}

void do_while(void)
{
    match(TK_do);
    int start = ip; /* save the start of the instructions corresponding to the <statement> */
    statement();
    match(TK_while);
    match(TK_LPAREN);
    int err_line = curtoken.line;
    int err_column = curtoken.column;
    Type t = O();
    if (t == TK_INT || t == TK_CHAR || t == TK_REAL) {
        match(TK_RPAREN);
    }
    else {
        printf("error: the condition expression of the do...while must be of numeric type (line %i, col %i)\n", err_line, err_column);
        exit(EXIT_FAILURE);
    }
    /* jump in the negative direction to the start */
    printf("jtrue %i\n", start-ip);
    gen_op(op_jtrue); /* note that this increases ip by 1 */
    gen_addr(start-(ip-1));
    match(TK_SEMICOLON);
}

void while_loop(void)
{
    match(TK_while);
    match(TK_LPAREN);
    int err_line = curtoken.line;
    int err_column = curtoken.column;
    /* mark the instruction to jump back to after each iteration (to check the condition again) */
    int start = ip;
    Type t = O(); /* leave value of condition expression on stack */
    if (t == TK_INT || t == TK_CHAR || t == TK_REAL) {
        match(TK_RPAREN);
    }
    else {
        printf("error: the condition expression of the while-loop must be of numeric type (line %i, col %i)\n", err_line, err_column);
        exit(EXIT_FAILURE);
    }
    /* jump past <statement> if condition is false */
    int hole = ip; /* we don't know the jump size yet */
    printf("jfalse <arg>\n");
    gen_op(op_jfalse);
    gen_addr(0);
    statement();
    /* jump to the beginning */
    printf("jmp %i\n", start-ip);
    gen_op(op_jmp); /* note that this increases ip by 1 */
    gen_addr(start-(ip-1));
    /* fix hole after jfalse instruction */
    printf("jfalse arg computed: %i\n", ip-hole);
    gen_addr_rel(ip-hole, hole+1);
}

void switch_statement(void)
{
    inside_switch = 1;
    match(TK_switch);
    match(TK_LPAREN);
    Type t = O(); /* push expr on stack */
    match(TK_RPAREN);
    if (t != TK_INT && t != TK_CHAR) {
        printf("error: switch expr not an integer\n");
        exit(EXIT_FAILURE);
    }
    int hole = ip;
    printf("pushi <end-of-switch-addr>\n");
    gen_op(op_pushi);
    gen_addr(0); /* leave switch-exit address on stack */
    printf("exch\n");
    gen_op(op_exch); /* move expr back to top of stack */
    switch_body();
    printf("remove\n");
    gen_op(op_remove); /* pop expr from stack */
    gen_op(op_remove); /* pop retaddr from stack */
    gen_addr_rel(ip, hole+1); /* end switch */
    inside_switch = 0;
}

void switch_body(void)
{
    match(TK_LCURL);
    begin_scope(); 
    while (curtoken.type == TK_case || curtoken.type == TK_default) {
        if (curtoken.type == TK_case) {
            gettoken(); 
            printf("dup\n");
            gen_op(op_dup);
            Type t = O();
            match(TK_COLON);
            if (t != TK_INT && t != TK_CHAR) {
                printf("error: case label not an integer\n");
                exit(EXIT_FAILURE);
            }
            printf("eq\n");
            gen_op(op_eq);
            /* jump past statement if dup != label */
            int hole = ip;
            printf("jfalse <end-of-case-addr>\n");
            gen_op(op_jfalse);
            gen_addr(0);
            statement();
            /* remove the switch val from stack and jump to end of switch */
            printf("remove\n");
            gen_op(op_remove);
            printf("exit switch\n");
            gen_op(op_return);
            gen_addr_rel(ip-hole, hole+1); 
        }
        else if (curtoken.type == TK_default) {
            gettoken();
            match(TK_COLON);
            statement();
            printf("remove\n");
            gen_op(op_remove);
            printf("exit switch\n");
            gen_op(op_return);
        }
    }
    match(TK_RCURL);
    end_scope(); /* close the scope (delete the symtab) */
}

void labeled_statement(void)
{
    Type t;
    if (curtoken.type == TK_case) {
        gettoken();
        printf("dup\n");
        gen_op(op_dup); /* leave a copy of the switch value on stack */
        Type t = O();
        match(TK_COLON);
        if (t != TK_INT && t != TK_CHAR) {
            printf("error: case label not an integer\n");
            exit(EXIT_FAILURE);
        }
        printf("eq\n");
        gen_op(op_eq);
        /* jump past statement if dup != label */
        int hole = ip;
        printf("jfalse <arg>\n");
        gen_op(op_jfalse);
        gen_addr(0);
        statement();
        printf("remove\n");
        gen_op(op_remove);
        printf("exit switch\n");
        gen_op(op_return); /* exit the switch */
        gen_addr_rel(ip-hole, hole+1); /* jump to here */
    }
    else if (curtoken.type == TK_default) {
        gettoken();
        match(TK_COLON);
        statement();
        printf("remove\n");
        gen_op(op_remove);
    }
    else {
    }
}

void jump_statement(void)
{
    if (curtoken.type == TK_return) {
        printf("return\n");
        gen_op(op_return);
    }
    match(TK_SEMICOLON);
}

void procedure_definition(void)
{
    /* leave a jmp instr to skip calling the procedure until a real call is made */
    int hole = ip;
    printf("jmp <arg>\n");
    gen_op(op_jmp);
    gen_addr(0);

    /* output instructions for the procedure */
    match(TK_void);
    if (curtoken.type == TK_ID) {
        Node* entry = install_id(symtabs.table[symtabs.top], curtoken.text); /* may give a redeclaration error */
        entry->type = TK_FUNC;
        entry->line = curtoken.line;
        gettoken();
        match(TK_LPAREN);
        int num_of_params = 0;
        /* process entry into procedure */
        /* a modification of compound_statement() is what follows.
        begin a new scope (symtab) *before* the brace in order to declare any params within the procedure scope */
        begin_scope();
        entry->addr = ip; /* save the entry addr */
        int type;
        while (curtoken.type == TK_int || curtoken.type == TK_float || curtoken.type == TK_char) {
            if (curtoken.type == TK_int) {
                type = entry->params[num_of_params++] = TK_INT;
            }
            else if (curtoken.type == TK_float) {
                type = entry->params[num_of_params++] = TK_REAL;
            }
            else {
                type = entry->params[num_of_params++] = TK_CHAR;
            }
            gettoken();
            Node* entry2 = install_id(symtabs.table[symtabs.top], curtoken.text);
            entry2->type = type;
            entry2->size = 4;
            entry2->addr = dp;
            entry2->line = curtoken.line;
            entry2->arrlength = 1;
            dp += entry2->size;
            /* when a proc is called, the args (converted to right type) are left on the stack,
            so we must retrieve them first */
            printf("inserted the declared symbol '%s' (addr: %d)\n", entry2->name, entry2->addr);
            if (type == TK_INT || type == TK_CHAR) {
                printf("pop '%s' (addr: %i)\n", entry2->name, entry2->addr);
                gen_op(op_pop);
                gen_addr(entry2->addr);
            }
            else {
                printf("fpop '%s' (addr: %i)\n", entry2->name, entry2->addr);
                gen_op(op_fpop);
                gen_addr(entry2->addr);
            }
            gettoken();
            if (curtoken.type == TK_COMMA) {
                gettoken();
            }
        }
        entry->num_of_params = num_of_params;
        /* process the body (compound statement) */
        match(TK_RPAREN);
        match(TK_LCURL);
        while (curtoken.type == TK_int || curtoken.type == TK_float || curtoken.type == TK_char) {
            declaration();
        }
        while (curtoken.type != TK_RCURL) {
            if (curtoken.type == TK_int || curtoken.type == TK_float || curtoken.type == TK_char) {
                declaration();
            }
            else {
                statement();
            }
        }
        match(TK_RCURL);
        end_scope();
        /* let return be the last instr */
        printf("return\n");
        gen_op(op_return);
        /* we can update the hole now, which is at code[hole+1] */
        /* note that if no instructions are generated for the procedure, ip-hole = 0 */
        gen_addr_rel(ip-hole, hole+1); printf("jmp arg computed: %i\n", ip-hole);
    }
    else {
        error();
    }
}

void conv(Type t1, Type t2)
{
    if (t1 == TK_INT || t1 == TK_CHAR) {
        if (t2 == TK_REAL) {
            printf("conv to int\n");
            gen_op(op_conv_to_int);
        }
    }
    else if (t1 == TK_REAL) {
        if (t2 == TK_INT || t2 == TK_CHAR) {
            printf("conv to float\n");
            gen_op(op_conv_to_float);
        }
    }
    else {
        error();
    }
}

void procedure_call(void)
{
    /* Before placing the set of instructions corresponding to a proc definition in the code array, place a jump <hole> operation in the code array.
    Update the initially-unknown <hole> so that the jump skips to the instruction right after the last instruction in the proc.
    The return keyword generates a return instruction, but append an op_return to the end of the procedure instructions regardless.
    An op_call shall begin executing the proc instructions by going to the particular ip (check the addr field of the func's entry in the symtab).
    The input to the op_return is left on the stack beforehand. (op_return removes from the stack and returning to the original ip). */

    if (curtoken.type == TK_ID) {
        Node* entry = search_symtabs(&symtabs, curtoken.text);
        gettoken();
        match(TK_LPAREN);
        if (entry == NULL) {
            printf("error: undefined reference to function '%s' in line %i\n", entry->name, entry->line);
            exit(EXIT_FAILURE);
        }
        if (entry->type != TK_FUNC) {
            printf("error: expected '%s' to be a procedure in line %i\n", entry->name, entry->line);
            exit(EXIT_FAILURE);
        }
        int num_of_params = entry->num_of_params;
        // somehow push the args to stack (after passing them through the conv func) backward...
        // check for same nmber of args as well.
        int count = 0;
        Type t;
        while (curtoken.type != TK_RPAREN) {
            t = O();
            conv(entry->params[count], t); /* convert the arg on stack appropriately */
            count++;
            if (count > num_of_params) {
                printf("error: too many arguments to function '%s' called in line %i\n", entry->name, entry->line);
                exit(EXIT_FAILURE);
            }
            if (curtoken.type == TK_COMMA) {
                gettoken();
            }
        }
        if (count != num_of_params) {
            printf("error: too few arguments to function '%s' called in line %i\n", entry->name, entry->line);
            exit(EXIT_FAILURE);
        }
        /* 
        for function call 'func(a,b,c);' the stack before reversal should look like

        retaddr
        c
        b
        a

        so that after reverse(count+1), it will look like

        a
        b
        c
        retaddr

        Thus, when the function is called, it takes the 3 parameters from stack in the correct order;
        When it returns, retaddr will be at the top of the stack.

        Note that the return address pushed cannot be the ip at the time the push instruction is generated
        because the machine has to skip past 15 instructions (including the push instruction):

        | pushi <retaddr> | reverse <n> | call <procaddr> |
        0                 5             10                15
        */
        int ret_addr = ip + 15;
        printf("pushi retaddr %i\n", ret_addr);
        gen_op(op_pushi);
        gen_addr(ret_addr);
        printf("reverse %i\n", count+1);
        gen_op(op_reverse);
        gen_int(count+1);
        match(TK_RPAREN);
        match(TK_SEMICOLON);
        printf("call '%s' (addr: %i)\n", entry->name, entry->addr);
        gen_op(op_call);
        gen_addr(entry->addr);
    }
    else {
        error();
    }
}

void print_statement(void)
{
    Type t;
    match(TK_print);
    if (curtoken.type == TK_STR) {
        /* push an array of characters onto the stack (in backward order) */
        int length = strlen(curtoken.s);
        for (int i = length-1; i >= 0; i--) {
            printf("pushi '%c'\n", curtoken.s[i]);
            gen_op(op_pushi);
            gen_int(curtoken.s[i]);
        }
        gettoken();
        for (int i = 0; i < length; i++) {
            printf("op_printchar\n");
            gen_op(op_printchar);
        }
        printf("op_println\n");
        gen_op(op_println);
    }
    else {
        t = O();
        if (t == TK_INT) {
            printf("op_printint\n");
            gen_op(op_printint);
            printf("op_println\n");
            gen_op(op_println);
        }
        else if (t == TK_REAL) {
            printf("op_printfloat\n");
            gen_op(op_printfloat);
            printf("op_println\n");
            gen_op(op_println);
        }
        else if (t == TK_CHAR) {
            printf("op_printchar\n");
            gen_op(op_printchar);
            printf("op_println\n");
            gen_op(op_println);
        }
        else {
            printf("error: invalid type operand to print\n");
            exit(EXIT_FAILURE);
        }
    }
    match(TK_SEMICOLON);
}

void begin_scope(void)
{
    push_symtab(create_table(), &symtabs);
}

void end_scope(void)
{
    remove_symtab(&symtabs);
}

void start(void)
{
    Node* mainproc = search_symtabs(&symtabs, "main"); /* find entry in symtab */
    if (mainproc == NULL) {
        printf("error: undefined reference to function 'main'\n");
        exit(EXIT_FAILURE);
    }
    else {
        int ret_addr = ip + 10;
        printf("pushi retaddr %i\n", ret_addr);
        gen_op(op_pushi);
        gen_addr(ret_addr);

        printf("call 'main' (addr: %i)\n", mainproc->addr);
        gen_op(op_call);
        gen_addr(mainproc->addr);
    }
}

void parse(void)
{
    code = malloc(100000);
    FILE* fp = read_file(sourcefile);
    toks = scan(); /* get tokens from tokenizer */
    print_tokens(toks);
    printf("\n");

    /* initialize stack of symbol tables */
    symtabs.top = -1;
    List* tab = create_table();
    push_symtab(tab, &symtabs);

    while (toks[0].type == TK_EOL) {
        toks++;
    }
    curtoken = toks[0];

    G();

    /* append instructions to call main */
    start();

    /* append halt instr */
    printf("halt\n");
    gen_op(op_halt);

    for (int i = 0; i < ip; i++) {
        printf("%i\n", code[i]);
    }
}
