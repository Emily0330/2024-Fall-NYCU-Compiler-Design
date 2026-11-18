%{
#include <stdint.h> //array_type check positive or negative!!
#include <stdio.h>
#include <stdlib.h>

extern int32_t line_num;    /* declared in scanner.l */
extern char current_line[]; /* declared in scanner.l */
extern FILE *yyin;          /* declared by lex */
extern char *yytext;        /* declared by lex */

extern int yylex(void);
static void yyerror(const char *msg);
extern int yylex_destroy(void);
%}

%token ID COMMA SEMI COLON L_PAREN R_PAREN L_BRACK R_BRACK PLUS MINUS MUL DIV MOD ASSIGN LT LEQ NE GEQ GT EQ AND OR NOT VAR ARRAY OF BOOL KWINT REAL KWSTRING KWTRUE KWFALSE DEF RETURN KWBEGIN END WHILE DO IF THEN ELSE FOR TO PRINT READ INT OCTAL FLOAT SCIENTIFIC STRING
%left AND OR NOT
%left LT LEQ NE GEQ GT EQ
%left MINUS
%left PLUS
%left MOD DIV
%left MUL
%left UMINUS

%%

program_unit: program
            | function
            ;

program: ID SEMI p0 compound_statement END
       ;

p0: declaration p0
  | p1
  |
  ;

p1: function p1
  |
  ;

function: function_declaration
        | function_definition
        ;

function_definition: function_header compound_statement END

function_declaration: function_header SEMI
                    ;

function_header: ID L_PAREN fh1 R_PAREN fh0
               ;

fh0: COLON scalar_type
   |
   ;

fh1: formal_argument fh2
   |
   ;

fh2: SEMI formal_argument fh2
   |
   ;

formal_argument: identifier_list COLON type
               ;

statement: simple_statement
         | conditional_statement
         | function_call_statement
         | loop_statement
         | return_statement
         | compound_statement
         ;

conditional_statement: IF expr THEN compound_statement cs
                     ;
cs: ELSE compound_statement END IF
  | END IF
  ;

loop_statement: while_statement
              | for_statement
              ;

while_statement: WHILE expr DO compound_statement END DO
               ;

for_statement: FOR ID ASSIGN integer_literal TO integer_literal DO compound_statement END DO
             ;

compound_statement: KWBEGIN cs0 END

cs0: declaration cs0
   | cs1
   |
   ;

cs1: statement cs1
   |
   ;

return_statement: RETURN expr SEMI;

function_call_statement: function_call SEMI
                       ;

simple_statement: assignment
                | print_statement
                | read_statement
                ;

read_statement: READ variable_reference SEMI
              ;

print_statement: PRINT expr SEMI
               ;

assignment: variable_reference ASSIGN expr SEMI
          ;

expr: literal_constant
    | variable_reference
    | function_call
    | binary_operation
    | unary_operation
    | L_PAREN expr R_PAREN

variable_reference: ID vr;

vr: L_BRACK expr R_BRACK
  | vr
  |
  ;

function_call: ID L_PAREN fc0 R_PAREN
               ;

fc0: expr fc1
   |
   ;

fc1: COMMA expr
    | fc1
    |
    ;

declaration: variable_declaration
           | constant_declaration
           ;


constant_declaration: VAR identifier_list COLON cd0 SEMI
                    ;

cd0: MINUS cd1 %prec UMINUS
   | cd1
   | string_literal
   | boolean_literal
   ;

cd1: integer_literal
   | real_literal
   ;
              
variable_declaration: VAR identifier_list COLON type SEMI
                    ;

identifier_list: ID il
               ;

il: COMMA ID il
  |
  ;

type: scalar_type
    | array_type
    ;

scalar_type: KWINT
           | REAL
           | KWSTRING
           | BOOL
           ;

array_type: ARRAY integer_literal OF type //check positive or negative!!
          ;

binary_operation: expr PLUS expr
                | expr MINUS expr
                | expr MUL expr
                | expr DIV expr
                | expr MOD expr
                | expr LT expr
                | expr LEQ expr
                | expr NE expr
                | expr GEQ expr
                | expr GT expr
                | expr EQ expr
                | expr AND expr
                | expr OR expr
                ;

unary_operation: MINUS expr %prec UMINUS
               | NOT expr
               ;
literal_constant: integer_literal
                | real_literal
                | string_literal
                | boolean_literal
                ;
integer_literal: OCTAL
               | INT
               ;
real_literal: FLOAT
            | SCIENTIFIC
            ;
string_literal: STRING
              ;
boolean_literal: KWTRUE
               | KWFALSE
               ;

%%

void yyerror(const char *msg) {
    fprintf(stderr,
            "\n"
            "|-----------------------------------------------------------------"
            "---------\n"
            "| Error found in Line #%d: %s\n"
            "|\n"
            "| Unmatched token: %s\n"
            "|-----------------------------------------------------------------"
            "---------\n",
            line_num, current_line, yytext);
    exit(-1);
}

int main(int argc, const char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        exit(-1);
    }

    yyin = fopen(argv[1], "r");
    if (yyin == NULL) {
        perror("fopen() failed");
        exit(-1);
    }

    yyparse();

    fclose(yyin);
    yylex_destroy();

    printf("\n"
           "|--------------------------------|\n"
           "|  There is no syntactic error!  |\n"
           "|--------------------------------|\n");
    return 0;
}
