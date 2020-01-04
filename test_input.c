/* Usage:
    - each case/default automatically breaks, as in Pascal
    - therefore, it's pointless to have default anywhere except at the end (where it acts as an 'else')
   Implementation:
    - the switch val is left on the stack as well as the return address (address to the end of the switch)
    - each case duplicates the switch val for comparison with the label (the comparison converts the switch val to either 0 and 1).
      then it does jfalse to the end of the case, removing the duplicated val from stack.
    - the jfalse goes past the final instructions of a case: remove (delete the original switch val) and return (delete the retaddr as well).
    - if a case did not clean up the 2 values on stack (if none of the labels matched), then the switch will.
*/
void main()
{
    char x = 'a';

    switch (x) {
    }

    switch (x) {
        default: ;
    }

    switch (x) {
        case 'a': ;
        case 'b': ;
    }

    while (x <= 'd') {
        switch (x) {
            case 'a': print x;
            case 'b': print x;
            case 'c': print x;
            default: print "unknown";
        }
        x = x + 1;
    }

    switch (x) {
        default: print "catch";
        case 'e': print "fall";
    }
}
