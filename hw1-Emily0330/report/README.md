# hw1 report

|Field|Value|
|-:|:-|
|Name|your full name |
|ID|your student id|

## How much time did you spend on this project

10 hours for coding & research.  
4.5 hours for report.

## Project overview

Describe the project structure and how you implemented it.
### Project Structure

The goal of this project is to write a scanner for the `P` language in `lex`. The below gives the definitions of different objects in the `P` language.

* HW1 - EMILY0330
    * /src
        * Makefile
        * `scanner.l`
    * /report
        * `README.md`

#### Things to note
1. Only two control characters are used in `P` language: `\n` and `\t`.
2. Every rule other than multi-line comment is in <INITIAL> state because I don't want lexemes in comment (/**/) to match those rules. If I didn't do this, the lexemes may match other undesired rules due to the longest-match policy.

### Lexical Definition - Tokens passed to the parser

#### Delimiters
def: `delimiter [,;:()\[\]]`  
rule: `<INITIAL>{delimiter} { listToken(yytext); }`

#### Arithmetic, Relational, and Logical Operators
* -, +, *, /, =, <, >  
    * def: `arithmetic [-+*/=<>]` ,  
    * rule: `<INITIAL>{arithmetic} { listToken(yytext); }` 
* mod, <=, >=, <>, :=, and, or, not
    * rule: `<INITIAL>OPERATOR { listToken("OPERATOR"); }`

#### Reserved Words
* rule: `<INITIAL>"RESERVED_WORD" { listToken("RESERVED_WORD"); }`  
#### Identifiers
* def: `identifier [a-zA-Z][0-9a-zA-Z]*`  
* rule: `<INITIAL>{identifier} { listLiteral("id", yytext); }`  

#### Integer Constant
* Octal
    * def: `octal_int [0][0-7]+`
    * rule: `<INITIAL>{octal_int} { listLiteral("oct_integer", yytext); }`
* Decimal
    * def: `decimal_int [0]|([1-9][0-9]*)`
        * cannot have leading zeros
    * rule: `<INITIAL>{decimal_int} { listLiteral("integer", yytext); }`  
#### Floating-Point Constants
* def: `float ([0]|([1-9][0-9]*))\.(([0-9]*[1-9])|0)`
    * `([0]|([1-9][0-9]*))` : a decimal integer
    * `(([0-9]*[1-9])|0)` : one or more digits w/o redundant trailing zeros
* rule: `<INITIAL>{float} { listLiteral("float", yytext); }`  

#### Scientific Notations
* def: `scientific (([1-9][0-9]*\.(([0-9]*[1-9])|0))|(0\.([0-9]*[1-9]))|([1-9][0-9]*))[eE][-+]?([0]|([1-9][0-9]*))`
    * `(([1-9][0-9]*\.(([0-9]*[1-9])|0))|(0\.([0-9]*[1-9]))|([1-9][0-9]*))` : a **nonzero** decimal integer or floating point
    * `[-+]?([0]|([1-9][0-9]*))` : a decimal integer prefixed with an optional sign  
* rule: `<INITIAL>{scientific} { listLiteral("scientific", yytext); }`  
#### String Constants
* def: `string \"([^"]|(\"\"))*\"`  
    * `\"` : double quotes outside the string
    * `([^"]|(\"\"))*` : the content of string can be **a sequence of** 
        1. `[^"]` : any character excluding a single double quote (`"`)
        2. `(\"\")`: two double quotes (`""`) 
* rule: `<INITIAL>{string} { listLiteral("string", processString(yytext, yyleng));}`
* user-defined function to process string
    * the RE-matched string contains the outside double quotes and the `""` inside it, and we need to remove them

    ```c
    static const char* processString(char *old, int strlen) {
        //remove the outmost double quotes
        old++;
        strlen -= 1;
        old[strlen - 1] = '\0';
        strlen -= 1;
        // deal with ""
        for(int co = 0; co < strlen - 1; co++){
            if(*(old + co) == '"' && *(old + co + 1) == '"'){
                memmove(old + co, old + co + 1, strlen - co); //'\0' 也要移
                //co does not need to increase because we move the string afterwards 1 position forward
                strlen -= 1;
            }
        }
        return old;
    }
    ```  

:bulb: A string constant is supposed to be always properly closed.

:bulb: A string is NOT supposed to have embedded newlines! (p.s. embedded newline **is not** a newline character `\n`)

### Lexical Definition - Tokens NOT passed to the parser

#### Whitespaces
* def: `whitespace [ \t\n]`  
* rule: `<INITIAL>{whitespace} {;}`  

#### Comments
* One-line comment (*C++-style*) (`//`)
    * def: `one_line_comment \/\/[^\n]*`
    * rule: `<INITIAL>{one_line_comment} {;}`
    * Note that the pseudocomments will be categorized into this rule as well, but they are processed int the `#define YY_USER_ACTION` section.
        * The macro `YY_USER_ACTION` provides an self-defined action which is always executed **prior to the matched rule's action**.
* Multiple-line comment (*C-style*) (`/**/`)
    * *C-style* comments are always properly closed
    * Defined a start state `COMMENT` (exclusive state): `%s COMMENT` 
    * rule:
    ```c
    <COMMENT>\n {;}
    <COMMENT>"*/" {BEGIN INITIAL;}
    <COMMENT>. {;} //. does not include \n !!
    <INITIAL>"/*" {BEGIN COMMENT;}
    ```

#### pseudocomment
* in def section:
I modified the macro `YY_USER_ACTION`.
Before modified, it execute the function `updateCurrentLine(yytext)` every time a rule is matched. I added some control on the two signals `opt_src` and `opt_tok` as follows. The signal values will be modified when not in `COMMENT` state, i.e. the pseudocomments are not enclosed in `/**/`.

|option|signal|
|---|---|
|`&S+`|`opt_src` = 1|
|`&S-`|`opt_src` = 0|
|`&T+`|`opt_tok` = 1|
|`&T-`|`opt_tok` = 0|

```c
// Compare the characters at 2 to 4 positions with &[ST][-+] to see if it's pseudocomment
#define YY_USER_ACTION \
if(YY_START != COMMENT) { \
    if(strncmp(yytext + 2,"&S-", 3) == 0) opt_src = 0; \
    else if(strncmp(yytext + 2,"&S+", 3) == 0) opt_src = 1; \
    else if(strncmp(yytext + 2, "&T+", 3) == 0) opt_tok = 1; \
    else if(strncmp(yytext + 2, "&T-", 3) == 0) opt_tok = 0; \
} \
updateCurrentLine(yytext);
```

> [!note]  
> `&[ST][-+]` 如果要當pseudocomment，前面必接 `//`。  
因為 pseudocomment 一定會有 longest match 成 one-line comment 的可能，因此就讓他 match 到 one_line_comment rule，再針對 `yytext` 第 2 ~ 4 個字符做判斷處理（反正 pseudocomment有限制在第一組 `&[ST][-+]` 後面的字符都忽略，對於被忽略的字符處理方式跟 comment 一樣）  

#### Error Handling
* rule:
```c
/* Catch the character which is not accepted by rules above */
. {
    printf("Error at line %d: bad character \"%s\"\n", line_num, yytext);
    exit(-1);
}
```

## What is the hardest you think in this project

* Debug很難! 因為不知道lex怎麼用debugger...
* 對 lex 不熟悉
    * YY_USER_ACTION 定義了會執行在 rules 的 actions 前還是後 => 前
        * 本來一直以為是後執行，城市行為很怪 debug 超久才發現是在 action 前執行
    * 是 longest rule 還是 earliest rule 優先?
        * 匹配到多個規則時是 longest rule, 一樣長度才是 earliest rule 
* 對 regular expression 不熟悉
    * 哪些字符要轉譯常常忘記，且有沒有在 character class 中也會影響要不要轉譯

* 如果說是最難處理的就是 pseudocomment (應該在哪裡改變變數值試很久)、string(re、`processString` function 改了一陣子)
## Feedback to T.A.s

謝謝 TA 們，寫完滿有成就感的，但就是 hidden testcases 占比好大，不知道可以拿幾分 QQ
