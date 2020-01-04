#include <stdio.h>
#include <stdlib.h>
// #include "tokenizer.h"
// #include "tokenizer.h"
#include "parser.h"
#include "parser.h"

void main(void)
{
    // FILE* fp = read_file("test_input.c");
    // token* tks = scan();
    // print_tokens(tks);

    sourcefile = "test/switch.c";
    parse();
}
