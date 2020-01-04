/* Usage:
    - each case/default automatically breaks, as in Pascal
      (therefore, it's pointless to have default anywhere except at the end, where it acts as an 'else')
    - no statements can occur between each case/default.
    - the case labels and switch value are restricted to int/char type.
    - the case/default labels can be repeated (only the first matched case applies) and be variables.
   Implementation:
    - the switch val is left on the stack as well as the return address (address to the end of the switch)
    - each case/default duplicates the switch val (the comparison with the label converts the switch val to either 0 and 1).
      then a jfalse operation (which removes the duplicate) possibly tells the machine to go to the end of the case.
    - the final instructions of a case clean up the stack: remove (delete the original switch val) and return
      (delete the retaddr as well). however, the jfalse could go past those instructions.
    - if a case did not clean up the 2 values on stack (i.e., if none of the labels matched), then the switch will.
      if a case does, then the switch won't.
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
}
