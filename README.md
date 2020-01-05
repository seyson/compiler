# List of C constructs processed by the compiler

```
[✔] Arithmetic expressions

[✔] Operators

[✔] Procedures

[✔] Arrays

[✔] Statements
  [✔] declarations
  [✔] assignments
  [✔] case
  [ ] goto 
  [✔] if
  [✔] while
  [✔] do...while
  [ ] for
  [✔] print
```

Differences from actual C (not totally complete list):
+ Not all operators are implemented (e.g., there are no bitwise operators). Expressions can be identifiers, literals, or combinations of identifiers, literals, parentheses, unary `+`, unary `-`, `*`, `/`, `%`, `+`, `-`, `>`, `<`, `<=`, `>=`, `==`, `!=`, `&&` and `||`. Precedence rules are respected for all these operators. For example, `(1 + 2) * -5` evaluates to `-15` and `1 < 2 - 3` evaluates to `0`.
+ Assignments are not considered expressions by this compiler (`x = y = 5` is impossible).
+ Function prototypes (function declarations without a definition) are impossible.
+ Functions can have any number of parameters of type `int`, `char` or `float` (but the parameters can't be arrays. Functions cannot return anything and must be declared with `void`.
+ Recursion does not work properly.
+ There is a `print` statement that looks like this: `print A[i]`.

# How to run
1. `cd` to the directory that contains the project
1. `gcc vm.c stack.c symtab.c tokenizer.c parser.c`
1. `./a.out <file>` or `./a.out`. If you don't provide an input file, the input file will be `test_input.c` by default. Pre-written test files are given in the `test` folder.

The output of the program will always be the following sequence:
1. `<tokenizer output>` (the list of program tokens)
1. `<parser output>` (the list of instructions, in human-readable format and then in bytes: but note that the human-readable format will contain some intermediate instructions, such as "inserted 'x' into symbol table"
1. `<interpreter output>` (the output of the program)

Example: 

```c
// test/dowhile.c
void main()
{
    int i = 0;
    do {
        print i;
        i = i - 1;
    } while (i >= 0);
    print i;
}
```

`./a.out test/if.c` outputs

```console
Token: (Keyword) void
Token: (Identifier) main
Token: (
Token: )
Token: End of line
Token: {
Token: End of line
Token: (Keyword) int
Token: (Identifier) i
Token: =
Token: (Integer) 0
Token: ;
Token: End of line
Token: (Keyword) do
Token: {
Token: End of line
Token: (Keyword) print
Token: (Identifier) i
Token: ;
Token: End of line
Token: (Identifier) i
Token: =
Token: (Identifier) i
Token: -
Token: (Integer) 1
Token: ;
Token: End of line
Token: }
Token: (
Token: (Identifier) i
Token: >=
Token: (Integer) 0
Token: )
Token: ;
Token: End of line
Token: (Keyword) print
Token: (Identifier) i
Token: ;
Token: End of line
Token: }
Token: End of line
Token: End of file
```

```console
jmp <arg>
declaration type: int
inserted the declared symbol 'i' (addr: 0)
pushi 0
pop 'i' (addr: 0)
push 'i' (addr: 0)
op_printint
op_println
push 'i' (addr: 0)
pushi 1
sub
pop 'i' (addr: 0)
push 'i' (addr: 0)
pushi 0
I (||&&==!=<<=>>=) I -> (i) conv to real, (ii) exchange, (iii) conv to real, (iv) exchange
greater-eq
jtrue -38
push 'i' (addr: 0)
op_printint
op_println
return
jmp arg computed: 66
pushi retaddr 76
call 'main' (addr: 5)
halt
39
0
0
0
66
2
0
0
0
0
4
0
0
0
0
0
0
0
0
0
48
51
0
0
0
0
0
2
0
0
0
1
14
4
0
0
0
0
0
0
0
0
0
2
0
0
0
0
37
9
37
9
28
41
255
255
255
218
0
0
0
0
0
48
51
43
2
0
0
0
76
42
0
0
0
5
52

ip: 77, dp: 4
```

```console
;;;;;;;;;;;;;; BEGINNING OF OUTPUT ;;;;;;;;;;;;;;;;
0
-1
;;;;;;;;;;;;;;;;; END OF OUTPUT ;;;;;;;;;;;;;;;;;;;
```
