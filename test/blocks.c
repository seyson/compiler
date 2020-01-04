/* Nested blocks (compound statements) :
A new symbol table is created at the beginning of a block and destroyed at the end of the same block.
Thus, variables declared inside the block are not visible in the enclosing block.
However, variables declared outside of the block are visible inside the block.

<compound-statement> ::= { {<declaration-or-statement>}* }
<statement> ::= <compound-statement>
*/

void p(int a)
{
    print a;
}

int j = 4;

void main()
{
    {
        int j = 3;
        {
            int j = 2;
            {
                int j = 1;
                print j;
            }
            print j;
        }
        print j;
    }
    print j;
}
