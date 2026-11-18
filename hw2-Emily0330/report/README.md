# hw2 report

|||
|-:|:-|
|Name|your full name |
|ID|your student id|

## How much time did you spend on this project

5.5 hours for coding & research.  
4.5 hours for report. 

## Project overview

> Please describe the structure of your code and the ideas behind your implementation in an organized way. \
> The point is to show us how you deal with the problems. It is not necessary to write a lot of words or paste all of your code here. 

### Project Structure
* HW2-EMILY0330
    * /src
        * Makefile
        * `scanner.l`
        * `parser.y`
        * other files generated when running `make`
            * ex. `parser.output` for parsing table
    * /report
        * `README.md`
### Implementation
#### Table of Contents
- [Modifications in `scanner.l`](#modifications-in-scannerl)
- [Implementation in `parser.y`](#implementation-in-parsery)
    - [Operator Precedence & Associativity](#operator-precedence--associativity)
    - [Dealing with zero or more occurrences of an item](#dealing-with-zero-or-more-occurrences-of-an-item)
    - [Dealing with an optional item](#dealing-with-an-optional-item)
    - [Left factoring](#left-factoring)
#### Modifications in `scanner.l`
1. `#include "parser.h"` in the C includes part.
2. For each token, add the return value so that `yyparse()` can  get the token by calling `yylex()`.  
`TOKEN { listToken(TOKEN); return TOKEN_NAME;}`

#### Implementation in `parser.y`
* ##### Operator Precedence & Associativity

    * Follow the operator precedence table given in `HW2-EMILY0330/README.md`, I constructed the precedence using the following code.
    * All operators in this assignment are left-associative.

    ```c
    %left AND OR NOT // lowest precedence
    %left LT LEQ NE GEQ GT EQ
    %left MINUS
    %left PLUS
    %left MOD DIV
    %left MUL
    %left UMINUS //unary minus, highest precedence
    ```

* ##### Dealing with zero or more occurrences of an item  
    Define another nonterminal that has 2 productions with different bodies: 
    1. *repeated_item* *nonterminal_itself* 
    2. empty string  

    Take the nonterminal *program* for example.  
    ```
    program: ID SEMI p0 compound_statement END
           ;

    p0: declaration p0
      | p1
      |
      ;

    p1: function p1
      |
      ;
    ```

    *program*: 
    ```
    identifier ;
    { declaration }
    { function }
    compound_statement
    end
    ```
    *p0*: *declaration* can appear zero or more times, and *function* may appear after all declarations.  
    *p1*: *function* can appear zero or more times.  

* ##### Dealing with an optional item  
    Define another nonterminal that has two productions:
    1. *new_nonterminal* -> body with the optional item
    2. *new_nonterminal* -> body without the optional item  

    Take *constant_declaration* for example, a *constant_declaration* has the following forms, where in the top 2 forms, the unary minus is optional.  
        
    - **var** _identifier_list_ **:** [ **-** ] _integer_literal_ **;**
    - **var** _identifier_list_ **:** [ **-** ] _real_literal_ **;**
    - **var** _identifier_list_ **:** _string_literal_ **;**
    - **var** _identifier_list_ **:** _boolean_literal_ **;**  
    
    ```c
    constant_declaration: VAR identifier_list COLON cd0 SEMI
                        ;

    cd0: MINUS cd1 %prec UMINUS /* with MINUS sign */
       | cd1 /* without MINUS sign */
       | string_literal
       | boolean_literal
       ;

    cd1: integer_literal
       | real_literal
       ;
    ```
* ##### Left factoring  

    I performed left factoring on some of the productions. Although I found that it is not necessary to do left factoring when using LALR(1), but anyway I did it and preferred not to change. It is always a good practice to write unambiguous grammar...right?? **orz**.  
    
    Take *conditional_statement* for instance, a *conditional_statement* has the following forms:
    
    - **if** *expression* **then** \
       *compound_statement* \
       **else** \
       *compound_statement* \
       **end** **if**
    - **if** *expression* **then** \
       *compound_statement* \
       **end** **if**

    As a result, I factored out the part "**if** *expression* **then** *compound_statement*", and made the rest another nonterminal *cs*.
    
    ```
    conditional_statement: IF expr THEN compound_statement cs
                         ;
    cs: ELSE compound_statement END IF
      | END IF
      ;
    ```

    :bulb: We do not have to deal with dangling-ELSE problem because `P` language uses `END IF` to mark the end of a conditional statement.  

## What is the hardest you think in this project

> Not required, but bonus point may be given.  

The first bug I encountered took me the longest time to solve.  
Originally, I used `BEGIN` as the return value for the keyword `begin` in `scanner.l`. However, a lot of errors in the form of `yacc error: 'xxx' undeclared (first use in this function);` are shown when running `make`.  
Eventually, I found that in `scanner.c`, the word `BEGIN` had been defined as an macro. As a result, I changed the return value of the keyword `begin` to `KWBEGIN` to solve the problem. 

In `scanner.c`, `BEGIN` is used as a macro.

```c
#define BEGIN (yy_start) = 1 + 2 *
```

In `scanner.l`, I changed the return value of the keyword `begin` to `KWBEGIN`.

```c
"begin" {listToken("KWbegin"); return KWBEGIN;}
```

## Feedback to T.A.s

> Not required, but bonus point may be given.

謝謝 TAs，覺得寫作業消除了上課內容的抽象感，滿有趣的！
