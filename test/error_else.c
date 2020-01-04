void main()
{
    /* <compound-statement> ::= { {<declaration>}* {<statement>}* } */
    int x = 7; /* {<declaration>} */
    /* <if-statement> ::= if ( <O> ) <statement>
                        | if ( <O> ) <statement> else <statement> */
    if (1)     /* if ( <O> ) */
        x = 6; /* <statement> */
               /* end of if-statement */
        x = 7; /* another <statement> in our <compound-statement> */
    else       /* unexpected else ("else without previous if") */
        x = 7;
}
