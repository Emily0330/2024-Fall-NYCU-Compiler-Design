%{
#include "AST/BinaryOperator.hpp"
#include "AST/CompoundStatement.hpp"
#include "AST/ConstantValue.hpp"
#include "AST/FunctionInvocation.hpp"
#include "AST/UnaryOperator.hpp"
#include "AST/VariableReference.hpp"
#include "AST/assignment.hpp"
#include "AST/ast.hpp"
#include "AST/decl.hpp"
#include "AST/expression.hpp"
#include "AST/for.hpp"
#include "AST/function.hpp"
#include "AST/if.hpp"
#include "AST/print.hpp"
#include "AST/program.hpp"
#include "AST/read.hpp"
#include "AST/return.hpp"
#include "AST/variable.hpp"
#include "AST/while.hpp"
#include "AST/AstDumper.hpp"
#include "AST/ast.hpp"
#include "AST/statement.hpp" // added
#include "AST/expr.hpp" // added

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <iostream>

#define YYLTYPE yyltype

typedef struct YYLTYPE {
    uint32_t first_line;
    uint32_t first_column;
    uint32_t last_line;
    uint32_t last_column;
} yyltype;

extern uint32_t line_num;   /* declared in scanner.l */
extern char current_line[]; /* declared in scanner.l */
extern FILE *yyin;          /* declared by lex */
extern char *yytext;        /* declared by lex */

static AstNode *root;

extern "C" int yylex(void);
static void yyerror(const char *msg);
extern int yylex_destroy(void);
%}

// This guarantees that headers do not conflict when included together.
%define api.token.prefix {TOK_}

%code requires {
    #include "AST/ast.hpp"
    #include "AST/statement.hpp"
    class AstNode;
    class CompoundStatementNode;
    class DeclNode;
    class ConstantValueNode;
    class VariableNode;
    class FunctionNode;
    class PrintNode;
    class BinaryOperatorNode;
    class UnaryOperatorNode;
    class FunctionInvocationNode;
    class VariableReferenceNode;
    class AssignmentNode;
    class ReadNode;
    class IfNode;
    class WhileNode;
    class ForNode;
    class ReturnNode;
}

    /* For yylval */
%union {
    /* basic semantic value */
    char *identifier;
    char* str_literal;
    char* type;
    bool bool_literal;
    int int_literal;
    double real_literal;

    AstNode *node;
    CompoundStatementNode *compound_stmt_ptr;
    DeclNode* decl_node;
    std::vector<DeclNode *>* decl_list;
    ConstantValueNode *constant_val_ptr;
    bool neg;
    Variable *variable_info;
    TypeInfo *type_info;
    std::vector<IdLoc*>* idloc_list;
    FunctionNode* function_ptr;
    std::vector<FunctionNode*>* func_list;
    std::vector<VariableNode*>* var_list;
    std::vector<std::vector<VariableNode*>*>* list_of_var_list;
    StatementNode* stmt_ptr;
    std::vector<StatementNode*>* stmt_list;
    ExprNode* expr_ptr;
    std::vector<ExprNode*>* expr_list;
    VariableReferenceNode* var_ref_ptr;
    IfNode* if_ptr;
    WhileNode* while_ptr;
    ForNode* for_ptr;
    ReturnNode* return_ptr;
};

%type <identifier> ProgramName FunctionName
%type <compound_stmt_ptr> CompoundStatement
%type <decl_node> Declaration
%type <decl_list> Declarations DeclarationList FormalArgList
%type <constant_val_ptr> LiteralConstant
%type <neg> NegOrNot
%type <variable_info> StringAndBoolean IntegerAndReal
%type <type_info> ArrDecl ArrType ScalarType Type ReturnType
%type <idloc_list> IdList
%type <function_ptr> FunctionDeclaration FunctionDefinition Function
%type <func_list> Functions FunctionList
%type <var_list> FormalArg
%type <list_of_var_list> FormalArgs
%type <stmt_ptr> Statement Simple ElseOrNot
%type <stmt_list> StatementList Statements
%type <expr_ptr> Expression FunctionInvocation FunctionCall
%type <expr_list> ExpressionList Expressions ArrRefs ArrRefList
%type <var_ref_ptr> VariableReference
%type <if_ptr> Condition
%type <while_ptr> While
%type <for_ptr> For
%type <return_ptr> Return

    /* Follow the order in scanner.l */

    /* Delimiter */
%token COMMA SEMICOLON COLON
%token L_PARENTHESIS R_PARENTHESIS
%token L_BRACKET R_BRACKET

    /* Operator */
%token ASSIGN
    /* TODO: specify the precedence of the following operators */
%token MULTIPLY DIVIDE MOD
%token LESS LESS_OR_EQUAL EQUAL GREATER GREATER_OR_EQUAL NOT_EQUAL
%token OR AND NOT
%token PLUS MINUS
%token NEG

%left OR AND NOT
%left LESS LESS_OR_EQUAL EQUAL GREATER GREATER_OR_EQUAL NOT_EQUAL
%left MINUS
%left PLUS
%left DIVIDE MOD
%left MULTIPLY
%left NEG


    /* Keyword */
%token <type> ARRAY BOOLEAN INTEGER REAL STRING
%token END BEGIN
%token DO ELSE FOR IF THEN WHILE
%token DEF OF TO RETURN VAR
%token <bool_literal> FALSE TRUE
%token PRINT READ

    /* Identifier */
%token <identifier> ID

    /* Literal */
%token <int_literal> INT_LITERAL
%token <real_literal> REAL_LITERAL
%token <str_literal> STRING_LITERAL

%%

ProgramUnit:
    Program
    |
    Function
;

Program:
    ProgramName SEMICOLON
    /* ProgramBody */
    DeclarationList FunctionList CompoundStatement
    /* End of ProgramBody */
    END {
        root = new ProgramNode(@1.first_line, @1.first_column,
                               $1, $5, $3, *($4)); // $5 is for compound statement, we've added this argument in ProgramNode in program.hpp

        free($1);
    }
;

ProgramName:
    ID
;

DeclarationList:
    Epsilon {
        // do nothing, just initialize the vector
        $$ = new std::vector<DeclNode*>;
    }
    |
    Declarations{
        $$ = $1;
    }
;

Declarations:
    Declaration{
        $$ = new std::vector<DeclNode*>;
        $$->emplace_back($1);
    }
    |
    Declarations Declaration {
        $1->emplace_back($2);
        $$ = $1;
    }
;

FunctionList: // std::vector<FunctionNode*>*
    Epsilon{
        $$ = new std::vector<FunctionNode*>;
    }
    |
    Functions{
        $$ = $1;
    }
;

Functions: // std::vector<FunctionNode*>*
    Function{
        $$ = new std::vector<FunctionNode*>;
        $$->emplace_back($1);
    }
    |
    Functions Function{
        $1->emplace_back($2);
        $$ = $1;
    }
;

Function: // FunctionNode*
    FunctionDeclaration{
        $$ = $1;
    }
    |
    FunctionDefinition{
        $$ = $1;
    }
;

FunctionDeclaration:
    FunctionName L_PARENTHESIS FormalArgList R_PARENTHESIS ReturnType SEMICOLON {
        if($3 != nullptr){
            CompoundStatementNode* compound = nullptr;
            $$ = new FunctionNode(@1.first_line,@1.first_column,$1, *($3), $5, compound);
        }
        else{
            std::vector<DeclNode*>* tmp = new std::vector<DeclNode*>;
            CompoundStatementNode* compound = nullptr;
            $$ = new FunctionNode(@1.first_line,@1.first_column,$1,*tmp,$5, compound);
        }
    }
;

FunctionDefinition:
    FunctionName L_PARENTHESIS FormalArgList R_PARENTHESIS ReturnType
    CompoundStatement
    END { // if FormalArgList returns nullptr, then the function does not have args!
        if($3 != nullptr){
            $$ = new FunctionNode(@1.first_line,@1.first_column,$1, *($3), $5, $6);
        }
        else{
            std::vector<DeclNode*>* tmp = new std::vector<DeclNode*>;
            $$ = new FunctionNode(@1.first_line,@1.first_column,$1,*tmp,$5, $6);
        }
    }
;

FunctionName:
    ID{
        $$ = $1;
    }
;

FormalArgList: // std::vector<DeclNode*>*
    Epsilon {
        $$ = nullptr;
    }
    |
    FormalArgs{// FormalArgs is std::vector<std::vector<VariableNode*>*>*
        $$ = new std::vector<DeclNode*>;
        for(auto& var_node_ptr: *($1)){ // var_node_ptr is std::vector<VariableNode*>*
            DeclNode* tmp = new DeclNode((*var_node_ptr)[0]->location.line,(*var_node_ptr)[0]->location.col, *var_node_ptr/*location, var_list,...*/);
            $$->emplace_back(tmp);
        }
    }
;

FormalArgs: // std::vector<std::vector<VariableNode*>*>*
    FormalArg{// append $1;
        $$ = new std::vector<std::vector<VariableNode*>*>;
        $$->emplace_back($1);
    }
    |
    FormalArgs SEMICOLON FormalArg{
        $$->emplace_back($3);
    }
;

FormalArg: // std::vector<VariableNode*>* 
    IdList COLON Type{ // IdList returns std::vector<IdLoc*>*
        $$ = new std::vector<VariableNode*>;
        for(auto& idloc_ptr: *($1)){
            VariableNode* tmp_var = new VariableNode(idloc_ptr->line, idloc_ptr->col, idloc_ptr->identifier, $3, nullptr);/*type and constval*/
            $$->emplace_back(tmp_var);
        }
    }
;

IdList:
    ID{
        IdLoc* tmp = new IdLoc;
        tmp->line = @1.first_line;
        tmp->col = @1.first_column;
        tmp->identifier = $1;

        $$ = new std::vector<IdLoc*>;
        $$->emplace_back(tmp);
    }
    |
    IdList COMMA ID{
        IdLoc* tmp = new IdLoc;
        tmp->line = @3.first_line;
        tmp->col = @3.first_column;
        tmp->identifier = $3;

        $$->emplace_back(tmp);
    }
;

ReturnType: // should return TypeInfo*
    COLON ScalarType{
        $$ = $2; // ScalarType also returns TypeInfo*
    }
    |
    Epsilon{
        // return nothing
        $$ = new TypeInfo;
        $$->type = NULLTYPE;
    }
;

    /*
       Data Types and Declarations
                                   */

Declaration:
    VAR IdList COLON Type SEMICOLON
    {   std::vector<VariableNode*> tmp;
        for(auto& idloc_ptr: *($2)){
            VariableNode* tmp_var = new VariableNode(idloc_ptr->line, idloc_ptr->col, idloc_ptr->identifier, $4, nullptr);/*type and constval*/
            tmp.emplace_back(tmp_var);
        }
        $$ = new DeclNode(@1.first_line, @1.first_column, tmp);
    }
    |
    VAR IdList COLON LiteralConstant SEMICOLON // LiteralConstant is a ConstantValueNode*
    {
        std::vector<VariableNode*> tmp;
        TypeInfo* tmp_var_type = new TypeInfo;
        tmp_var_type->type = $4->constant_value->var_type;
        if($4->constant_value->var_type == INTEGER){
            tmp_var_type->scalar_type = "integer";
        }
        else if($4->constant_value->var_type == REAL){
            tmp_var_type->scalar_type = "real";
        }
        else if($4->constant_value->var_type == STRING){
            tmp_var_type->scalar_type = "string";
        }
        else if($4->constant_value->var_type == BOOLEAN){
            tmp_var_type->scalar_type = "boolean";
        }
        else if($4->constant_value->var_type == ARRAY){ // literal constant won't have array type!?
            // do nothing
        }
        for(auto& idloc_ptr: *($2)){
            VariableNode* tmp_var = new VariableNode(idloc_ptr->line, idloc_ptr->col, idloc_ptr->identifier, tmp_var_type, $4);/*type and constval*/
            tmp.emplace_back(tmp_var);
        }
        $$ = new DeclNode(@1.first_line, @1.first_column, tmp);
    }
;

Type:
    ScalarType{
        $$ = $1;
    }
    |
    ArrType{
        $$ = $1;
    }
;

ScalarType:
    INTEGER{
        TypeInfo* tmp = new TypeInfo;
        tmp->type = INTEGER;
        tmp->scalar_type = "integer";
        $$ = tmp;
    }
    |
    REAL{
        TypeInfo* tmp = new TypeInfo;
        tmp->type = REAL;
        tmp->scalar_type = "real";
        $$ = tmp;
    }
    |
    STRING{
        TypeInfo* tmp = new TypeInfo;
        tmp->type = STRING;
        tmp->scalar_type = "string";
        $$ = tmp;
    }
    |
    BOOLEAN{
        TypeInfo* tmp = new TypeInfo;
        tmp->type = BOOLEAN;
        tmp->scalar_type = "boolean";
        $$ = tmp;
    }
;

ArrType:
    ArrDecl ScalarType{
        $1->scalar_type = ($2)->scalar_type;
        $$ = $1;
    }
;

ArrDecl:
    ARRAY INT_LITERAL OF{
        TypeInfo *tmp = new TypeInfo;
        tmp->type = ARRAY;
        tmp->array_dim.emplace_back($2);
        $$ = tmp;
    }
    |
    ArrDecl ARRAY INT_LITERAL OF{
        $$ = $1;
        $$->array_dim.emplace_back($3);
    }
;

LiteralConstant:
    NegOrNot INT_LITERAL{
        Variable *tmp = new Variable;
        tmp->var_type = INTEGER;
        if($1) {tmp->int_val = -$2;}
        else tmp->int_val = $2;
        $$ = new ConstantValueNode(@2.first_line, @2.first_column, tmp);
    }
    |
    NegOrNot REAL_LITERAL{
        Variable *tmp = new Variable;
        tmp->var_type = REAL;
        if($1) tmp->real_val = -$2;
        else tmp->real_val = $2;
        $$ = new ConstantValueNode(@1.first_line, @1.first_column, tmp);
    }
    |
    StringAndBoolean{
        Variable *tmp = new Variable;
        *tmp = *($1);
        $$ = new ConstantValueNode(@1.first_line, @1.first_column, tmp);
    }
;

NegOrNot:
    Epsilon{
        $$ = false;
    }
    |
    MINUS{
        $$ = true;
    }
;

StringAndBoolean:
    STRING_LITERAL{
        Variable* tmp = new Variable;
        tmp->var_type = STRING;
        tmp->str_val = $1;
        $$ = tmp;
    }
    |
    TRUE{
        Variable* tmp = new Variable;
        tmp->var_type = BOOLEAN;
        tmp->bool_val = $1;
        $$ = tmp;
    }
    |
    FALSE{
        Variable* tmp = new Variable;
        tmp->var_type = BOOLEAN;
        tmp->bool_val = $1;
        $$ = tmp;
    }
;

IntegerAndReal: // Variable*
    INT_LITERAL{ // INT_LITERAL is an integer
        Variable* tmp = new Variable;
        tmp->var_type = INTEGER;
        tmp->int_val = $1;
        $$ = tmp;
    }
    |
    REAL_LITERAL{
        Variable* tmp = new Variable;
        tmp->var_type = REAL;
        tmp->real_val = $1;
        $$ = tmp;
    }
;

    /*
       Statements
                  */

Statement: // StatementNode*
    CompoundStatement{ // CompoundStatement returns CompoundStatementNode*
        $$ = new StatementNode;
        $$->stmt_type = EnumStmtType::TYPE_COMPOUND;
        $$->compound_node = $1;
    }
    |
    Simple{
        $$ = $1;
    }
    |
    Condition{ // Condition returns IfNode*
        $$ = new StatementNode;
        $$->stmt_type = EnumStmtType::TYPE_IF;
        $$->if_node = $1;
    }
    |
    While{
        $$ = new StatementNode;
        $$->stmt_type = EnumStmtType::TYPE_WHILE;
        $$->while_node = $1;
    }
    |
    For{
        $$ = new StatementNode;
        $$->stmt_type = EnumStmtType::TYPE_FOR;
        $$->for_node = $1;
    }
    |
    Return{
        $$ = new StatementNode;
        $$->stmt_type = EnumStmtType::TYPE_RETURN;
        $$->return_node = $1;
    }
    |
    FunctionCall{ // FunctionCall is ExprNode*
        $$ = new StatementNode;
        $$->stmt_type = EnumStmtType::TYPE_FUNCTION_CALL;
        $$->function_call_node = $1->function_call_node;
    }
;

CompoundStatement:
    BEGIN
    DeclarationList
    StatementList
    END {
        // std::vector<StatementNode*> stmt_list;
        $$ = new CompoundStatementNode(@1.first_line, @1.first_column, *$2, *$3);
    }
;

Simple: // StatementNode*
    VariableReference ASSIGN Expression SEMICOLON{
        $$ = new StatementNode;
        $$->stmt_type = EnumStmtType::TYPE_ASSIGNMENT;
        $$->assignment_node = new AssignmentNode(@2.first_line,@2.first_column,$1,$3);
    }
    |
    PRINT Expression SEMICOLON{
        $$ = new StatementNode;
        $$->stmt_type = TYPE_PRINT;
        $$->print_node = new PrintNode(@1.first_line, @1.first_column, $2);
    }
    |
    READ VariableReference SEMICOLON{ // VariableReference is VariableReferenceNode*
        $$ = new StatementNode;
        $$->stmt_type = EnumStmtType::TYPE_READ;
        $$->read_node = new ReadNode(@1.first_line,@1.first_column,$2);
    }
;

VariableReference: // VariableReferenceNode*
    ID ArrRefList{
        $$ = new VariableReferenceNode(@1.first_line,@1.first_column,$1,*($2));
    }
;

ArrRefList: // std::vector<ExprNode*>*
    Epsilon{
        $$ = new std::vector<ExprNode*>;
    }
    |
    ArrRefs{
        $$ = $1;
    }
;

ArrRefs: // std::vector<ExprNode*>*
    L_BRACKET Expression R_BRACKET{
        $$ = new std::vector<ExprNode*>;
        $$->emplace_back($2);
    }
    |
    ArrRefs L_BRACKET Expression R_BRACKET{
        $1->emplace_back($3);
        $$ = $1;
    }
;

Condition: // IfNode*
    IF Expression THEN
    CompoundStatement
    ElseOrNot
    END IF{
        std::vector<CompoundStatementNode*> tmp;
        tmp.emplace_back($4);
        if($5->stmt_type == EnumStmtType::TYPE_COMPOUND){ // ElseOrNot is a StatementNode*
            tmp.emplace_back($5->compound_node);
        }
        $$ = new IfNode(@1.first_line, @1.first_column, $2, tmp);
    }
;

ElseOrNot: // StatementNode*
    ELSE
    CompoundStatement{
        $$ = new StatementNode;
        $$->stmt_type = EnumStmtType::TYPE_COMPOUND;
        $$->compound_node = $2;
    }
    |
    Epsilon{
        $$ = new StatementNode;
        $$->stmt_type = EnumStmtType::TYPE_NULL;
    }
;

While: // WhileNode*
    WHILE Expression DO
    CompoundStatement
    END DO{
        $$ = new WhileNode(@1.first_line, @1.first_column, $2, $4);
    }
;

For: // ForNode* // ForNode::ForNode(const uint32_t line, const uint32_t col, DeclNode* decl, AssignmentNode* assignment, struct _ExprNode* expr, CompoundStatementNode* compound)
    FOR ID ASSIGN INT_LITERAL TO INT_LITERAL DO
    CompoundStatement
    END DO{
        // declaration of ID variable (integer type)
        TypeInfo* my_type_info = new TypeInfo;
        my_type_info->type = EnumType::INTEGER;
        my_type_info->scalar_type = "integer";

        VariableNode* var = new VariableNode(@2.first_line,@2.first_column,$2,my_type_info,nullptr); // does not have initial value
        
        std::vector<VariableNode*> my_vec;
        my_vec.emplace_back(var);

        DeclNode* decl = new DeclNode(@2.first_line,@2.first_column,my_vec);

        // AssignmentNode(const uint32_t line, const uint32_t col, VariableReferenceNode* variable_reference, struct _ExprNode* expr
        std::vector<ExprNode*>empty_expr_list;
        VariableReferenceNode* my_var_ref = new VariableReferenceNode(@2.first_line,@2.first_column,$2,empty_expr_list); // because the variable here is composed of an identifier (not array variable)
        ExprNode* my_expr = new ExprNode;
        my_expr->expr_type = TYPE_CONSTANT_VALUE;
        Variable* var_struct = new Variable;
        var_struct->var_type = EnumType::INTEGER;
        var_struct->int_val = $4;
        my_expr->constant_value_node = new ConstantValueNode(@4.first_line,@4.first_column,var_struct);
        AssignmentNode* assignment = new AssignmentNode(@3.first_line,@3.first_column,my_var_ref,my_expr);
    
        // ExprNode* (actually a constant value)    
        ExprNode* condition_const_value = new ExprNode;
        condition_const_value->expr_type = EnumExprType::TYPE_CONSTANT_VALUE;
        Variable* condition_var_struct = new Variable;
        condition_var_struct->var_type = EnumType::INTEGER;
        condition_var_struct->int_val = $6;
        condition_const_value->constant_value_node = new ConstantValueNode(@6.first_line,@6.first_column,condition_var_struct);

        $$ = new ForNode(@1.first_line,@1.first_column,decl,assignment,condition_const_value,$8);
    }
;

Return: // ReturnNode*
    RETURN Expression SEMICOLON{
        $$ = new ReturnNode(@1.first_line,@1.first_column,$2);
    }
;

FunctionCall: // ExprNode*
    FunctionInvocation SEMICOLON{
        $$ = $1;
    }
;

FunctionInvocation: // ExprNode*
    ID L_PARENTHESIS ExpressionList R_PARENTHESIS {
        // ID is function name
        $$ = new ExprNode;
        $$->expr_type =EnumExprType::TYPE_FUNC_CALL;
        
        FunctionInvocationNode* tmp = new FunctionInvocationNode(@1.first_line,@1.first_column,$1,*($3));
        $$->function_call_node = tmp;
    }
;

ExpressionList: // std::vector<ExprNode*>*
    Epsilon{
        $$ = new std::vector<ExprNode*>;
    }
    |
    Expressions{
        $$ = $1;
    }
;

Expressions: // std::vector<ExprNode*>*
    Expression{
        $$ = new std::vector<ExprNode*>;
        $$->emplace_back($1);
    }
    |
    Expressions COMMA Expression{
        $1->emplace_back($3);
        $$ = $1;
    }
;

StatementList:// std::vector<StatementNode*>*
    Epsilon{
        $$ = new std::vector<StatementNode*>;
    }
    |
    Statements{
        $$ = $1;
    }
;

Statements: // std::vector<StatementNode*>*
    Statement{
        $$ = new std::vector<StatementNode*>;
        $$->emplace_back($1);
    }
    |
    Statements Statement{
        $1->emplace_back($2);
        $$ = $1;
    }
;

Expression: // ExprNode*
    L_PARENTHESIS Expression R_PARENTHESIS{
        $$ = $2;
    }
    |
    MINUS Expression %prec NEG{
        $$ = new ExprNode;
        $$->expr_type = EnumExprType::TYPE_UNARY_OP;
        $$->unary_op_node = new UnaryOperatorNode(@1.first_line, @1.first_column, EnumOpType::TYPE_NEGATIVE, $2);
    }
    |
    Expression MULTIPLY Expression{
        std::vector<ExprNode*> tmp;
        tmp.emplace_back($1);
        tmp.emplace_back($3);
        $$ = new ExprNode;
        $$->expr_type = EnumExprType::TYPE_BINARY_OP;
        $$->binary_op_node = new BinaryOperatorNode(@2.first_line, @2.first_column, EnumOpType::TYPE_MULTIPLY, tmp);
    }
    |
    Expression DIVIDE Expression{
        std::vector<ExprNode*> tmp;
        tmp.emplace_back($1);
        tmp.emplace_back($3);
        $$ = new ExprNode;
        $$->expr_type = EnumExprType::TYPE_BINARY_OP;
        $$->binary_op_node = new BinaryOperatorNode(@2.first_line, @2.first_column, EnumOpType::TYPE_DIVIDE, tmp);
    }
    |
    Expression MOD Expression{
        std::vector<ExprNode*> tmp;
        tmp.emplace_back($1);
        tmp.emplace_back($3);
        $$ = new ExprNode;
        $$->expr_type = EnumExprType::TYPE_BINARY_OP;
        $$->binary_op_node = new BinaryOperatorNode(@2.first_line, @2.first_column, EnumOpType::TYPE_MOD, tmp);
    }
    |
    Expression PLUS Expression{
        std::vector<ExprNode*> tmp;
        tmp.emplace_back($1);
        tmp.emplace_back($3);
        $$ = new ExprNode;
        $$->expr_type = EnumExprType::TYPE_BINARY_OP;
        $$->binary_op_node = new BinaryOperatorNode(@2.first_line, @2.first_column, EnumOpType::TYPE_PLUS, tmp);
    }
    |
    Expression MINUS Expression{
        std::vector<ExprNode*> tmp;
        tmp.emplace_back($1);
        tmp.emplace_back($3);
        $$ = new ExprNode;
        $$->expr_type = EnumExprType::TYPE_BINARY_OP;
        $$->binary_op_node = new BinaryOperatorNode(@2.first_line, @2.first_column, EnumOpType::TYPE_SUBTRACT, tmp);
    }
    |
    Expression LESS Expression{
        std::vector<ExprNode*> tmp;
        tmp.emplace_back($1);
        tmp.emplace_back($3);
        $$ = new ExprNode;
        $$->expr_type = EnumExprType::TYPE_BINARY_OP;
        $$->binary_op_node = new BinaryOperatorNode(@2.first_line, @2.first_column, EnumOpType::TYPE_LESS, tmp);
    }
    |
    Expression LESS_OR_EQUAL Expression{
        std::vector<ExprNode*> tmp;
        tmp.emplace_back($1);
        tmp.emplace_back($3);
        $$ = new ExprNode;
        $$->expr_type = EnumExprType::TYPE_BINARY_OP;
        $$->binary_op_node = new BinaryOperatorNode(@2.first_line, @2.first_column, EnumOpType::TYPE_LESS_OR_EQUAL, tmp);
    }
    |
    Expression GREATER Expression{
        std::vector<ExprNode*> tmp;
        tmp.emplace_back($1);
        tmp.emplace_back($3);
        $$ = new ExprNode;
        $$->expr_type = EnumExprType::TYPE_BINARY_OP;
        $$->binary_op_node = new BinaryOperatorNode(@2.first_line, @2.first_column, EnumOpType::TYPE_GREATER, tmp);
    }
    |
    Expression GREATER_OR_EQUAL Expression{
        std::vector<ExprNode*> tmp;
        tmp.emplace_back($1);
        tmp.emplace_back($3);
        $$ = new ExprNode;
        $$->expr_type = EnumExprType::TYPE_BINARY_OP;
        $$->binary_op_node = new BinaryOperatorNode(@2.first_line, @2.first_column, EnumOpType::TYPE_GREATER_OR_EQUAL, tmp);
    }
    |
    Expression EQUAL Expression{
        std::vector<ExprNode*> tmp;
        tmp.emplace_back($1);
        tmp.emplace_back($3);
        $$ = new ExprNode;
        $$->expr_type = EnumExprType::TYPE_BINARY_OP;
        $$->binary_op_node = new BinaryOperatorNode(@2.first_line, @2.first_column, EnumOpType::TYPE_EQUAL, tmp);
    }
    |
    Expression NOT_EQUAL Expression{
        std::vector<ExprNode*> tmp;
        tmp.emplace_back($1);
        tmp.emplace_back($3);
        $$ = new ExprNode;
        $$->expr_type = EnumExprType::TYPE_BINARY_OP;
        $$->binary_op_node = new BinaryOperatorNode(@2.first_line, @2.first_column, EnumOpType::TYPE_NOT_EQUAL, tmp);
    }
    |
    NOT Expression{
        $$ = new ExprNode;
        $$->expr_type = EnumExprType::TYPE_UNARY_OP;
        $$->unary_op_node = new UnaryOperatorNode(@1.first_line, @1.first_column, EnumOpType::TYPE_NOT, $2);
    }
    |
    Expression AND Expression{
        std::vector<ExprNode*> tmp;
        tmp.emplace_back($1);
        tmp.emplace_back($3);
        $$ = new ExprNode;
        $$->expr_type = EnumExprType::TYPE_BINARY_OP;
        $$->binary_op_node = new BinaryOperatorNode(@2.first_line, @2.first_column, EnumOpType::TYPE_AND, tmp);
    }
    |
    Expression OR Expression{
        std::vector<ExprNode*> tmp;
        tmp.emplace_back($1);
        tmp.emplace_back($3);
        $$ = new ExprNode;
        $$->expr_type = EnumExprType::TYPE_BINARY_OP;
        $$->binary_op_node = new BinaryOperatorNode(@2.first_line, @2.first_column, EnumOpType::TYPE_OR, tmp);
    }
    |
    IntegerAndReal{ // Variable*
        $$ = new ExprNode;
        $$->expr_type = EnumExprType::TYPE_CONSTANT_VALUE;
        $$->constant_value_node = new ConstantValueNode(@1.first_line,@1.first_column,$1);
    }
    |
    StringAndBoolean{ // Variable*
        $$ = new ExprNode;
        $$->expr_type = EnumExprType::TYPE_CONSTANT_VALUE;
        $$->constant_value_node = new ConstantValueNode(@1.first_line,@1.first_column,$1);
    }
    |
    VariableReference{ // VariableReferenceNode*
        $$ = new ExprNode;
        $$->expr_type = EnumExprType::TYPE_VARIABLE_REFER;
        $$->variable_reference_node = $1;
    }
    |
    FunctionInvocation{ // ExprNode*
        $$ = $1;
    }
;

    /*
       misc
            */
Epsilon:
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
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <filename> [--dump-ast]\n", argv[0]);
        exit(-1);
    }

    yyin = fopen(argv[1], "r");
    if (yyin == NULL) {
        perror("fopen() failed");
        exit(-1);
    }

    yyparse();

    if (argc >= 3 && strcmp(argv[2], "--dump-ast") == 0) {
        // root->print();
        AstDumper ast_dumper;
        root->accept(ast_dumper); // why???
    }

    printf("\n"
           "|--------------------------------|\n"
           "|  There is no syntactic error!  |\n"
           "|--------------------------------|\n");

    delete root;
    fclose(yyin);
    yylex_destroy();
    return 0;
}