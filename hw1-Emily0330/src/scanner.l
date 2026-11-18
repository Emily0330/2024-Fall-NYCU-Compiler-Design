%option never-interactive
%option nounput
%option noinput
%s COMMENT
%{
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LEN 512
/* Code runs each time a token is matched. */
//pseudocomment in start does not take effects
#define YY_USER_ACTION \
    if(YY_START != COMMENT) { \
        if(strncmp(yytext + 2,"&S-", 3) == 0) opt_src = 0; \
        else if(strncmp(yytext + 2,"&S+", 3) == 0) opt_src = 1; \
        else if(strncmp(yytext + 2, "&T+", 3) == 0) opt_tok = 1; \
        else if(strncmp(yytext + 2, "&T-", 3) == 0) opt_tok = 0; \
    } \
    updateCurrentLine(yytext);
uint32_t line_num = 1;
uint32_t col_num = 1;
static uint32_t opt_src = 1;
static uint32_t opt_tok = 1;
static char current_line[MAX_LINE_LEN];

static void updateCurrentLine(const char *source);
static void listToken(const char *name);
static void listLiteral(const char *name, const char *literal);
//static void processPsuedoComment(const char *command);
static const char* processString(char *old, int strlen);

%}
decimal_int [0]|([1-9][0-9]*)
octal_int [0][0-7]+
identifier [a-zA-Z][0-9a-zA-Z]*
float ([0]|([1-9][0-9]*))\.(([0-9]*[1-9])|0)
scientific (([1-9][0-9]*\.(([0-9]*[1-9])|0))|(0\.([0-9]*[1-9]))|([1-9][0-9]*))[eE][-+]?([0]|([1-9][0-9]*)) 
delimiter [,;:()\[\]]
arithmetic [-+*/=<>]
string \"([^"]|(\"\"))*\"
whitespace [ \t\n]
one_line_comment "//"[^\n]*
%%
<INITIAL>"and" { listToken("and"); }
<INITIAL>"or" { listToken("or"); }
<INITIAL>"not" { listToken("not"); }
<INITIAL>"<=" { listToken("<="); }
<INITIAL>">=" { listToken(">="); }
<INITIAL>"<>" { listToken("<>"); }
<INITIAL>"mod" { listToken("mod"); }
<INITIAL>":=" { listToken(":="); }
<INITIAL>{delimiter} { listToken(yytext); }
<INITIAL>{arithmetic} { listToken(yytext); }
<INITIAL>"var" { listToken("KWvar"); }
<INITIAL>"def" { listToken("KWdef"); }
<INITIAL>"array" { listToken("KWarray"); }
<INITIAL>"of" { listToken("KWof"); }
<INITIAL>"boolean" { listToken("KWboolean"); }
<INITIAL>"integer" { listToken("KWinteger"); }
<INITIAL>"real" { listToken("KWreal"); }
<INITIAL>"string" { listToken("KWstring"); }
<INITIAL>"true" { listToken("KWtrue"); }
<INITIAL>"false" { listToken("KWfalse"); }
<INITIAL>"while" { listToken("KWwhile"); }
<INITIAL>"do" { listToken("KWdo"); }
<INITIAL>"if" { listToken("KWif"); }
<INITIAL>"then" { listToken("KWthen"); }
<INITIAL>"else" { listToken("KWelse"); }
<INITIAL>"for" { listToken("KWfor"); }
<INITIAL>"to" { listToken("KWto"); }
<INITIAL>"begin" { listToken("KWbegin"); }
<INITIAL>"end" { listToken("KWend"); }
<INITIAL>"print" { listToken("KWprint"); }
<INITIAL>"read" { listToken("KWread"); }
<INITIAL>"return" { listToken("KWreturn"); }
<INITIAL>{octal_int} { listLiteral("oct_integer", yytext); }
<INITIAL>{decimal_int} { listLiteral("integer", yytext); }
<INITIAL>{float} { listLiteral("float", yytext); } 
<INITIAL>{identifier} { listLiteral("id", yytext); }
<INITIAL>{scientific} { listLiteral("scientific", yytext); }
<INITIAL>{string} { listLiteral("string", processString(yytext, yyleng));}

<INITIAL>{one_line_comment} {;} //C++-style one-line comment, put after pseudocomment
<INITIAL>{whitespace} {;}
<COMMENT>\n {;}
<COMMENT>"*/" {BEGIN INITIAL;}
<COMMENT>. {;} //. does not include \n !!
<INITIAL>"/*" {BEGIN COMMENT;}

    /* Catch the character which is not accepted by rules above */
. {
    printf("Error at line %d: bad character \"%s\"\n", line_num, yytext);
    exit(-1);
}

%%

/** @note The line is printed out and flushed when a newline character is encountered. */
static void updateCurrentLine(const char *source) {
    /* col_num is one-based */
    for (const char *c = source; *c; ++c) {
        if (*c == '\n') {
            if (opt_src) {
                // printf("opt_src: %d\n", opt_src);
                // printf("yytext: %s\n", current_line);
                printf("%d: %s\n", line_num, current_line);
            }
            ++line_num;
            col_num = 1;
            current_line[0] = '\0';
        } else {
            if (col_num < MAX_LINE_LEN) {
                current_line[col_num - 1] = *c;
                current_line[col_num] = '\0';
            } else {
                /* Truncate silently; doesn't affect the program's correctness. */
            }
            ++col_num;
        }
    }
}

static void listToken(const char *name) {
    if (opt_tok) {
        printf("<%s>\n", name);
    }
}

static void listLiteral(const char *name, const char *literal) {
    if (opt_tok) {
        printf("<%s: %s>\n", name, literal);
    }
}

// static void processPsuedoComment(const char *command){
//     command += 2; // remove the first two //
//     //the comment after the pseudocomment is ignored

//     if(strncmp(command, "&S+", 3) == 0) opt_src = 1;
//     else if(strncmp(command, "&S-", 3) == 0) opt_src = 0;
//     else if(strncmp(command, "&T+", 3) == 0) opt_tok = 1;
//     else opt_tok = 0; //&T-

//     return;
// }

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
            //co++; //does not need to skip the second double quote because we move the string afterwards 1 position forward
            strlen -= 1;
        }
    }
    return old;
}

/** @note This function is not required if the input file is guaranteed to end
 * with a newline. However, students may find it useful to handle the case where
 * the input file does not end with a newline, as it has been reported several
 * times in the past.
 */
int yywrap(void) {
    /* If the file is not ended with a newline, fake it to print out the last line. */
    if (col_num > 1) {
        updateCurrentLine("\n");
    }
    /* no more input file */
    return 1;
}

int main(int argc, const char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s [filename]\n", argv[0]);
        exit(-1);
    }

    yyin = fopen(argv[1], "r");
    if (yyin == NULL) {
        fprintf(stderr, "Open file error\n");
        exit(-1);
    }

    yylex();

    fclose(yyin);
    /* Free resources used by the scanner; otherwise, unused data structures
    will persist throughout the successive compiler phases. */
    yylex_destroy();

    return 0;
}
