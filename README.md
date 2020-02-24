# Instructions to run
1. `cd` to the directory that contains the project
1. `gcc vm.c stack.c symtab.c tokenizer.c parser.c`
1. `./a.out <file>` or `./a.out`. If you don't provide an input file, the input file will be `test_input.c` by default.

The output will always be the following sequence:
1. `<tokenizer output>` (the list of tokens that the source code was broken into)
1. `<parser output>` (the list of instructions, in human-readable format and then in bytes: but note that the human-readable format will contain some intermediate instructions, such as "inserted 'x' into symbol table")
1. `<interpreter output>` (the output of your program)
